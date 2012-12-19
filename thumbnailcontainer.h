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
    public:
        explicit ThumbnailContainer(QWidget *parent = 0);

        void addThumbnail(vfg::VideoFrameThumbnail* thumbnail);
        void clearThumbnails();
        void resizeThumbnails(unsigned width);
        vfg::VideoFrameThumbnail* takeSelected();
        int numThumbnails() const;

    protected:
        void mousePressEvent(QMouseEvent *ev);

    signals:
        void thumbnailDoubleClicked(vfg::VideoFrameThumbnail* thumbnail);

    public slots:

    private slots:
        void handleThumbnailSelection(vfg::VideoFrameThumbnail* thumbnail);

    };
}

#endif // THUMBNAILCONTAINER_H
