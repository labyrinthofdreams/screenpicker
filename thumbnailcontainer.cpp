#include <QSettings>
#include "videoframethumbnail.h"
#include "thumbnailcontainer.h"
#include "flowlayout.h"

// TODO: This implementation is a fucking mess. Fix it.
using vfg::ui::ThumbnailContainer;
using vfg::ui::VideoFrameThumbnail;

ThumbnailContainer::ThumbnailContainer(QWidget *parent) :
    QWidget(parent),
    activeWidget(0),
    maxThumbnails(0)
{
    layout = new FlowLayout;
    setLayout(layout);
}

void ThumbnailContainer::addThumbnail(VideoFrameThumbnail *thumbnail)
{
    connect(thumbnail, SIGNAL(selected(VideoFrameThumbnail*)),
            this, SLOT(handleThumbnailSelection(VideoFrameThumbnail*)));

    connect(thumbnail, SIGNAL(doubleClicked(int)),
            this, SIGNAL(thumbnailDoubleClicked(int)));

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

void ThumbnailContainer::clearThumbnails()
{
    QLayoutItem *item;
    while((item = layout->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }

    activeWidget = NULL;

    // This will force the layout to resize back to normal size
    layout->invalidate();
}

void ThumbnailContainer::resizeThumbnails(int width)
{
    // TODO: Parallelize
    for(int i = 0; i < layout->count(); ++i)
    {
        QLayoutItem* item = layout->itemAt(i);
        if(item)
        {
            item->widget()->setFixedWidth(width);
        }
    }
}

void ThumbnailContainer::handleThumbnailSelection(VideoFrameThumbnail *thumbnail)
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

VideoFrameThumbnail* ThumbnailContainer::takeSelected()
{
    // Bugfix: If the widget is not marked unselected before removing it,
    // it won't remove the stylesheet from the container
    activeWidget->markUnselected();

    // Remove the layout item the widget belongs to,
    // otherwise ownership stays in the layout
    // and the widget isn't removed properly
    const int widgetIndex = layout->indexOf(activeWidget);
    QLayoutItem* item = layout->takeAt(widgetIndex);

    VideoFrameThumbnail* ret = item->widget();

    // Bugfix: If this item is not deleted, the ownership of the widget will remain
    // in the container
    delete item;
    activeWidget = NULL;

    return ret;
}

VideoFrameThumbnail* ThumbnailContainer::selected()
{
    return activeWidget;
}

int ThumbnailContainer::numThumbnails() const
{
    return layout->count();
}

void ThumbnailContainer::setMaxThumbnails(int max)
{
    maxThumbnails = max;

    emit maximumChanged(max);
}

bool ThumbnailContainer::isFull() const
{
    return numThumbnails() == maxThumbnails;
}

void ThumbnailContainer::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);

    if(activeWidget)
    {
        activeWidget->markUnselected();
        activeWidget = NULL;
    }
}
