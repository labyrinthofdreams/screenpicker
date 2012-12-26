#include <QSettings>
#include "videoframethumbnail.h"
#include "thumbnailcontainer.h"
#include "flowlayout.h"

vfg::ThumbnailContainer::ThumbnailContainer(QWidget *parent) :
    QWidget(parent),
    activeWidget(0),
    maxThumbnails(0)
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

//    QSettings cfg("config.ini", QSettings::IniFormat);
//    const int maxThumbnails = cfg.value("maxthumbnails").toInt();


    int numThumbnails = layout->count();
    if((maxThumbnails > 0) && (numThumbnails >= maxThumbnails))
    {        
        do
        {
            QLayoutItem *item = layout->takeAt(0);
            if(item)
            {
                // bugfix: Deleting a selected widget and then accessing it
                // causes a nasty crash
                // Only mark unselected if it's being removed
                if(activeWidget && (activeWidget == item->widget()))
                {
                    activeWidget->markUnselected();
                    activeWidget = NULL;
                }

                delete item->widget();
                delete item;
                numThumbnails--;
            }
        }
        while(numThumbnails >= maxThumbnails);
    }

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
    const int widgetIndex = layout->indexOf(activeWidget);
    QLayoutItem* item = layout->takeAt(widgetIndex);

    vfg::VideoFrameThumbnail* ret = item->widget();

    // Bugfix: If this item is not deleted, the ownership of the widget will remain
    // in the container
    delete item;
    activeWidget = NULL;

    return ret;
}

int vfg::ThumbnailContainer::numThumbnails() const
{
    return layout->count();
}

void vfg::ThumbnailContainer::setMaxThumbnails(unsigned max)
{
    maxThumbnails = max;
}

void vfg::ThumbnailContainer::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);

    if(activeWidget)
    {
        activeWidget->markUnselected();
        activeWidget = NULL;
    }
}
