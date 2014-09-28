#include <cstddef>
#include <limits>
#include <QLoggingCategory>
#include <QSettings>
#include "flowlayout.h"
#include "ptrutil.hpp"
#include "thumbnailcontainer.h"
#include "videoframethumbnail.h"

Q_LOGGING_CATEGORY(CONTAINER, "thumbnailcontainer")

vfg::ui::ThumbnailContainer::ThumbnailContainer(QWidget *parent) :
    QWidget(parent),
    layout(new FlowLayout),
    activeWidget(nullptr),
    maxThumbnails(std::numeric_limits<int>::max()),
    thumbnailWidth(200)
{
    setLayout(layout.get());
}

void vfg::ui::ThumbnailContainer::removeFirst()
{
    qCDebug(CONTAINER) << "Removing first from container";

    std::unique_ptr<QLayoutItem> item(layout->takeAt(0));
    if(!item) {
        qCCritical(CONTAINER) << "Invalid item while removing first";

        return;
    }

    std::unique_ptr<QWidget> widget(item->widget());
    if(activeWidget && activeWidget == widget.get()) {
        activeWidget.reset();
    }
}

void vfg::ui::ThumbnailContainer::addThumbnail(std::unique_ptr<vfg::ui::VideoFrameThumbnail> thumbnail)
{
    qCDebug(CONTAINER) << "Adding thumbnail to container";

    if(!thumbnail) {
        qCCritical(CONTAINER) << "Invalid thumbnail while adding";

        return;
    }

    const int numThumbnails = layout->count() + 1;
    if(numThumbnails > maxThumbnails) {
        emit full();

        return;
    }
    if(numThumbnails == maxThumbnails) {
        emit full();
    }

    thumbnail->setFixedWidth(thumbnailWidth);

    connect(thumbnail.get(),    SIGNAL(selected(vfg::ui::VideoFrameThumbnail*)),
            this,               SLOT(handleThumbnailSelection(vfg::ui::VideoFrameThumbnail*)));

    connect(thumbnail.get(),    SIGNAL(doubleClicked(int)),
            this,               SIGNAL(thumbnailDoubleClicked(int)));

    // If the container has filled max thumbnails
    // remove oldest widgets until there's space

//    while(numThumbnails-- >= maxThumbnails) {
//        removeFirst();
//    }

    layout->addWidget(thumbnail.release());
}

void vfg::ui::ThumbnailContainer::clearThumbnails()
{
    qCDebug(CONTAINER) << "Clearing thumbnails";

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

    thumbnailWidth = width;
}

void vfg::ui::ThumbnailContainer::handleThumbnailSelection(vfg::ui::VideoFrameThumbnail *thumbnail)
{
    qCDebug(CONTAINER) << "Thumbnail selected";

    if(activeWidget == thumbnail) {
        qCDebug(CONTAINER) << "Same selection";

        return;
    }

    if(activeWidget) {
        qCDebug(CONTAINER) << "Unselecting previously selected widget";

        activeWidget->markUnselected();
    }

    thumbnail->markSelected();
    activeWidget = thumbnail;
}

std::unique_ptr<vfg::ui::VideoFrameThumbnail> vfg::ui::ThumbnailContainer::takeSelected()
{
    qCDebug(CONTAINER) << "Taking selected thumbnail";

    if(!activeWidget) {
        qCCritical(CONTAINER) << "No thumbnail selected";

        return nullptr;
    }

    const int widgetIndex = layout->indexOf(activeWidget.get());
    if(widgetIndex == -1) {
        qCCritical(CONTAINER) << "Invalid widget while taking selected";

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
        qCCritical(CONTAINER) << "Index out of range while accessing widget by index";

        return nullptr;
    }

    util::observer_ptr<QLayoutItem> item = layout->itemAt(idx);

    return static_cast<vfg::ui::VideoFrameThumbnail*>(item->widget());
}

void vfg::ui::ThumbnailContainer::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);

    if(activeWidget) {
        qCDebug(CONTAINER) << "Unselecting selected thumbnail";

        activeWidget->markUnselected();
        activeWidget.reset();
    }
}
