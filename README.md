# FFMPEGProjects

1. ScreenRecorder - Project taken from here (https://github.com/abdullahfarwees/screen-recorder-ffmpeg-cpp).

<b>FFMPEG version:</b>
ffmpeg version 3.2.14 Copyright (c) 2000-2019 the FFmpeg developers

built with gcc 8 (Ubuntu 8.4.0-3ubuntu2)

configuration:

libavutil      55. 34.101 / 55. 34.101
libavcodec     57. 64.101 / 57. 64.101
libavformat    57. 56.101 / 57. 56.101 
libavdevice    57.  1.100 / 57.  1.100 
libavfilter     6. 65.100 /  6. 65.100 
libswscale      4.  2.100 /  4.  2.100 
libswresample   2.  3.100 /  2.  3.100

<b>How to compile:</b>

g++ screenrecorder.h screenrecorder.cpp main.cpp -o screenrecorder `pkg-config --libs libavcodec libavformat libswscale libavutil libavdevice`

