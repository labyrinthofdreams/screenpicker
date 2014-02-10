#ifndef VIDEOFRAMEWIDGET_H
#define VIDEOFRAMEWIDGET_H

#include <QImage>
#include <QMenu>
#include <QPair>
#include <QPixmap>
#include <QScrollArea>
#include <QVector>

// Forward declarations
class QLabel;
class QMouseEvent;
class QRect;
class QResizeEvent;
class QSize;
class QVBoxLayout;

namespace vfg
{
    enum class ZoomMode : int {
        Zoom_25,
        Zoom_50,
        Zoom_100,
        Zoom_200,
        Zoom_Scale
    };

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
        ZoomMode zoomMode;

        void updateFrame();
        void drawCropArea();

        double getZoomFactor() const;
    public:
        explicit VideoFrameWidget(QWidget *parent = 0);

        QSize getFrameSize() const;
    protected:
        void resizeEvent(QResizeEvent *event);

    signals:
        void fullsizeChanged(bool);

    public slots:
        void setZoom(ZoomMode mode);
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
