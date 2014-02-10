#ifndef VIDEOFRAMEWIDGET_H
#define VIDEOFRAMEWIDGET_H

#include <QImage>
#include <QPair>
#include <QPixmap>
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
    /**
     * @brief The ZoomMode enum
     *
     * Specifies the zoom mode for the \link VideoFrameWidget preview frame \endlink
     * @sa VideoFrameWidget
     */
    enum class ZoomMode : int {
        Zoom_25, //!< Zoom 25%
        Zoom_50, //!< Zoom 50%
        Zoom_100, //!< Zoom 100%
        Zoom_200, //!< Zoom 200%
        Zoom_Scale //!< Scale to window
    };

    /**
     * @brief The VideoFrameWidget class implements the video preview frame
     */
    class VideoFrameWidget : public QWidget
    {
        Q_OBJECT
    private:
        //! Areas to crop are drawn on the current frame
        QVector<QRect> cropBorders;
        //! Layout to hold the preview frame
        QVBoxLayout* layout;
        //! The label contains the frame
        QLabel* frameLabel;
        //! Modifiable copy of the frame that is displayed in the widget
        QPixmap framePixmap;
        //! The original frame that is never modified
        QPixmap original;
        //! Specifies the current zoom mode
        ZoomMode zoomMode;

        /**
         * @brief updateFrame Applies new changes to the current frame
         */
        void updateFrame();

        /**
         * @brief drawCropArea Paints area to crop on current frame
         */
        void drawCropArea();

        /**
         * @brief getZoomFactor Calculates new frame height based on zoom mode
         * @return Zoom factor
         */
        double getZoomFactor() const;

    public:
        /**
         * @brief VideoFrameWidget Constructor
         * @param parent Owner of the widget
         */
        explicit VideoFrameWidget(QWidget *parent = 0);

        QSize getFrameSize() const;
    protected:
        /**
         * @brief resizeEvent Updates the current frame
         * @param event
         */
        void resizeEvent(QResizeEvent *event);

    public slots:
        /**
         * @brief setZoom Changes the zoom mode
         * @param mode New zoom mode
         */
        void setZoom(ZoomMode mode);

        /**
         * @brief setFrame Sets the current frame
         * @param img Frame to set
         */
        void setFrame(QImage img);

        /**
         * @brief setFrame Sets the current frame
         * @param img Frame to set with a frame number
         */
        void setFrame(QPair<unsigned, QImage> img);

        /**
         * @brief setCrop Sets an area to crop
         * @param area Area to crop
         */
        void setCrop(QRect area);

        /**
         * @brief resetCrop Resets the areas to crop
         */
        void resetCrop();
    };
}

#endif // VIDEOFRAMEWIDGET_H
