#include <QtGui>
#include "videoframewidget.h"

vfg::VideoFrameWidget::VideoFrameWidget(QWidget *parent) :
    QWidget(parent)
{
    frameLabel = new QLabel;
    frameLabel->setSizePolicy(QSizePolicy::Expanding,
                              QSizePolicy::Expanding);
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
    plt.setColor(QPalette::Window, Qt::red);
    setPalette(plt);

    setContentsMargins(0, 0, 0, 0);
}

void vfg::VideoFrameWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(framePixmap.isNull())
        return;

    updateFrameSize();
}

void vfg::VideoFrameWidget::setFrame(QImage img)
{
    framePixmap = QPixmap::fromImage(img);
    updateFrameSize();
}

void vfg::VideoFrameWidget::updateFrameSize()
{
    frameLabel->setPixmap(framePixmap.scaled(frameLabel->size(),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
}

void vfg::VideoFrameWidget::setFullsize(bool value)
{
    if(value)
    {
        setFixedSize(framePixmap.size());
    }
    else
    {
        setMinimumSize(1, 1);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    }
}
