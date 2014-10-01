#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QWidget>
#include "videoframethumbnail.h"

vfg::ui::VideoFrameThumbnail::VideoFrameThumbnail(
        const int frame, const QImage& thumbnail, QWidget *parent) :
    QWidget(parent),
    layout(new QVBoxLayout),
    pixmapLabel(new QLabel),
    thumb(QPixmap::fromImage(thumbnail.scaledToWidth(200, Qt::SmoothTransformation))),
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

void vfg::ui::VideoFrameThumbnail::updateFrameSize()
{
    pixmapLabel->setPixmap(thumb.scaledToWidth(pixmapLabel->width(),
                                               Qt::SmoothTransformation));
}

void vfg::ui::VideoFrameThumbnail::markSelected()
{
    setStyleSheet("background-color: #e0e0e0; border: 1px solid #dd22ff;");
}

void vfg::ui::VideoFrameThumbnail::markUnselected()
{
    setStyleSheet("background-color: inherit; border: 0;");
}

int vfg::ui::VideoFrameThumbnail::frameNum() const
{
    return frameNumber;
}

void vfg::ui::VideoFrameThumbnail::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit doubleClicked(frameNumber);
}

void vfg::ui::VideoFrameThumbnail::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    updateFrameSize();
}

void vfg::ui::VideoFrameThumbnail::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit selected(this);
}

void vfg::ui::VideoFrameThumbnail::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}
