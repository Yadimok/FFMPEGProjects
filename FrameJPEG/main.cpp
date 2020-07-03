/*
*
* Copyright (C) Yadimok2020
*
* FrameJPEG is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* FrameJPEG is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with FrameJPEG. If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
**/

#include <iostream>
#include <memory>
#include <fstream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <jpeglib.h>
}

void SaveToJPEG(uint8_t *data, int width, int height, int counter);

const char gFileName[]  = "/home/XXXXX/video.mp4";


int main(int argc, char *argv[])
{
    ///#1
    av_register_all();

    AVFormatContext *pAVFormatContext = nullptr;
    if (avformat_open_input(&pAVFormatContext, gFileName, nullptr, nullptr) != 0) {
        std::cout << "avformat_open_input failed" << std::endl;
        return 1;
    }
    std::shared_ptr<AVFormatContext *> avFormatContextDeleter(&pAVFormatContext, avformat_close_input);

    if (avformat_find_stream_info(pAVFormatContext, nullptr) < 0) {
        std::cout << "avformat_find_stream_info failed" << std::endl;
        return 1;
    }

    int videoStream = -1;
    for (decltype (pAVFormatContext->nb_streams) i=0; i<pAVFormatContext->nb_streams; ++i) {
        if (pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1) {
        std::cout << "videoStream failed" << std::endl;
        return 1;
    }

    AVCodecParameters *pAVCodecParams = pAVFormatContext->streams[videoStream]->codecpar;
    AVCodec *pAVCodec = avcodec_find_decoder(pAVCodecParams->codec_id);
    if (pAVCodec == nullptr) {
        std::cout << "avcodec_find_decoder failed" << std::endl;
        return 1;
    }

    AVCodecContext *pAVCodecContext = avcodec_alloc_context3(nullptr);
    if (avcodec_parameters_to_context(pAVCodecContext, pAVCodecParams) < 0) {
        std::cout << "avcodec_parameters_to_context failed" << std::endl;
        return 1;
    }
    std::shared_ptr<AVCodecContext> avCodecContextDeleter(pAVCodecContext, avcodec_close);

    if (avcodec_open2(pAVCodecContext, pAVCodec, nullptr) < 0) {
        std::cout << "avcodec_open2 failed" << std::endl;
        return 1;
    }

    AVFrame *pAVFrame = av_frame_alloc();
    std::shared_ptr<AVFrame *> avFrameDeleter(&pAVFrame, av_frame_free);

    AVFrame *pFrameBGR = av_frame_alloc();
    std::shared_ptr<AVFrame *> avFrameBGRDeleter(&pFrameBGR, av_frame_free);

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pAVCodecContext->width, pAVCodecContext->height, 1);
    uint8_t *buffer = (uint8_t *)av_malloc(numBytes * sizeof (uint8_t));
    std::shared_ptr<uint8_t> bufferDeleter(buffer, av_free);

    av_image_fill_arrays(pFrameBGR->data, pFrameBGR->linesize, buffer, AV_PIX_FMT_RGB24,
                         pAVCodecContext->width, pAVCodecContext->height, 1);

    SwsContext *pSwsContext = sws_getContext(pAVCodecContext->width, pAVCodecContext->height,
                                             pAVCodecContext->pix_fmt, pAVCodecContext->width, pAVCodecContext->height,
                                             AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
    std::shared_ptr<SwsContext> swsCtxDeleter(pSwsContext, sws_freeContext);

    AVPacket packet;
    av_init_packet(&packet);

    int frameCount = 0;
    while (av_read_frame(pAVFormatContext, &packet) >= 0)
    {
        std::shared_ptr<AVPacket> packedtDeleter(&packet, av_packet_unref);
        if (packet.stream_index == videoStream)
        {
            avcodec_send_packet(pAVCodecContext, &packet);
            if (avcodec_receive_frame(pAVCodecContext, pAVFrame) == 0)
            {
                sws_scale(pSwsContext, pAVFrame->data, pAVFrame->linesize, 0,
                          pAVCodecContext->height, pFrameBGR->data, pFrameBGR->linesize);

                if (frameCount == 100)
                    SaveToJPEG(pFrameBGR->data[0], pAVCodecContext->width, pAVCodecContext->height, frameCount);

                frameCount++;
            }
        }
    }
    return 0;
}

void SaveToJPEG(uint8_t *data, int width, int height, int counter)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    JSAMPROW row_pointer[1];

    char fileName[32];
    sprintf(fileName, "video%4d.jpeg", counter);

    /* Step 1: allocate and initialize JPEG compression object */
    cinfo.err = jpeg_std_error( &jerr );
    jpeg_create_compress(&cinfo);

    /* Step 2: Setting the parameters of the output file here */
    FILE *outfile = fopen( fileName, "wb" );
    if ( !outfile )
    {
        printf("Error opening output jpeg file %s\n!", fileName);
        return;
    }
    jpeg_stdio_dest(&cinfo, outfile);

    /* Step 3: set parameters for compression */
    cinfo.image_width       = width;
    cinfo.image_height      = height;
    cinfo.input_components  = 3;
    cinfo.in_color_space    = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    cinfo.num_components    = 3;
    cinfo.dct_method        = JDCT_ISLOW;
    jpeg_set_quality(&cinfo, 100, TRUE);

    /* Step 4: Start compressor */
    jpeg_start_compress(&cinfo, TRUE);

    /* Step 5: while (scan lines remain to be written) */
    while(cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &data[cinfo.next_scanline * cinfo.image_width * cinfo.num_components];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    /* Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);

    /* Step 7: release JPEG compression object */
    jpeg_destroy_compress(&cinfo);

    fclose(outfile);
}
