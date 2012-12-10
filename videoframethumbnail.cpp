#include <QtGui>
#include "videoframethumbnail.h"

vfg::VideoFrameThumbnail::VideoFrameThumbnail(QWidget *parent) :
    QWidget(parent)
{
    layout = new QVBoxLayout;
    pixmapLabel = new QLabel;
    layout->addWidget(pixmapLabel);
    setLayout(layout);
}

void vfg::VideoFrameThumbnail::setThumbnail(QPixmap thumbnail)
{
    pixmapLabel->setPixmap(thumbnail);
}

void vfg::VideoFrameThumbnail::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);



    emit selected(1);
}
