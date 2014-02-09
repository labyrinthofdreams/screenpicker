#include <QtWidgets>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QPair>
#include <QVector>
#include "videoframewidget.h"

vfg::VideoFrameWidget::VideoFrameWidget(QWidget *parent) :
    QWidget(parent),
    cropBorders(),
    framePixmap(),
    original(),
    fullsize(false),
    contextMenu()
{
    frameLabel = new QLabel;
    frameLabel->setAlignment(Qt::AlignCenter);
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

    fsAction = new QAction {tr("Full resolution"), this};
    fsAction->setCheckable(true);
    fsAction->setChecked(fullsize);
    connect(fsAction, SIGNAL(triggered(bool)),
            this, SLOT(setFullsize(bool)));
    contextMenu.addAction(fsAction);
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
    contextMenu.exec(event->globalPos());
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

    if(!fullsize)
    {
        setMinimumSize(1, 1);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

        frameLabel->setPixmap(framePixmap.scaled(frameLabel->size(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation));
    }
    else
    {
        frameLabel->setPixmap(framePixmap);
        setFixedHeight(framePixmap.height());
    }
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

void vfg::VideoFrameWidget::setFullsize(bool value)
{
    fullsize = value;
    fsAction->setChecked(value);

    updateFrame();
}

QSize vfg::VideoFrameWidget::getFrameSize() const
{
    if(framePixmap.isNull())
        return QSize();

    return framePixmap.size();
}
