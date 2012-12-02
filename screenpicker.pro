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
    videoframewidget.cpp

HEADERS  += mainwindow.h \
    flowlayout.h \
    videoframegrabber.h \
    videoframewidget.h

FORMS    += mainwindow.ui

INCLUDEPATH += C:\cpplibs\ffms-2.17-sdk\include
LIBS += -LC:\cpplibs\ffms-2.17-sdk -lffms2 -lole32
