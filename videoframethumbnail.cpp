#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QWidget>
#include "videoframethumbnail.h"

using vfg::ui::VideoFrameThumbnail;

VideoFrameThumbnail::VideoFrameThumbnail(
        int frame, QPixmap thumbnail, QWidget *parent) :
    QWidget(parent),
    layout(new QVBoxLayout),
    pixmapLabel(new QLabel),
    thumb(thumbnail),
    frameNumber(frame)
{
    pixmapLabel->setPixmap(thumb);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(pixmapLabel);
    setLayout(layout);
    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void VideoFrameThumbnail::updateFrameSize()
{
    pixmapLabel->setPixmap(thumb.scaledToWidth(pixmapLabel->width(),
                                               Qt::SmoothTransformation));
}

void VideoFrameThumbnail::markSelected()
{
    setStyleSheet("background-color: #e0e0e0; border: 1px solid #dd22ff;");
}

void VideoFrameThumbnail::markUnselected()
{
    setStyleSheet("background-color: inherit; border: 0;");
}

int VideoFrameThumbnail::frameNum() const
{
    return frameNumber;
}

void VideoFrameThumbnail::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit doubleClicked(frameNumber);
}

void VideoFrameThumbnail::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    updateFrameSize();
}

void VideoFrameThumbnail::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit selected(this);
}

void VideoFrameThumbnail::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}
