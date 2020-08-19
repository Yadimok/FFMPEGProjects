QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    minirecorder.cpp

HEADERS += \
    mainwindow.h \
    minirecorder.h

FORMS += \
    mainwindow.ui

QMAKE_CXXFLAGS += -D__STDC_CONSTANT_MACROS

LIBS += -pthread
LIBS += -L/usr/local/lib
LIBS += -lavdevice
LIBS += -lavfilter
LIBS += -lpostproc
LIBS += -lavformat
LIBS += -lavcodec
LIBS += -ldl
LIBS += -lXfixes
LIBS += -lXext
LIBS += -lX11
LIBS += -lasound
LIBS += -lx264
LIBS += -lvpx
LIBS += -lvorbisenc
LIBS += -lvorbis
LIBS += -logg
LIBS += -lmp3lame
LIBS += -lz
LIBS += -lrt
LIBS += -lswresample
LIBS += -lswscale
LIBS += -lavutil
LIBS += -lm
LIBS += -lSDL2
LIBS += -lXv
LIBS += -lxcb
LIBS += -lxcb-shm
LIBS += -lsndio
LIBS += -lopus
LIBS += -lasound
LIBS += -lfreetype
LIBS += -lva
LIBS += -lx265
LIBS += -lass
LIBS += -lvdpau
LIBS += -lxcb-xfixes
LIBS += -lxcb-render
LIBS += -lxcb-shape
LIBS += -lva-drm
LIBS += -lva-x11

RESOURCES += \
    res.qrc
