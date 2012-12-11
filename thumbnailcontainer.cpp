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

    layout->addWidget(thumbnail);
}

void vfg::ThumbnailContainer::handleThumbnailSelection(vfg::VideoFrameThumbnail *thumbnail)
{
    // Do nothing if same selection
    if(activeWidget == thumbnail)
        return;

    // Mark previous as unselected only if it there is previous
    if(activeWidget)
        activeWidget->markUnselected();

    thumbnail->markSelected();
    activeWidget = thumbnail;
}
