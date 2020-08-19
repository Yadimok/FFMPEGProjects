#ifndef MINIRECORDER_H
#define MINIRECORDER_H

#include <QObject>
#include <QCoreApplication>
#include <QMutex>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>

#define __STDC_CONSTANT_MACROS

//FFMPEG LIBRARIES
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"

#include "libavdevice/avdevice.h"

#include "libavfilter/avfilter.h"
//#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

#include "libavformat/avformat.h"
#include "libavformat/avio.h"

#include "libavutil/opt.h"
#include "libavutil/common.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/file.h"
#include "libavutil/hwcontext.h"
#include "libavutil/timestamp.h"

#include "libswscale/swscale.h"
}


class MiniRecorder : public QObject
{
    Q_OBJECT
public:
    explicit MiniRecorder(QObject *parent = nullptr);
    ~MiniRecorder();

    void Init(const int width, const int height, const int framerate);
    void OpenDesktop();
    void InitOutputFile(QString &fileName);
    void RecordDesktop();
    void CloseDesktop();

signals:
    void signalMessage(QString message);
    void signalLogger(const AVFormatContext *fmt_ctx, const AVPacket *pkt, uint64_t bytes, int64_t datetime);

private:
    std::atomic<bool> bRunning;

    AVInputFormat *pAVInputFormat;
    AVOutputFormat *pAVOutputFormat;

    AVCodecContext *pAVCodecContext;
    AVCodecContext *outAVCodecContext;

    AVFormatContext *pAVFormatContext;
    AVFormatContext *pOutAVFormatContext;

    AVFrame *pAVFrame;
    AVFrame *outFrame;

    AVCodec *pAVCodec;
    AVCodec *pOutAVCodec;

    AVPacket *pAVPacket;

    AVDictionary *pOptions;

    AVStream *pAVStream;
    SwsContext* pSwsCtx;

    int mValue;
    int mVideoStreamIndx;

    int mFrameRate;
    int mWidth;
    int mHeight;

    QMutex mMutex;

    static void logger(const AVFormatContext *fmt_ctx, const AVPacket *pkt, uint64_t bytes);
};

#endif // MINIRECORDER_H
