#include "minirecorder.h"
#include <QDebug>

MiniRecorder::MiniRecorder(QObject *parent) : QObject(parent)
{
    bRunning = false;

    pAVInputFormat = nullptr;
    pAVCodecContext = nullptr;
    pAVFormatContext = nullptr;
    pAVFrame = nullptr;
    outFrame = nullptr;
    pAVCodec = nullptr;
    pOutAVCodec = nullptr;
    pAVPacket = nullptr;
    pOptions = nullptr;
    pAVOutputFormat = nullptr;
    pOutAVFormatContext = nullptr;
    outAVCodecContext = nullptr;
    pAVStream = nullptr;
    pSwsCtx = nullptr;
    mValue = 0;
    mVideoStreamIndx = -1;

    mFrameRate = -1;
    mWidth = -1;
    mHeight = -1;

    av_register_all();
    avcodec_register_all();
    avdevice_register_all();
}

MiniRecorder::~MiniRecorder()
{

}

void MiniRecorder::Init(const int width, const int height, const int framerate)
{
    mWidth = width;
    mHeight = height;
    mFrameRate = framerate;
}

void MiniRecorder::OpenDesktop()
{
    QString info;

    pAVFormatContext = avformat_alloc_context();
    if (!pAVFormatContext) {
        emit signalMessage(tr("avformat_alloc_context: Memory error"));
        return;
    }

    pAVInputFormat = av_find_input_format("x11grab");

    info = tr("%1").arg(mFrameRate);
    mValue = av_dict_set( &pOptions,"framerate",info.toLatin1(),0);

    if(mValue < 0)
    {
        emit signalMessage(tr("av_dict_set: framerate"));
        return;
    }

    info.clear();
    info = tr("%1x%2").arg(mWidth).arg(mHeight);
    mValue = av_dict_set(&pOptions, "video_size", info.toLatin1(), 0);
    if(mValue < 0)
    {
        emit signalMessage(tr("av_dict_set: video_size"));
        return;
    }

    mValue = av_dict_set(&pOptions, "probesize", "32M", 0);
    if (mValue < 0) {
        emit signalMessage(tr("av_dict_set: probesize"));
        return;
    }

    mValue = avformat_open_input(&pAVFormatContext, ":0.0", pAVInputFormat, &pOptions);
    if(mValue != 0)
    {
        emit signalMessage(tr("avformat_open_input: Cannot open input device"));
        return;
    }

    mValue = avformat_find_stream_info(pAVFormatContext,NULL);
    if(mValue < 0)
    {
        emit signalMessage(tr("avformat_find_stream_info: Cannot find stream information"));
        return;
    }

    mVideoStreamIndx = -1;

    for(int i = 0; i < pAVFormatContext->nb_streams; i++ )
    {
        if( pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
        {
            mVideoStreamIndx = i;
            break;
        }
    }

    if( mVideoStreamIndx == -1)
    {
        emit signalMessage(tr("Unable to find the video stream index"));
        return;
    }

    pAVCodecContext = pAVFormatContext->streams[mVideoStreamIndx]->codec;

    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    if( pAVCodec == NULL )
    {
        emit signalMessage(tr("avcodec_find_decoder: could not find [%1] codec").arg(av_get_media_type_string(pAVCodec->type)));
        return;
    }

    mValue = avcodec_open2(pAVCodecContext , pAVCodec , NULL);
    if( mValue < 0 )
    {
        emit signalMessage(tr("avcodec_open2: failed to open decoder for stream #%u").arg(mVideoStreamIndx));
        return;
    }
}

void MiniRecorder::InitOutputFile(QString &fileName)
{
    avformat_alloc_output_context2(&pOutAVFormatContext, NULL, NULL, fileName.toLatin1());
    if (!pOutAVFormatContext)
    {
        emit signalMessage(tr("avformat_alloc_output_context2: could not create output context"));
        return;
    }

    pAVStream = avformat_new_stream(pOutAVFormatContext ,NULL);
    if( !pAVStream )
    {
        emit signalMessage(tr("avformat_alloc_output_context2: could not create output context"));
        return;
    }

    outAVCodecContext = avcodec_alloc_context3(pOutAVCodec);
    if( !outAVCodecContext )
    {
        emit signalMessage(tr("avcodec_alloc_context3: could not allocate output context"));
        return;
    }

    outAVCodecContext = pAVStream->codec;
    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt  = AV_PIX_FMT_YUV420P;
    outAVCodecContext->bit_rate = 512 * 1000; //4096 * 1000
    outAVCodecContext->width = mWidth;
    outAVCodecContext->height = mHeight;
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = mFrameRate;

    pOutAVCodec = avcodec_find_encoder(outAVCodecContext->codec_id);
    if( !pOutAVCodec )
    {
        emit signalMessage(tr("avcodec_find_encoder: could not find encoder for '%1'").arg(avcodec_get_name(outAVCodecContext->codec_id)));
        return;
    }

    if (pOutAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
    {
        outAVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    mValue = avcodec_open2(outAVCodecContext, pOutAVCodec, NULL);
    if( mValue < 0)
    {
        emit signalMessage(tr("avcodec_open2: failed to open output codec"));
        return;
    }

    if ( !(pOutAVFormatContext->flags & AVFMT_NOFILE) )
    {
        if( avio_open2(&pOutAVFormatContext->pb , fileName.toLatin1() , AVIO_FLAG_WRITE ,NULL, NULL) < 0 )
        {
            emit signalMessage(tr("avio_open2: failed to open video file"));
            return;
        }
    }

    if(!pOutAVFormatContext->nb_streams)
    {
        emit signalMessage(tr("Output file does not contain any stream"));
        return;
    }

    mValue = avformat_write_header(pOutAVFormatContext , &pOptions);
    if(mValue < 0)
    {
        emit signalMessage(tr("avformat_write_header: error occurred when opening output file for write"));
        return;
    }

    bRunning = true;
}

void MiniRecorder::RecordDesktop()
{
    int frameFinished;
    int gotPicture;
    uint64_t countFrames = 0;
    uint64_t totalBytes = 0;
    AVPacket outPacket;

    pAVPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(pAVPacket);

    outFrame = av_frame_alloc();
    if(!outFrame)
    {
        emit signalMessage(tr("av_frame_alloc: unable to alloc memory"));
        return;
    }

    int nbytes = av_image_get_buffer_size(outAVCodecContext->pix_fmt,outAVCodecContext->width,outAVCodecContext->height,1);
    uint8_t *video_outbuf = (uint8_t*)av_malloc(nbytes);
    if( video_outbuf == NULL )
    {
        emit signalMessage(tr("av_malloc: unable to alloc memory"));
        return;
    }

    mValue = av_image_fill_arrays( outFrame->data, outFrame->linesize, video_outbuf , AV_PIX_FMT_YUV420P, mWidth, mHeight, 1); // returns : the size in bytes required for src
    if(mValue < 0)
    {
        emit signalMessage(tr("av_image_fill_arrays: error in filling image array"));
    }

    pSwsCtx = sws_getContext(pAVCodecContext->width,
                             pAVCodecContext->height,
                             pAVCodecContext->pix_fmt,
                             outAVCodecContext->width,
                             outAVCodecContext->height,
                             outAVCodecContext->pix_fmt,
                             SWS_BILINEAR, NULL, NULL, NULL);


    while (bRunning)
    {
        if( av_read_frame( pAVFormatContext , pAVPacket ) >= 0 )
        {
            pAVFrame = av_frame_alloc();
            if( !pAVFrame )
            {
                emit signalMessage(tr("av_frame_alloc: unable to alloc memory"));
                break;
            }
            pAVFrame->width = outFrame->width = mWidth;
            pAVFrame->height = outFrame->height = mHeight;
            pAVFrame->format = outFrame->format = AV_PIX_FMT_YUV420P;
            if(pAVPacket->stream_index == mVideoStreamIndx)
            {
                {
                    QMutexLocker locker(&mMutex);
                    mValue = avcodec_decode_video2( pAVCodecContext , pAVFrame , &frameFinished , pAVPacket );
                }
                if( mValue < 0)
                {
                    av_frame_free(&pAVFrame);
                    emit signalMessage(tr("avcodec_decode_video2: decoding failed"));
                    break;
                }

                if(frameFinished)
                {
                    sws_scale(pSwsCtx, pAVFrame->data, pAVFrame->linesize,0, pAVCodecContext->height, outFrame->data,outFrame->linesize);
                    av_init_packet(&outPacket);
                    outPacket.data = NULL;
                    outPacket.size = 0;

                    {
                        QMutexLocker locker(&mMutex);
                        mValue = avcodec_encode_video2(outAVCodecContext , &outPacket ,outFrame , &gotPicture);
                    }
                    if (mValue < 0)
                    {
                        emit signalMessage(tr("avcodec_encode_video2: encoding failed"));
                        break;
                    }

                    if(gotPicture)
                    {
                        countFrames++;
                        if(outPacket.pts != AV_NOPTS_VALUE)
                            outPacket.pts = av_rescale_q(outPacket.pts, pAVStream->codec->time_base, pAVStream->time_base);
                        if(outPacket.dts != AV_NOPTS_VALUE)
                            outPacket.dts = av_rescale_q(outPacket.dts, pAVStream->codec->time_base, pAVStream->time_base);
                        outPacket.pos = -1;
                        totalBytes = totalBytes + outPacket.size;

                        if (countFrames % mFrameRate == 0) {
//                            logger(pOutAVFormatContext, &outPacket, totalBytes);
                            emit signalLogger(pOutAVFormatContext, &outPacket, totalBytes, av_gettime());
                        }

                        if (av_interleaved_write_frame(pOutAVFormatContext, &outPacket) < 0)
                        {
                            emit signalMessage(tr("av_interleaved_write_frame: error muxing packet"));
                            break;
                        }
                    } // gotPicture
                    av_frame_free(&pAVFrame);

                    av_packet_unref(&outPacket);
                } // frameFinished
            }
            av_free_packet(pAVPacket);

            QCoreApplication::processEvents();
        }
    } //bRunning

    av_free(video_outbuf);
    av_frame_free(&outFrame);
    sws_freeContext(pSwsCtx);
}

void MiniRecorder::CloseDesktop()
{
    bRunning = false;

    mValue = av_write_trailer(pOutAVFormatContext);
    if (mValue < 0) {
        emit signalMessage(tr("av_write_trailer: error in writing trailer"));
        return;
    }
    avio_close(pOutAVFormatContext->pb);
    for (unsigned i=0; i<pOutAVFormatContext->nb_streams; i++)
        av_freep(&pOutAVFormatContext->streams[i]);

    avformat_close_input(&pAVFormatContext);
    avformat_free_context(pAVFormatContext);

    avcodec_close(pAVCodecContext);

    avcodec_free_context(&outAVCodecContext);
    avcodec_close(outAVCodecContext);
}

void MiniRecorder::logger(const AVFormatContext *fmt_ctx, const AVPacket *pkt, uint64_t bytes)
{
    char buf1[AV_TS_MAX_STRING_SIZE] = {0};
    char buf2[AV_TS_MAX_STRING_SIZE] = {0};
    char buf3[AV_TS_MAX_STRING_SIZE] = {0};

    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    if (pkt->pts == AV_NOPTS_VALUE) {
        snprintf(buf1, AV_TS_MAX_STRING_SIZE, "NOPTS");
    } else {
        snprintf(buf1, AV_TS_MAX_STRING_SIZE, "%.6g", av_q2d(*time_base) * pkt->pts);
    }

    if (pkt->dts == AV_NOPTS_VALUE) {
        snprintf(buf2, AV_TS_MAX_STRING_SIZE, "NOPTS");
    } else {
        snprintf(buf2, AV_TS_MAX_STRING_SIZE, "%.6g", av_q2d(*time_base) * pkt->dts);
    }
}

