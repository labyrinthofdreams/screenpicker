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

vfg::ui::VideoPreviewWidget::VideoPreviewWidget(QWidget *parent) :
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

    QPalette plt = palette();
    plt.setColor(QPalette::Window, Qt::white);
    setPalette(plt);

    setAutoFillBackground(true);
    setContentsMargins(0, 0, 0, 0);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void vfg::ui::VideoPreviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(framePixmap.isNull()) {
        return;
    }

    updateFrame();
}

void vfg::ui::VideoPreviewWidget::setFrame(const QImage& img)
{
    original = QPixmap::fromImage(img);
    framePixmap = original.copy();
    updateFrame();
}

void vfg::ui::VideoPreviewWidget::setFrame(const int frameNum, const QImage& frame)
{
    setFrame(frame);
}

void vfg::ui::VideoPreviewWidget::setCrop(const QRect& area)
{
    const QRect left(0, 0, area.left(), framePixmap.height());
    const QRect top(0, 0, framePixmap.width(), area.top());
    const QRect right(framePixmap.width() - area.width(), 0,
                        area.width(), framePixmap.height());
    const QRect bottom(0, framePixmap.height() - area.height(),
                        framePixmap.width(), area.height());

    QVector<QRect> newBorders {left, top, right, bottom};
    cropBorders.swap(newBorders);

    updateFrame();
}

void vfg::ui::VideoPreviewWidget::resetCrop()
{
    framePixmap = original.copy();
    cropBorders.clear();
    updateFrame();
}

void vfg::ui::VideoPreviewWidget::updateFrame()
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

void vfg::ui::VideoPreviewWidget::drawCropArea()
{
    QPixmap copiedFrame = original.copy();
    QPainter painter(&copiedFrame);
    painter.setBrush(Qt::cyan);
    // Performs an inverse operation which works well with cyan
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setOpacity(0.6);
    painter.setPen(Qt::NoPen);
    painter.drawRects(cropBorders);
    framePixmap.swap(copiedFrame);
}

QSize vfg::ui::VideoPreviewWidget::calculateSize() const
{
    if(zoomMode == ZoomMode::Zoom_Scale) {
        return frameLabel->size();
    }

    const std::map<ZoomMode, double> factors {
        {ZoomMode::Zoom_25, 0.25}, {ZoomMode::Zoom_50, 0.5},
        {ZoomMode::Zoom_100, 1.0}, {ZoomMode::Zoom_200, 2.0},
        {ZoomMode::Zoom_Scale, static_cast<double>(frameLabel->height()) / original.height()}
    };

    const auto zoomfactor = factors.at(zoomMode);
    return {static_cast<int>(original.width() * zoomfactor),
            static_cast<int>(original.height() * zoomfactor)};
}

void vfg::ui::VideoPreviewWidget::setZoom(const ZoomMode mode)
{
    zoomMode = mode;

    updateFrame();
}
