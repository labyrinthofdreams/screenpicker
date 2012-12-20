#include <QtGui>
#include <QDebug>
#include "videoframethumbnail.h"

vfg::VideoFrameThumbnail::VideoFrameThumbnail(unsigned frame, QPixmap thumbnail, QWidget *parent) :
    QWidget(parent)
{
    frameNumber = frame;
    thumb = thumbnail;
    pixmapLabel = new QLabel;
    pixmapLabel->setPixmap(thumb);
    layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(pixmapLabel);
    setLayout(layout);
    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void vfg::VideoFrameThumbnail::updateFrameSize()
{
    pixmapLabel->setPixmap(thumb.scaledToWidth(pixmapLabel->width(),
                                               Qt::SmoothTransformation));
}

void vfg::VideoFrameThumbnail::markSelected()
{
    setStyleSheet("background-color: #e0e0e0; border: 1px solid #dd22ff;");
}

void vfg::VideoFrameThumbnail::markUnselected()
{
    setStyleSheet("background-color: inherit; border: 0;");
}

unsigned vfg::VideoFrameThumbnail::frameNum() const
{
    return frameNumber;
}

void vfg::VideoFrameThumbnail::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit doubleClicked(this);
}

void vfg::VideoFrameThumbnail::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    updateFrameSize();
}

void vfg::VideoFrameThumbnail::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit selected(this);
}

void vfg::VideoFrameThumbnail::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}
