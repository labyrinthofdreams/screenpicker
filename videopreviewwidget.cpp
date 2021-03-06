#include <QImage>
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QPair>
#include <QPalette>
#include <QPixmap>
#include <QRect>
#include <QResizeEvent>
#include <QSize>
#include <QVBoxLayout>
#include <QVector>
#include <QVideoWidget>
#include <QWidget>
#include "videopreviewwidget.h"

vfg::ui::VideoPreviewWidget::VideoPreviewWidget(QWidget *parent) :
    QWidget(parent),
    layout(new QVBoxLayout),
    frameLabel(new QLabel),
    videoWidget(new QVideoWidget)
{
    frameLabel->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    // If size policy is not ignored,
    // it prevents the resizeEvent() from being called properly
    // when the widget is resized smaller
    frameLabel->setSizePolicy(QSizePolicy::Ignored,
                              QSizePolicy::Ignored);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(frameLabel.get());
    setLayout(layout.get());

    QPalette plt = palette();
    plt.setColor(QPalette::Window, Qt::white);
    setPalette(plt);
    videoWidget->setPalette(plt);

    setAutoFillBackground(true);
    setContentsMargins(0, 0, 0, 0);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void vfg::ui::VideoPreviewWidget::showVideo()
{
    layout->removeWidget(frameLabel.get());
    frameLabel->setParent(0);
    layout->addWidget(videoWidget.get());
    state = VideoState::Playing;
}

void vfg::ui::VideoPreviewWidget::hideVideo()
{
    layout->removeWidget(videoWidget.get());
    videoWidget->setParent(0);
    layout->addWidget(frameLabel.get());
    state = VideoState::Stopped;
}

void vfg::ui::VideoPreviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

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
    Q_UNUSED(frameNum);

    setFrame(frame);
}

void vfg::ui::VideoPreviewWidget::setCrop(const QRect& area)
{
    if(area == QRect{}) {
        framePixmap = original.copy();
        cropBorders.clear();
    }
    else {
        const QRect left(0, 0, area.left(), framePixmap.height());
        const QRect top(0, 0, framePixmap.width(), area.top());
        const QRect right(framePixmap.width() - area.width(), 0,
                            area.width(), framePixmap.height());
        const QRect bottom(0, framePixmap.height() - area.height(),
                            framePixmap.width(), area.height());
        QVector<QRect> newBorders {left, top, right, bottom};
        cropBorders.swap(newBorders);
    }

    updateFrame();

    // Prevent widget from disappearing
    adjustSize();
}

void vfg::ui::VideoPreviewWidget::updateFrame()
{
    if(state == VideoState::Playing) {
        layout->setGeometry(calculateSize());
        if(zoomMode == ZoomMode::Zoom_Scale) {
            layout->invalidate();
        }
    }
    else {
        if(framePixmap.isNull()) {
            return;
        }

        if(!cropBorders.isEmpty()) {
            drawCropArea();
        }

        frameLabel->setPixmap(framePixmap.scaled(calculateSize().size(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation));
    }
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

QRect vfg::ui::VideoPreviewWidget::calculateSize() const
{
    const QRect geometry = layout->geometry();
    if(zoomMode == ZoomMode::Zoom_Scale) {
        return geometry;
    }

    static const QMap<ZoomMode, double> factors {
        {ZoomMode::Zoom_25, 0.25}, {ZoomMode::Zoom_50, 0.5},
        {ZoomMode::Zoom_100, 1.0}, {ZoomMode::Zoom_200, 2.0}
    };

    const auto zoomfactor = factors.value(zoomMode);
    return {geometry.x(), geometry.y(),
                static_cast<int>(original.width() * zoomfactor),
            static_cast<int>(original.height() * zoomfactor)};
}

void vfg::ui::VideoPreviewWidget::setZoom(const ZoomMode mode)
{
    zoomMode = mode;

    updateFrame();
}
