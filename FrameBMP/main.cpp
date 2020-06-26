/*
*
* Copyright (C) Yadimok2020
*
* FrameBMP is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* FrameBMP is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with FrameBMP. If not, write to
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
}

typedef int             LONG;
typedef unsigned char   BYTE;
typedef unsigned int    DWORD;
typedef unsigned short  WORD;

typedef struct tagBITMAPFILEHEADER {
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} __attribute__((packed)) BITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    BITMAPFILEHEADER bmfHeader;
} __attribute__((packed)) BITMAPINFO;

void SaveToBitmap(uint8_t *data, int width, int height, int bpp);

const char gFileName[]  = "/home/Yadimok/video.mp4";
const int FRAME_INDEX   = 750;


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

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, pAVCodecContext->width, pAVCodecContext->height, 1);
    uint8_t *buffer = (uint8_t *)av_malloc(numBytes * sizeof (uint8_t));
    std::shared_ptr<uint8_t> bufferDeleter(buffer, av_free);

    av_image_fill_arrays(pFrameBGR->data, pFrameBGR->linesize, buffer, AV_PIX_FMT_BGR24,
                         pAVCodecContext->width, pAVCodecContext->height, 1);

    SwsContext *pSwsContext = sws_getContext(pAVCodecContext->width, pAVCodecContext->height,
                                             pAVCodecContext->pix_fmt, pAVCodecContext->width, pAVCodecContext->height,
                                             AV_PIX_FMT_BGR24, SWS_BILINEAR, nullptr, nullptr, nullptr);
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
                if (frameCount++ == FRAME_INDEX)
                {
                    ///#2
                    SaveToBitmap(pFrameBGR->data[0], pAVCodecContext->width, pAVCodecContext->height, 24);
                    break;
                }
            }
        }
    }
    return 0;
}

void SaveToBitmap(uint8_t *data, int width, int height, int bpp)
{
    BITMAPFILEHEADER bmpHeader = { 0 };
    bmpHeader.bfType = ('M' << 8) | 'B';
    bmpHeader.bfReserved1 = 0;
    bmpHeader.bfReserved2 = 0;
    bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpHeader.bfSize = bmpHeader.bfOffBits + width*height*(bpp / 8);

    BITMAPINFO bmpInfo = { 0 };
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = width;
    bmpInfo.bmiHeader.biHeight = -height; // reverse image
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = bpp;
    bmpInfo.bmiHeader.biCompression = 0;
    bmpInfo.bmiHeader.biSizeImage = 0;
    bmpInfo.bmiHeader.biXPelsPerMeter = 100;
    bmpInfo.bmiHeader.biYPelsPerMeter = 100;
    bmpInfo.bmiHeader.biClrUsed = 0;
    bmpInfo.bmiHeader.biClrImportant = 0;

         // open a file
    std::ofstream fout("video.bmp", std::ofstream::out | std::ofstream::binary);
    if (!fout)
    {
        return;
    }
         // Close after use ends
    std::shared_ptr<std::ofstream> foutCloser(&fout, [](std::ofstream *f){ f->close(); });

    fout.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(BITMAPFILEHEADER));
    fout.write(reinterpret_cast<const char*>(&bmpInfo.bmiHeader), sizeof(BITMAPINFOHEADER));
    fout.write(reinterpret_cast<const char*>(data), width * height * (bpp / 8));
}



