#ifndef VIDEOFRAMETHUMBNAIL_H
#define VIDEOFRAMETHUMBNAIL_H

#include <QPixmap>
#include <QWidget>

class QLabel;
class QMouseEvent;
class QResizeEvent;
class QVBoxLayout;

namespace vfg
{
    /**
     * @brief The VideoFrameThumbnail class
     */
    class VideoFrameThumbnail : public QWidget
    {
        Q_OBJECT
    public:
        /**
         * @brief VideoFrameThumbnail Constructor
         * @param frame Frame number
         * @param thumbnail Thumbnail image
         * @param parent Owner of the widget
         */
        explicit VideoFrameThumbnail(int frame, QPixmap thumbnail,
                                     QWidget *parent = 0);

        /**
         * @brief markSelected Highlights the thumbnail
         */
        void markSelected();

        /**
         * @brief markUnselected Removes highlight from thumbnail
         */
        void markUnselected();

        /**
         * @brief frameNum
         * @return Frame number
         */
        unsigned frameNum() const;

    private:
        QVBoxLayout *layout;
        QLabel *pixmapLabel;
        QPixmap thumb;

        unsigned frameNumber;

        /**
         * @brief updateFrameSize Scales the thumbnail to the widget size
         */
        void updateFrameSize();

    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
        void resizeEvent(QResizeEvent *event);
        void paintEvent(QPaintEvent *event);

    signals:
        /**
         * @brief selected Emits the selected widget
         * @param thumbnail
         */
        void selected(vfg::VideoFrameThumbnail* thumbnail);
        void doubleClicked(unsigned frameNumber);

    public slots:

        /**
         * @brief doubleClicked Emits the frame number of the widget
         * @param frameNumber
         */
    };
}
#endif // VIDEOFRAMETHUMBNAIL_H
