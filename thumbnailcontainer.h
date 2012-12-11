#ifndef THUMBNAILCONTAINER_H
#define THUMBNAILCONTAINER_H

#include <QWidget>

class FlowLayout;

namespace vfg
{
    class VideoFrameThumbnail;

    class ThumbnailContainer : public QWidget
    {
        Q_OBJECT
    private:
        FlowLayout* layout;

        vfg::VideoFrameThumbnail* activeWidget;
    public:
        explicit ThumbnailContainer(QWidget *parent = 0);

        void addThumbnail(vfg::VideoFrameThumbnail* thumbnail);

    signals:

    public slots:

    private slots:
        void handleThumbnailSelection(vfg::VideoFrameThumbnail* thumbnail);

    };
}

#endif // THUMBNAILCONTAINER_H
