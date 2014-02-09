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

        QMenu contextMenu;
        QAction *fsAction;

        void updateFrame();
        void drawCropArea();
    public:
        explicit VideoFrameWidget(QWidget *parent = 0);

        QSize getFrameSize() const;
    protected:
        void resizeEvent(QResizeEvent *event);
        void mousePressEvent(QMouseEvent *event);

    signals:

    public slots:
        void setFullsize(bool value);
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
