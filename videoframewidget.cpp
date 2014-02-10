#include <map>
#include <QtWidgets>
#include <QMouseEvent>
#include <QPair>
#include <QVector>
#include "videoframewidget.h"

vfg::VideoFrameWidget::VideoFrameWidget(QWidget *parent) :
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

void vfg::VideoFrameWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(framePixmap.isNull())
        return;

    updateFrame();
}

void vfg::VideoFrameWidget::mousePressEvent(QMouseEvent *event)
{
    // Enable context menu only on right-click
    if(event->button() != Qt::RightButton) {
        event->ignore();
        return;
    }

    event->accept();
}

void vfg::VideoFrameWidget::setFrame(QImage img)
{
    original = QPixmap::fromImage(img);
    framePixmap = original.copy();
    updateFrame();
}

void vfg::VideoFrameWidget::setFrame(QPair<unsigned, QImage> img)
{
    original = QPixmap::fromImage(img.second);
    framePixmap = original.copy();
    updateFrame();
}

void vfg::VideoFrameWidget::setCrop(QRect area)
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

void vfg::VideoFrameWidget::resetCrop()
{
    framePixmap = original.copy();
    cropBorders.clear();
    updateFrame();
}

void vfg::VideoFrameWidget::updateFrame()
{
    if(framePixmap.isNull())
    {
        return;
    }

    if(!cropBorders.isEmpty())
    {
        drawCropArea();
    }

    QSize zoomSize {};
    if(zoomMode == ZoomMode::Zoom_Scale) {
        zoomSize = frameLabel->size();
    }
    else {
        const auto zoomfactor = getZoomFactor();
        zoomSize = QSize{static_cast<int>(original.width() * zoomfactor),
                         static_cast<int>(original.height() * zoomfactor)};
    }

    frameLabel->setPixmap(framePixmap.scaled(zoomSize,
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));

}

void vfg::VideoFrameWidget::drawCropArea()
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

double vfg::VideoFrameWidget::getZoomFactor() const
{
    std::map<ZoomMode, double> factors {
        {ZoomMode::Zoom_25, 0.25}, {ZoomMode::Zoom_50, 0.5},
        {ZoomMode::Zoom_100, 1.0}, {ZoomMode::Zoom_200, 2.0},
        {ZoomMode::Zoom_Scale, static_cast<double>(frameLabel->height()) / original.height()}
    };

    return factors[zoomMode];
}

void vfg::VideoFrameWidget::setZoom(ZoomMode mode)
{
    zoomMode = mode;

    updateFrame();
}

QSize vfg::VideoFrameWidget::getFrameSize() const
{
    if(framePixmap.isNull())
        return QSize();

    return framePixmap.size();
}
