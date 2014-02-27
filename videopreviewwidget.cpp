#include <map>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPair>
#include <QPalette>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include "videopreviewwidget.h"

using vfg::ZoomMode;
using vfg::ui::VideoPreviewWidget;

VideoPreviewWidget::VideoPreviewWidget(QWidget *parent) :
    QWidget(parent),
    cropBorders(),
    framePixmap(),
    original(),
    zoomMode(ZoomMode::Zoom_Scale)
{
    frameLabel = new QLabel;
    frameLabel->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    // If size policy is not ignored,
    // it prevents the resizeEvent() from being called properly
    // when the widget is resized smaller
    frameLabel->setSizePolicy(QSizePolicy::Ignored,
                              QSizePolicy::Ignored);

    layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(frameLabel);
    setLayout(layout);

    setAutoFillBackground(true);
    QPalette plt = palette();
    plt.setColor(QPalette::Window, Qt::white);
    setPalette(plt);

    setContentsMargins(0, 0, 0, 0);

    // Enable context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void VideoPreviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(framePixmap.isNull())
        return;

    updateFrame();
}

void VideoPreviewWidget::setFrame(QImage img)
{
    original = QPixmap::fromImage(img);
    framePixmap = original.copy();
    updateFrame();
}

void VideoPreviewWidget::setFrame(QPair<int, QImage> img)
{
    original = QPixmap::fromImage(img.second);
    framePixmap = original.copy();
    updateFrame();
}

void VideoPreviewWidget::setCrop(QRect area)
{
    QRect left {0, 0, area.left(), framePixmap.height()};
    QRect top {0, 0, framePixmap.width(), area.top()};
    QRect right {framePixmap.width() - area.width(), 0,
                area.width(), framePixmap.height()};
    QRect bottom {0, framePixmap.height() - area.height(),
                 framePixmap.width(), area.height()};

    QVector<QRect> newBorders{left, top, right, bottom};
    cropBorders.swap(newBorders);

    updateFrame();
}

void VideoPreviewWidget::resetCrop()
{
    framePixmap = original.copy();
    cropBorders.clear();
    updateFrame();
}

void VideoPreviewWidget::updateFrame()
{
    if(framePixmap.isNull())
    {
        return;
    }

    if(!cropBorders.isEmpty())
    {
        drawCropArea();
    }

    frameLabel->setPixmap(framePixmap.scaled(calculateSize(),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));

}

void VideoPreviewWidget::drawCropArea()
{
    QPixmap copiedFrame = original.copy();
    QPainter painter {&copiedFrame};
    painter.setBrush(Qt::cyan);
    // Performs an inverse operation which works well with cyan
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setOpacity(0.6);
    painter.setPen(Qt::NoPen);
    painter.drawRects(cropBorders);
    framePixmap.swap(copiedFrame);
}

const QSize VideoPreviewWidget::calculateSize() const
{
    if(zoomMode == ZoomMode::Zoom_Scale) {
        return frameLabel->size();
    }

    std::map<ZoomMode, double> factors {
        {ZoomMode::Zoom_25, 0.25}, {ZoomMode::Zoom_50, 0.5},
        {ZoomMode::Zoom_100, 1.0}, {ZoomMode::Zoom_200, 2.0},
        {ZoomMode::Zoom_Scale, static_cast<double>(frameLabel->height()) / original.height()}
    };

    const auto zoomfactor = factors[zoomMode];
    return QSize {static_cast<int>(original.width() * zoomfactor),
                  static_cast<int>(original.height() * zoomfactor)};
}

void VideoPreviewWidget::setZoom(ZoomMode mode)
{
    zoomMode = mode;

    updateFrame();
}
