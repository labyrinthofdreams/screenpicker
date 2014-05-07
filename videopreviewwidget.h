#ifndef VIDEOFRAMEWIDGET_H
#define VIDEOFRAMEWIDGET_H

#include <QPixmap>
#include <QtContainerFwd>
#include <QWidget>

// Forward declarations
class QImage;
class QLabel;
class QMouseEvent;
class QRect;
class QResizeEvent;
class QSize;
class QVBoxLayout;
template <class T, class U> class QPair;

namespace vfg {

/**
 * @brief Zoom mode for the \link vfg::ui::VideoPreviewWidget preview frame \endlink
 * @sa VideoPreviewWidget
 */
enum class ZoomMode : int {
    Zoom_25, //!< Zoom 25%
    Zoom_50, //!< Zoom 50%
    Zoom_100, //!< Zoom 100%
    Zoom_200, //!< Zoom 200%
    Zoom_Scale //!< Scale to window
};

namespace ui {

/**
 * @brief The VideoPreviewWidget class
 */
class VideoPreviewWidget : public QWidget
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
     * @brief Applies new changes to the current frame
     */
    void updateFrame();

    /**
     * @brief Paints area to crop on current frame
     */
    void drawCropArea();

    /**
     * @brief Calculates new size based on zoom mode
     * @return New frame size
     */
    QSize calculateSize() const;

public:
    /**
     * @brief Constructor
     * @param parent Owner of the widget
     */
    explicit VideoPreviewWidget(QWidget *parent = 0);

protected:
    /**
     * @brief Updates the current frame
     * @param event
     */
    void resizeEvent(QResizeEvent *event);

public slots:
    /**
     * @brief Changes the zoom mode
     * @param mode New zoom mode
     */
    void setZoom(ZoomMode mode);

    /**
     * @brief Sets the current frame
     * @param img Frame to set
     */
    void setFrame(const QImage& img);

    /**
     * @brief Sets the current frame
     * @param img Frame to set with a frame number
     */
    void setFrame(const QPair<int, QImage>& img);

    /**
     * @brief Sets an area to crop
     * @param area Area to crop
     */
    void setCrop(const QRect& area);

    /**
     * @brief Resets the areas to crop
     */
    void resetCrop();
};

} // namespace ui
} // namespace vfg

#endif // VIDEOFRAMEWIDGET_H
