#ifndef VIDEOFRAMETHUMBNAIL_H
#define VIDEOFRAMETHUMBNAIL_H

#include <QWidget>
#include <QPixmap>

class QVBoxLayout;
class QLabel;

class QKeyEvent;
class QResizeEvent;
class QMouseEvent;

namespace vfg
{
    /**
     * @brief The VideoFrameThumbnail class
     * Displays video thumbnail, emits a frame number when selected
     */
    class VideoFrameThumbnail : public QWidget
    {
        Q_OBJECT
    public:
        explicit VideoFrameThumbnail(QWidget *parent = 0);
        void setThumbnail(QPixmap thumbnail);
        void setFrameNumber(unsigned frame);

    private:
        QVBoxLayout *layout;
        QLabel *pixmapLabel;
        QPixmap thumb;

        unsigned frameNumber;

        void updateFrameSize();

    protected:
        void keyPressEvent(QKeyEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
        void resizeEvent(QResizeEvent *event);

    signals:
        void selected(unsigned frameId);

    public slots:

    };
}
#endif // VIDEOFRAMETHUMBNAIL_H
