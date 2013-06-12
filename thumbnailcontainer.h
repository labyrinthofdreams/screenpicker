#ifndef THUMBNAILCONTAINER_H
#define THUMBNAILCONTAINER_H

#include <QWidget>

class FlowLayout;
class QMouseEvent;

namespace vfg
{
    class VideoFrameThumbnail;

    class ThumbnailContainer : public QWidget
    {
        Q_OBJECT
    private:
        FlowLayout* layout;

        vfg::VideoFrameThumbnail* activeWidget;

        unsigned maxThumbnails;
    public:
        explicit ThumbnailContainer(QWidget *parent = 0);

        void addThumbnail(vfg::VideoFrameThumbnail* thumbnail);
        void clearThumbnails();
        void resizeThumbnails(const unsigned width);
        vfg::VideoFrameThumbnail* takeSelected();
        vfg::VideoFrameThumbnail* selected();
        int numThumbnails() const;
        void setMaxThumbnails(const unsigned max);
        bool isFull() const;

    protected:
        void mousePressEvent(QMouseEvent *ev);

    signals:
        void thumbnailDoubleClicked(unsigned frameNumber);
        void maximumChanged(int newMaximum);

    public slots:

    private slots:
        void handleThumbnailSelection(vfg::VideoFrameThumbnail* thumbnail);

    };
}

#endif // THUMBNAILCONTAINER_H
