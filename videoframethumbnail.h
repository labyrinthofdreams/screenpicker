#ifndef VIDEOFRAMETHUMBNAIL_H
#define VIDEOFRAMETHUMBNAIL_H

#include <QPixmap>
#include <QWidget>
#include "ptrutil.hpp"

class QImage;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

namespace vfg {
namespace ui {

/**
 * @brief The VideoFrameThumbnail class
 */
class VideoFrameThumbnail : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     * @param frame Frame number
     * @param thumbnail Thumbnail image
     * @param parent Owner of the widget
     */
    explicit VideoFrameThumbnail(int frame, const QImage& thumbnail,
                                 QWidget *parent = 0);

    /**
     * @brief Highlights the thumbnail
     */
    void markSelected();

    /**
     * @brief Removes highlight from thumbnail
     */
    void markUnselected();

    /**
     * @brief Retrieves frame number
     * @return Frame number
     */
    int frameNum() const;

private:
    util::observer_ptr<QLabel> pixmapLabel;
    QPixmap thumb;
    const int frameNumber;

    /**
     * @brief Scales the thumbnail to the widget size
     */
    void updateFrameSize();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);

signals:
    /**
     * @brief Emits the selected widget
     * @param thumbnail
     */
    void selected(vfg::ui::VideoFrameThumbnail* thumbnail);

    /**
     * @brief Emits the frame number of the widget
     * @param frameNumber
     */
    void doubleClicked(int frameNumber);
};

} // namespace ui
} // namespace vfg

#endif // VIDEOFRAMETHUMBNAIL_H
