#ifndef VIDEOFRAMETHUMBNAIL_H
#define VIDEOFRAMETHUMBNAIL_H

#include <QWidget>
#include <QPixmap>

class QVBoxLayout;
class QLabel;

class QKeyEvent;
class QResizeEvent;

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

    private:
        QVBoxLayout *layout;
        QLabel *pixmapLabel;
        QPixmap thumb;

        void updateFrameSize();

    protected:
        void keyPressEvent(QKeyEvent *event);
        void resizeEvent(QResizeEvent *event);

    signals:
        void selected(int frameId);

    public slots:

    };
}
#endif // VIDEOFRAMETHUMBNAIL_H
