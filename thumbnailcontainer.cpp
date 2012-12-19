#include "videoframethumbnail.h"
#include "thumbnailcontainer.h"
#include "flowlayout.h"
#include <QDebug>

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

    if(layout->count() == 100)
    {
        QLayoutItem *item = layout->takeAt(0);
        if(item)
        {
            delete item->widget();
            delete item;
        }
    }

    layout->addWidget(thumbnail);
}

void vfg::ThumbnailContainer::clearThumbnails()
{
    QLayoutItem *item;
    qDebug() << layout->count();
    while((item = layout->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }

    activeWidget = NULL;
}

void vfg::ThumbnailContainer::resizeThumbnails(unsigned width)
{
    for(int i = 0; i < layout->count(); ++i)
    {
        QLayoutItem* item = layout->itemAt(i);
        if(item)
        {
            item->widget()->setFixedWidth(width);
        }
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

    // Remove the layout item the widget belongs to,
    // otherwise ownership stays in the layout
    // and the widget isn't removed properly
    int widgetIndex = layout->indexOf(activeWidget);
    QLayoutItem* item = layout->takeAt(widgetIndex);

    vfg::VideoFrameThumbnail* ret = item->widget();

    // Bugfix: If this item is not deleted, the ownership will remain
    // in the layout, causing weird crashes when it's deleted in clearThumbnails()
    // and when the widget is then used later
    delete item;
    activeWidget = NULL;

    return ret;
}

int vfg::ThumbnailContainer::numThumbnails() const
{
    return layout->count();
}
