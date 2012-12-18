#-------------------------------------------------
#
# Project created by QtCreator 2012-11-21T13:24:25
#
#-------------------------------------------------

QT       += core gui

TARGET = screenpicker
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    flowlayout.cpp \
    videoframegrabber.cpp \
    videoframewidget.cpp \
    videoframethumbnail.cpp \
    thumbnailcontainer.cpp \
    avisynthvideosource.cpp \
    scripteditor.cpp

HEADERS  += mainwindow.h \
    flowlayout.h \
    videoframegrabber.h \
    videoframewidget.h \
    videoframethumbnail.h \
    thumbnailcontainer.h \
    abstractvideosource.h \
    avisynthvideosource.h \
    scripteditor.h

FORMS    += mainwindow.ui \
    scripteditor.ui

INCLUDEPATH += C:\cpplibs\avs2yuv-0.24bm2\avs2yuv\src
#LIBS += -LC:\cpplibs\ffms-2.17-sdk -lffms2 -lole32

# These are needed for Avisynth compilation
QMAKE_CXXFLAGS += -fpermissive -O3 -ffast-math -Wall -Wshadow -Wempty-body -I. -fomit-frame-pointer -s -fno-tree-vectorize -fno-zero-initialized-in-bss
DEFINES -= UNICODE

OTHER_FILES += \
    default.avs

RESOURCES += \
    resources.qrc
