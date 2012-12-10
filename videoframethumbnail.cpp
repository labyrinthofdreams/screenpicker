#include <QtGui>
#include <QDebug>
#include "videoframethumbnail.h"

vfg::VideoFrameThumbnail::VideoFrameThumbnail(QWidget *parent) :
    QWidget(parent)
{
    pixmapLabel = new QLabel;
//    pixmapLabel->setSizePolicy(QSizePolicy::Ignored,
//                               QSizePolicy::Ignored);
    layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(pixmapLabel);
    setLayout(layout);
    setFixedWidth(100);
    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void vfg::VideoFrameThumbnail::setThumbnail(QPixmap thumbnail)
{
    thumb = thumbnail;
    updateFrameSize();
}

void vfg::VideoFrameThumbnail::setFrameNumber(unsigned frame)
{
    frameNumber = frame;
}

void vfg::VideoFrameThumbnail::updateFrameSize()
{
    qDebug() << pixmapLabel->size();
    pixmapLabel->setPixmap(thumb.scaledToWidth(pixmapLabel->width(),
                                               Qt::SmoothTransformation));
    qDebug() << pixmapLabel->size();
}

void vfg::VideoFrameThumbnail::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit selected(frameNumber);
}

void vfg::VideoFrameThumbnail::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(thumb.isNull())
        return;

    updateFrameSize();
}

void vfg::VideoFrameThumbnail::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    setStyleSheet("background-color: #e0e0e0; border: 1px solid #dd22ff;");

    emit selected(frameNumber);
}

void vfg::VideoFrameThumbnail::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}
