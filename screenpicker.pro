#-------------------------------------------------
#
# Project created by QtCreator 2012-11-21T13:24:25
#
#-------------------------------------------------

QT       += core gui widgets concurrent multimedia multimediawidgets script

TARGET = screenpicker
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    flowlayout.cpp \
    videoframegrabber.cpp \
    videoframethumbnail.cpp \
    thumbnailcontainer.cpp \
    avisynthvideosource.cpp \
    scripteditor.cpp \
    configdialog.cpp \
    videoframegenerator.cpp \
    dvdprocessor.cpp \
    scriptparser.cpp \
    videosettingswidget.cpp \
    videopreviewwidget.cpp \
    .\libs\templet\templet.cpp .\libs\templet\nodes.cpp .\libs\templet\types.cpp \
    avisynthwrapper.cpp \
    gifmakerwidget.cpp \
    aboutwidget.cpp \
    x264encoderdialog.cpp \
    opendialog.cpp \
    httpdownload.cpp \
    downloadsdialog.cpp \
    progressbardelegate.cpp \
    downloadslistmodel.cpp \
    extractorfactory.cpp \
    extractors/dailymotionextractor.cpp \
    extractors/baseextractor.cpp \
    extractors/youtubeextractor.cpp \
    extractors/instagramextractor.cpp

HEADERS  += mainwindow.h \
    flowlayout.h \
    videoframegrabber.h \
    videoframethumbnail.h \
    thumbnailcontainer.h \
    abstractvideosource.h \
    avisynthvideosource.h \
    scripteditor.h \
    configdialog.h \
    init.h \
    videoframegenerator.h \
    dvdprocessor.h \
    scriptparser.h \
    videosettingswidget.h \
    videopreviewwidget.h \
    ptrutil.hpp \
    avisynthwrapper.hpp \
    gifmakerwidget.hpp \
    aboutwidget.hpp \
    x264encoderdialog.hpp \
    raiideleter.hpp \
    opendialog.hpp \
    httpdownload.hpp \
    downloadsdialog.hpp \
    progressbardelegate.hpp \
    downloadslistmodel.hpp \
    extractorfactory.hpp \
    extractors/baseextractor.hpp \
    extractors/dailymotionextractor.hpp \
    extractors/youtubeextractor.hpp \
    extractors/instagramextractor.hpp

FORMS    += mainwindow.ui \
    scripteditor.ui \
    configdialog.ui \
    videosettingswidget.ui \
    gifmakerwidget.ui \
    aboutwidget.ui \
    x264encoderdialog.ui \
    opendialog.ui \
    downloadsdialog.ui

INCLUDEPATH += .\libs\avs2yuv\src \
    .\libs\templet

QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -O3 -fpermissive

# Required for avisynth to compile without using wide characters
DEFINES -= UNICODE

OTHER_FILES += \
    d2v_template.avs \
    default_template.avs

RESOURCES += \
    resources.qrc
