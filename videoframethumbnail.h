#ifndef VIDEOFRAMETHUMBNAIL_H
#define VIDEOFRAMETHUMBNAIL_H

#include <QWidget>
#include <QPixmap>

class QVBoxLayout;
class QLabel;

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
        explicit VideoFrameThumbnail(const unsigned frame, QPixmap thumbnail,
                                     QWidget *parent = 0);

        void markSelected();
        void markUnselected();

        unsigned frameNum() const;

    private:
        QVBoxLayout *layout;
        QLabel *pixmapLabel;
        QPixmap thumb;

        unsigned frameNumber;

        void updateFrameSize();

    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
        void resizeEvent(QResizeEvent *event);
        void paintEvent(QPaintEvent *event);

    signals:
        void selected(vfg::VideoFrameThumbnail* thumbnail);
        void doubleClicked(unsigned frameNumber);

    public slots:

    };
}
#endif // VIDEOFRAMETHUMBNAIL_H
