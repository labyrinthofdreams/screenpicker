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
    scripteditor.cpp \
    configdialog.cpp \
    videoframegenerator.cpp \
    dvdprocessor.cpp \
    scriptparserfactory.cpp \
    dgindexscriptparser.cpp
    avisynthscriptparser.cpp \
    scriptparser.cpp \
    defaultscriptparser.cpp

HEADERS  += mainwindow.h \
    flowlayout.h \
    videoframegrabber.h \
    videoframewidget.h \
    videoframethumbnail.h \
    thumbnailcontainer.h \
    abstractvideosource.h \
    avisynthvideosource.h \
    scripteditor.h \
    configdialog.h \
    init.h \
    videoframegenerator.h \
    dvdprocessor.h \
    scriptparserfactory.h \
    scriptparser.h \
    dgindexscriptparser.h
    avisynthscriptparser.h \
    defaultscriptparser.h

FORMS    += mainwindow.ui \
    scripteditor.ui \
    configdialog.ui

INCLUDEPATH += C:\cpplibs\avs2yuv-0.24bm2\avs2yuv\src

# These are needed for Avisynth compilation
QMAKE_CXXFLAGS += -fpermissive
DEFINES -= UNICODE

OTHER_FILES += \
    d2v_template.avs \
    default_template.avs

RESOURCES += \
    resources.qrc
