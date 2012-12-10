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
void vfg::VideoFrameThumbnail::updateFrameSize()
{
    qDebug() << pixmapLabel->size();
    pixmapLabel->setPixmap(thumb.scaledToWidth(pixmapLabel->width(),
                                               Qt::SmoothTransformation));
    qDebug() << pixmapLabel->size();
}

void vfg::VideoFrameThumbnail::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(thumb.isNull())
        return;

    updateFrameSize();
}

void vfg::VideoFrameThumbnail::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);



    emit selected(1);
}
