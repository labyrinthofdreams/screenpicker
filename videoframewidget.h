#ifndef VIDEOFRAMEWIDGET_H
#define VIDEOFRAMEWIDGET_H

#include <QImage>
#include <QPair>
#include <QPixmap>
#include <QScrollArea>
#include <QVector>

// Forward declarations
class QLabel;
class QRect;
class QResizeEvent;
class QSize;
class QVBoxLayout;

namespace vfg
{
    /*
     * Implements a simple video frame widget for displaying images (frames)
     */
    class VideoFrameWidget : public QWidget
    {
        Q_OBJECT
    private:
        QVector<QRect> cropBorders;
        QVBoxLayout* layout;
        QLabel* frameLabel;
        QPixmap framePixmap;
        QPixmap original;
        bool fullsize;

        void updateFrameSize();
    public:
        explicit VideoFrameWidget(QWidget *parent = 0);

        void setFullsize(bool value);
        QSize getFrameSize() const;
    protected:
        void resizeEvent(QResizeEvent *event);

    signals:

    public slots:
        void setFrame(QImage img);
        void setFrame(QPair<unsigned, QImage> img);
        /**
         * @brief setCrop draws the croppable area on the frame
         * @param area Area to crop
         */
        void setCrop(QRect area);
        /**
         * @brief resetCrop resets visualized croppable area
         */
        void resetCrop();
    };
}

#endif // VIDEOFRAMEWIDGET_H
