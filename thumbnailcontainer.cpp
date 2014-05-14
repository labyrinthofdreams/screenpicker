#include <limits>
#include <QSettings>
#include "videoframethumbnail.h"
#include "thumbnailcontainer.h"
#include "flowlayout.h"

vfg::ui::ThumbnailContainer::ThumbnailContainer(QWidget *parent) :
    QWidget(parent),
    activeWidget(nullptr),
    maxThumbnails(std::numeric_limits<int>::max())
{
    layout = new FlowLayout;
    setLayout(layout);
}

void vfg::ui::ThumbnailContainer::removeFirst()
{
    QLayoutItem *item = layout->takeAt(0);
    if(item)
    {
        // Only mark active widget as unselected if it's being removed
        if(activeWidget && activeWidget == item->widget())
        {
            activeWidget->markUnselected();
            activeWidget = nullptr;
        }

        delete item->widget();
        delete item;
    }
}

void vfg::ui::ThumbnailContainer::addThumbnail(std::unique_ptr<vfg::ui::VideoFrameThumbnail> thumbnail)
{
    if(!thumbnail) {
        return;
    }

    connect(thumbnail.get(),    SIGNAL(selected(vfg::ui::VideoFrameThumbnail*)),
            this,               SLOT(handleThumbnailSelection(vfg::ui::VideoFrameThumbnail*)));

    connect(thumbnail.get(),    SIGNAL(doubleClicked(int)),
            this,               SIGNAL(thumbnailDoubleClicked(int)));

    // If the container has filled max thumbnails
    // remove oldest widgets until there's space
    int numThumbnails = layout->count();
    while(numThumbnails-- >= maxThumbnails) {
        removeFirst();
    }

    layout->addWidget(thumbnail.release());
}

void vfg::ui::ThumbnailContainer::clearThumbnails()
{
    while(!layout->isEmpty()) {
        removeFirst();
    }

    // This will force the layout to resize back to normal size
    layout->invalidate();
}

void vfg::ui::ThumbnailContainer::resizeThumbnails(const int width)
{
    for(QLayoutItem* item : *layout) {
        if(item) {
            item->widget()->setFixedWidth(width);
        }
    }
}

void vfg::ui::ThumbnailContainer::handleThumbnailSelection(vfg::ui::VideoFrameThumbnail *thumbnail)
{
    if(activeWidget == thumbnail) {
        return;
    }

    if(activeWidget) {
        activeWidget->markUnselected();
    }

    thumbnail->markSelected();
    activeWidget = thumbnail;
}

std::unique_ptr<vfg::ui::VideoFrameThumbnail> vfg::ui::ThumbnailContainer::takeSelected()
{
    std::unique_ptr<vfg::ui::VideoFrameThumbnail> widget;
    if(!activeWidget) {
        return widget;
    }

    const int widgetIndex = layout->indexOf(activeWidget);
    std::unique_ptr<QLayoutItem> item(layout->takeAt(widgetIndex));
    if(!item) {
        return widget;
    }

    widget.reset(static_cast<vfg::ui::VideoFrameThumbnail*>(item->widget()));

    activeWidget->markUnselected();
    activeWidget = nullptr;

    return widget;
}

int vfg::ui::ThumbnailContainer::numThumbnails() const
{
    return layout->count();
}

void vfg::ui::ThumbnailContainer::setMaxThumbnails(const int max)
{
    maxThumbnails = max;

    emit maximumChanged(max);
}

bool vfg::ui::ThumbnailContainer::isFull() const
{
    return numThumbnails() == maxThumbnails;
}

void vfg::ui::ThumbnailContainer::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);

    if(activeWidget)
    {
        activeWidget->markUnselected();
        activeWidget = nullptr;
    }
}
