/*
*
* Copyright (C) Yadimok2020
*
* EncDecryptor is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* FrameViewer is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with FrameViewer. If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
**/

#include <iostream>
#include <memory>
#include <GLFW/glfw3.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

const char gFileName[]  = "/home/dmitrii/video.mp4";
const int WIDTH_WINDOW  = 1300;
const int HEIGHT_WINDOW = 1000;
const int FRAME_INDEX   = 750;


int main(int argc, char *argv[])
{
    int window_width = 0, window_height = 0;
    int frame_width = 0, frame_height = 0;
    uint8_t *frame_data = nullptr;

    GLFWwindow *pWindow;

    if (!glfwInit()) {
        std::cout << "glfwInit failed" << std::endl;
        return 1;
    }

    pWindow = glfwCreateWindow(WIDTH_WINDOW, HEIGHT_WINDOW, "FrameViewer", nullptr, nullptr);
    if (!pWindow) {
        std::cout << "glfwCreateWindow failed" << std::endl;
        return 1;
    }

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

    AVFrame *pAVFrameRGB = av_frame_alloc();
    std::shared_ptr<AVFrame *> avFrameBGRDeleter(&pAVFrameRGB, av_frame_free);

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pAVCodecContext->width, pAVCodecContext->height, 1);
    uint8_t *buffer = (uint8_t *)av_malloc(numBytes * sizeof (uint8_t));
    std::shared_ptr<uint8_t> bufferDeleter(buffer, av_free);

    av_image_fill_arrays(pAVFrameRGB->data, pAVFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24,
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
                          pAVCodecContext->height, pAVFrameRGB->data, pAVFrameRGB->linesize);
                if (frameCount++ == FRAME_INDEX)
                {
                    frame_width = pAVCodecContext->width;
                    frame_height = pAVCodecContext->height;
                    break;
                }
            }
        }
    }

    //3 bits per pixel
    frame_data = new uint8_t[frame_width * frame_height * 3]();
    memcpy(frame_data, pAVFrameRGB->data[0], (sizeof (uint8_t) * frame_width * frame_height * 3));


    ///#2
    glfwMakeContextCurrent(pWindow);

    GLuint text_handle;
    glGenTextures(1, &text_handle);
    glBindTexture(GL_TEXTURE_2D, text_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame_data);


    while (!glfwWindowShouldClose(pWindow)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetFramebufferSize(pWindow, &window_width, &window_height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window_width, window_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, text_handle);
        glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex2d(10, 10);
        glTexCoord2d(1, 0); glVertex2d(10 + frame_width, 10);
        glTexCoord2d(1, 1); glVertex2d(10 + frame_width, 10 + frame_height);
        glTexCoord2d(0, 1); glVertex2d(10, 10 + frame_height);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(pWindow);
        glfwWaitEvents();
    }

    delete [] frame_data;

    return 0;
}
