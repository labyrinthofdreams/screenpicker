#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <memory>
#include <QMutex>
#include <QObject>

// Forward declarations
class QImage;
class QSize;
class QString;

namespace vfg {
namespace core {
    class AbstractVideoSource;
}
}

namespace vfg {
namespace core {

/**
 * @brief The VideoFrameGrabber class
 *
 * An interface wrapped around a video source with some
 * common operations for retrieving frames
 */
class VideoFrameGrabber : public QObject
{
    Q_OBJECT

private:
    std::shared_ptr<vfg::core::AbstractVideoSource> avs;

    //! Tracks the last requested frame number to enable
    //! the use of next and prev member functions.
    //! Value is between range [0, numFrames)
    int currentFrame {0};

    mutable QMutex mutex {};

public:
    /**
     * @brief Constructor
     * @pre avs must not be nullptr
     * @param avs Shared pointer to video source
     * @param parent Owner of the object
     * @exception std::runtime_error If avs is nullptr
     */
    explicit VideoFrameGrabber(std::shared_ptr<vfg::core::AbstractVideoSource> avs,
                               QObject *parent = 0);

    /**
     * @brief Get video status
     * @return True if video is available, otherwise false
     */
    bool hasVideo() const;

    /**
     * @brief Set source to grab frames from
     * @pre newAvs must not be nullptr
     * @param newAvs New video source
     * @exception std::runtime_error If avs is nullptr
     */
    void setVideoSource(std::shared_ptr<vfg::core::AbstractVideoSource> newAvs);

    /**
     * @brief Get last requested frame number
     * @return Last requested frame number
     */
    int lastFrame() const;

    /**
     * @brief Get total number of frames in video source
     * @return Total number of frames
     */
    int totalFrames() const;

    /**
     * @brief Get frame from video source
     * @pre frameNum must be between [0, numFrames)
     * @param frameNum Frame to request
     * @return Frame (may be null)
     */
    QImage getFrame(int frameNum);

    /**
     * @brief Check if frame number is in valid range
     *
     * This function is used in place of the video source's equivalent
     * because the frame grabber uses different frame range
     *
     * @param frameNum Frame to check
     * @return True if in valid range, otherwise false
     */
    bool isValidFrame(int frameNum) const;

    /**
     * @brief Get video resolution
     * @return Video resolution
     */
    QSize resolution() const;

public slots:
    /**
     * @brief Request next frame from video source
     */
    void requestNextFrame();

    /**
     * @brief Request previous frame from video source
     */
    void requestPreviousFrame();

    /**
     * @brief Get frame from video source
     *
     * This is a slot equivalent of \link getFrame \endlink
     * It emits \link frameGrabbed \endlink instead of returning QImage
     *
     * @pre Requests are between range [0, numFrames)
     * @param frameNum
     */
    void requestFrame(int frameNum);

signals:
    /**
     * @brief Emit grabbed frame
     * @param frame Frame number and frame image
     */
    void frameGrabbed(int frameNum, const QImage& frame);

    /**
     * @brief Emit errors
     * @param msg Error message
     */
    void errorOccurred(QString msg);
};

} // namespace core
} // namespace vfg

#endif // VIDEOFRAMEGRABBER_H
