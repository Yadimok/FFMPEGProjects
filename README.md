# FFMPEGProjects:

1. <b>ScreenRecorder</b> - Project taken from here (https://github.com/abdullahfarwees/screen-recorder-ffmpeg-cpp).

`FFMPEG version: ffmpeg version 3.2.14 Copyright (c) 2000-2019 the FFmpeg developers`

<b>How to compile:</b>
```
g++ screenrecorder.h screenrecorder.cpp main.cpp -o screenrecorder `pkg-config --libs libavcodec libavformat libswscale libavutil libavdevice`
```
2. <b>FrameViewer</b> - In the code set the frame manually and show the picture using `GLFW`.

`FFMPEG version: ffmpeg version 4.2.3 Copyright (c) 2000-2020 the FFmpeg developers`

3. <b>FrameBMP</b> - In the code set the frame manually and save the image in `BMP` format.

`FFMPEG version: ffmpeg version 4.2.3 Copyright (c) 2000-2020 the FFmpeg developers`

4. <b>FrameJPEG</b> - In the code set the frame manually and save the image in `JPG` format using `jpeglib`.

`FFMPEG version: ffmpeg version 4.2.3 Copyright (c) 2000-2020 the FFmpeg developers`

5. <b>FramePlayOpenGL</b> - Playing videos using `OpenGL`

`FFMPEG version: ffmpeg version 4.2.3 Copyright (c) 2000-2020 the FFmpeg developers`
