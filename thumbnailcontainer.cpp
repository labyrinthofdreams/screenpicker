#include <cstddef>
#include <limits>
#include <QSettings>
#include "flowlayout.h"
#include "ptrutil.hpp"
#include "thumbnailcontainer.h"
#include "videoframethumbnail.h"

vfg::ui::ThumbnailContainer::ThumbnailContainer(QWidget *parent) :
    QWidget(parent),
    layout(new FlowLayout),
    activeWidget(nullptr),
    maxThumbnails(std::numeric_limits<int>::max())
{
    setLayout(layout.get());
}

void vfg::ui::ThumbnailContainer::removeFirst()
{
    std::unique_ptr<QLayoutItem> item(layout->takeAt(0));
    if(!item) {
        return;
    }

    std::unique_ptr<QWidget> widget(item->widget());

    if(activeWidget && activeWidget == widget.get()) {
        activeWidget.reset();
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
    for(util::observer_ptr<QLayoutItem> item : *layout) {
        item->widget()->setFixedWidth(width);
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
    if(!activeWidget) {
        return nullptr;
    }

    const int widgetIndex = layout->indexOf(activeWidget.get());
    if(widgetIndex == -1) {
        return nullptr;
    }

    activeWidget->markUnselected();
    activeWidget.reset();

    std::unique_ptr<QLayoutItem> item(layout->takeAt(widgetIndex));
    std::unique_ptr<vfg::ui::VideoFrameThumbnail> widget(
                static_cast<vfg::ui::VideoFrameThumbnail*>(item->widget()));

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

bool vfg::ui::ThumbnailContainer::isEmpty() const
{
    return layout->isEmpty();
}

util::observer_ptr<vfg::ui::VideoFrameThumbnail>
vfg::ui::ThumbnailContainer::at(const std::size_t idx) const
{
    const std::size_t count = layout->count();
    if(idx >= count) {
        return nullptr;
    }

    util::observer_ptr<QLayoutItem> item = layout->itemAt(idx);

    return static_cast<vfg::ui::VideoFrameThumbnail*>(item->widget());
}

void vfg::ui::ThumbnailContainer::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);

    if(activeWidget)
    {
        activeWidget->markUnselected();
        activeWidget.reset();
    }
}
