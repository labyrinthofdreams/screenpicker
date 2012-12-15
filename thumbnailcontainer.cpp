#include "videoframethumbnail.h"
#include "thumbnailcontainer.h"
#include "flowlayout.h"

vfg::ThumbnailContainer::ThumbnailContainer(QWidget *parent) :
    QWidget(parent),
    activeWidget(0)
{
    layout = new FlowLayout;
    setLayout(layout);
}

void vfg::ThumbnailContainer::addThumbnail(vfg::VideoFrameThumbnail *thumbnail)
{
    connect(thumbnail, SIGNAL(selected(vfg::VideoFrameThumbnail*)),
            this, SLOT(handleThumbnailSelection(vfg::VideoFrameThumbnail*)));

    connect(thumbnail, SIGNAL(doubleClicked(vfg::VideoFrameThumbnail*)),
            this, SIGNAL(thumbnailDoubleClicked(vfg::VideoFrameThumbnail*)));

    layout->addWidget(thumbnail);
}

void vfg::ThumbnailContainer::clearThumbnails()
{
    QLayoutItem *item;
    while((item = layout->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }
}

void vfg::ThumbnailContainer::handleThumbnailSelection(vfg::VideoFrameThumbnail *thumbnail)
{
    // Do nothing if same selection
    if(activeWidget == thumbnail)
    {
        return;
    }

    // Mark previous as unselected only if it there is previous
    if(activeWidget)
    {
        activeWidget->markUnselected();
    }

    thumbnail->markSelected();
    activeWidget = thumbnail;
}

vfg::VideoFrameThumbnail* vfg::ThumbnailContainer::takeSelected()
{
    // Bugfix: If the widget is not marked unselected before removing it,
    // it won't remove the stylesheet from the container
    activeWidget->markUnselected();
    layout->removeWidget(activeWidget);
    vfg::VideoFrameThumbnail* ret = activeWidget;
    activeWidget = NULL;
    return ret;
}
