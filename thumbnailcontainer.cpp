#include <memory>
#include <QLoggingCategory>
#include <QSettings>
#include "flowlayout.h"
#include "ptrutil.hpp"
#include "thumbnailcontainer.h"
#include "videoframethumbnail.h"

Q_LOGGING_CATEGORY(CONTAINER, "thumbnailcontainer")

namespace vfg {
namespace ui {

ThumbnailContainer::ThumbnailContainer(QWidget *parent) :
    QWidget(parent),
    layout(new FlowLayout)
{
    setLayout(layout.get());
}

void ThumbnailContainer::removeFirst()
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

void ThumbnailContainer::addThumbnail(std::unique_ptr<vfg::ui::VideoFrameThumbnail> thumbnail)
{
    qCDebug(CONTAINER) << "Adding thumbnail to container";

    if(!thumbnail) {
        qCCritical(CONTAINER) << "Invalid thumbnail while adding";

        return;
    }

    const auto numThumbnails = layout->count() + 1;
    if(numThumbnails == maxThumbnails) {
        emit full();
    }

    thumbnail->setFixedWidth(thumbnailWidth);

    connect(thumbnail.get(),    &vfg::ui::VideoFrameThumbnail::selected,
            this,               &ThumbnailContainer::handleThumbnailSelection);

    connect(thumbnail.get(),    &vfg::ui::VideoFrameThumbnail::doubleClicked,
            this,               &ThumbnailContainer::thumbnailDoubleClicked);

    layout->addWidget(thumbnail.release());
}

void ThumbnailContainer::clearThumbnails()
{
    qCDebug(CONTAINER) << "Clearing thumbnails";

    while(!layout->isEmpty()) {
        removeFirst();
    }

    // This will force the layout to resize back to normal size
    layout->invalidate();
}

void ThumbnailContainer::resizeThumbnails(const int width)
{
    for(util::observer_ptr<QLayoutItem> item : *layout) {
        item->widget()->setFixedWidth(width);
    }

    thumbnailWidth = width;
}

void ThumbnailContainer::handleThumbnailSelection(vfg::ui::VideoFrameThumbnail *thumbnail)
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

std::unique_ptr<vfg::ui::VideoFrameThumbnail> ThumbnailContainer::takeSelected()
{
    qCDebug(CONTAINER) << "Taking selected thumbnail";

    if(!activeWidget) {
        qCCritical(CONTAINER) << "No thumbnail selected";

        return {};
    }

    const auto widgetIndex = layout->indexOf(activeWidget.get());
    if(widgetIndex == -1) {
        qCCritical(CONTAINER) << "Invalid widget while taking selected";

        return {};
    }

    activeWidget->markUnselected();
    activeWidget.reset();

    std::unique_ptr<QLayoutItem> item(layout->takeAt(widgetIndex));
    std::unique_ptr<vfg::ui::VideoFrameThumbnail> widget(
                static_cast<vfg::ui::VideoFrameThumbnail*>(item->widget()));

    return widget;
}

int ThumbnailContainer::numThumbnails() const
{
    return layout->count();
}

void ThumbnailContainer::setMaxThumbnails(const int max)
{
    maxThumbnails = max;

    emit maximumChanged(max);
}

bool ThumbnailContainer::isFull() const
{
    return numThumbnails() == maxThumbnails;
}

bool ThumbnailContainer::isEmpty() const
{
    return layout->isEmpty();
}

util::observer_ptr<vfg::ui::VideoFrameThumbnail>
ThumbnailContainer::at(const int idx) const
{
    if(idx < 0 || idx >= layout->count()) {
        qCCritical(CONTAINER) << "Index out of range while accessing widget by index";

        return {};
    }

    util::observer_ptr<QLayoutItem> item = layout->itemAt(idx);

    return static_cast<vfg::ui::VideoFrameThumbnail*>(item->widget());
}

void ThumbnailContainer::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);

    if(activeWidget) {
        qCDebug(CONTAINER) << "Unselecting selected thumbnail";

        activeWidget->markUnselected();
        activeWidget.reset();
    }
}

} // namespace ui
} // namespace vfg
