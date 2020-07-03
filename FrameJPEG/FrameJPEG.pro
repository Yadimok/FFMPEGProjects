QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp

LIBS += -L/usr/local/lib -lavdevice -lXv -lX11 -lXext -ldl -lxcb -lxcb-shm -lX11 -lsndio -lasound -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lm -llzma -lbz2 -lz -pthread -lavfilter -lXv -lX11 -lXext -ldl -lxcb -lxcb-shm -lX11 -lsndio -lasound -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lm -llzma -lbz2 -lz -pthread -lswscale -lm -lavformat -lXv -lX11 -lXext -ldl -lxcb -lxcb-shm -lX11 -lsndio -lasound -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lm -llzma -lbz2 -lz -pthread -lavcodec -lXv -lX11 -lXext -ldl -lxcb -lxcb-shm -lX11 -lsndio -lasound -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lm -llzma -lbz2 -lz -pthread -lswresample -lm -lavutil -lm
LIBS += -ljpeg
