#include <memory>
#include <QAction>
#include <QCursor>
#include <QLoggingCategory>
#include <QMenu>
#include <QPoint>
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
    if(!thumbnail) {
        return;
    }

    const auto numThumbs = layout->count() + 1;
    if(numThumbs == maxThumbnails) {
        emit full();
    }

    thumbnail->setFixedWidth(thumbnailWidth);

    connect(thumbnail.get(),    &vfg::ui::VideoFrameThumbnail::selected,
            this,               &ThumbnailContainer::handleThumbnailSelection);

    connect(thumbnail.get(),    &vfg::ui::VideoFrameThumbnail::doubleClicked,
            this,               &ThumbnailContainer::thumbnailDoubleClicked);

    connect(thumbnail.get(),    &vfg::ui::VideoFrameThumbnail::customContextMenuRequested,
            this,               &ThumbnailContainer::showContextMenu);

    layout->addWidget(thumbnail.release());

    emit countChanged(numThumbnails());
}

void ThumbnailContainer::clearThumbnails()
{
    qCDebug(CONTAINER) << "Clearing thumbnails";

    while(!layout->isEmpty()) {
        removeFirst();
    }

    // This will force the layout to resize back to normal size
    layout->invalidate();

    emit countChanged(numThumbnails());
}

void ThumbnailContainer::resizeThumbnails(const int width)
{
    for(auto &widget : *this) {
        widget.setFixedWidth(width);
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

void ThumbnailContainer::showContextMenu(const QPoint &pos)
{
    Q_UNUSED(pos);

    QMenu menu;
    auto move = new QAction(tr("Move"), &menu);
    connect(move, &QAction::triggered, this, &ThumbnailContainer::requestMove);
    menu.addAction(move);
    menu.exec(QCursor::pos());
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

    disconnect(widget.get(), 0, 0, 0);

    emit countChanged(numThumbnails());

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

vfg::observer_ptr<vfg::ui::VideoFrameThumbnail>
ThumbnailContainer::at(const int idx) const
{
    if(idx < 0 || idx >= layout->count()) {
        qCCritical(CONTAINER) << "Index out of range while accessing widget by index";

        return {};
    }

    vfg::observer_ptr<QLayoutItem> item = layout->itemAt(idx);

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
