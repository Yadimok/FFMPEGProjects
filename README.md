# FFMPEGProjects

1. ScreenRecorder - Project taken from here (https://github.com/abdullahfarwees/screen-recorder-ffmpeg-cpp).

<b>FFMPEG version:</b>
ffmpeg version 3.2.14 Copyright (c) 2000-2019 the FFmpeg developers

<b>How to compile:</b>

g++ screenrecorder.h screenrecorder.cpp main.cpp -o screenrecorder `pkg-config --libs libavcodec libavformat libswscale libavutil libavdevice`

