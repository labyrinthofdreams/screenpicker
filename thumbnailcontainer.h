#ifndef THUMBNAILCONTAINER_H
#define THUMBNAILCONTAINER_H

#include <cstddef>
#include <memory>
#include <QWidget>
#include "ptrutil.hpp"

// Forward declarations
class FlowLayout;
class QMouseEvent;

namespace vfg {
namespace ui {
    class VideoFrameThumbnail;
}
}

namespace vfg {
namespace ui {

/**
 * @brief A UI widget for displaying VideoFrameThumbnail widgets
 *
 * The ThumbnailContainer class is a UI widget that displays
 * VideoFrameThumbnail widgets in a FlowLayout
 */
class ThumbnailContainer : public QWidget
{
    Q_OBJECT

private:
    //! Widgets are stored in a FlowLayout
    util::observer_ptr<FlowLayout> layout;

    //! Selected widget in the container
    util::observer_ptr<vfg::ui::VideoFrameThumbnail> activeWidget;

    //! Number of thumbnails allowed in the container
    int maxThumbnails;

    //! Thumbnail width in pixels (default: 200)
    int thumbnailWidth;

public:
    /**
     * @brief Constructor
     * @param parent Owner of the widget
     */
    explicit ThumbnailContainer(QWidget *parent = 0);

    /**
     * @brief Add thumbnail to container
     * @param thumbnail Thumbnail to add
     */
    void addThumbnail(std::unique_ptr<vfg::ui::VideoFrameThumbnail> thumbnail);

    /**
     * @brief Clear all thumbnails
     */
    void clearThumbnails();

    /**
     * @brief Resize thumbnails
     * @param width New width width
     */
    void resizeThumbnails(int width);

    /**
     * @brief Take selected widget
     *
     * Removes the selected (active) widget and returns it
     *
     * The returned value may be null
     *
     * @return Selected widget as unique_ptr
     */
    std::unique_ptr<vfg::ui::VideoFrameThumbnail> takeSelected();

    /**
     * @brief Get number of thumbnails
     * @return Number of thumbnails
     */
    int numThumbnails() const;

    /**
     * @brief Set max thumbnails for the container
     *
     * If the number of thumbnails reaches the max thumbnails
     * the oldest thumbnail in the container is removed \sa removeFirst
     *
     * @param max New max thumbnails value
     */
    void setMaxThumbnails(int max);

    /**
     * @brief Check if container is full
     * @return True if container is full, otherwise false
     */
    bool isFull() const;

    /**
     * @brief Check if container is empty
     * @return True if empty, otherwise false
     */
    bool isEmpty() const;

    /**
     * @brief Get item from container at position idx
     * @param idx Position to get item from (last is zero)
     * @return Observer pointer to the VideoFrameThumbnail widget, or nullptr
     */
    util::observer_ptr<vfg::ui::VideoFrameThumbnail> at(std::size_t idx) const;

    /**
     * @brief Removes the oldest thumbnail from the container
     *
     * This member function does nothing if the container is empty
     */
    void removeFirst();

protected:
    /**
     * @brief Handle mouse press event on the container
     * @param ev Mouse event to handle
     */
    void mousePressEvent(QMouseEvent *ev) override;

signals:
    /**
     * @brief Signals a thumbnail double click event
     * @param frameNumber Frame number that was clicked
     */
    void thumbnailDoubleClicked(int frameNumber);

    /**
     * @brief Signals a change in the maximum thumbnails value
     * @param newMaximum Value of the new max
     */
    void maximumChanged(int newMaximum);

    void full();

private slots:
    /**
     * @brief Handle thumbnail selection
     * @param thumbnail Thumbnail that was selected
     */
    void handleThumbnailSelection(vfg::ui::VideoFrameThumbnail* thumbnail);

};

} // namespace ui
} // namespace vfg

#endif // THUMBNAILCONTAINER_H
