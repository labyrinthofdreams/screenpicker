#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <memory>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QPair>
#include <QString>

// Forward declarations
namespace vfg {
namespace core {
    class AbstractVideoSource;
}
}

namespace vfg {

// TODO: This is not clever
static const int FirstFrame = 1;

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

    // Last frame - FirstFrame
    mutable int numFrames;

    // Between range 0 - (last frame - FirstFrame)
    int currentFrame;

    mutable QMutex mutex;

public:
    /**
     * @brief Constructor
     * @param parent Owner of the object
     */
    explicit VideoFrameGrabber(QObject *parent = 0);

    /**
     * @brief Constructor
     * @param avs Video source (does not take ownership)
     * @param parent Owner of the object
     */
    explicit VideoFrameGrabber(std::shared_ptr<vfg::core::AbstractVideoSource> avs,
                               QObject *parent = 0);

    /**
     * @brief Destructor
     */
    ~VideoFrameGrabber();

    /**
     * @brief Get video status
     * @return True if video is available, otherwise false
     */
    bool hasVideo() const;

    /**
     * @brief Set source to grab frames from
     * @param newAvs New video source
     */
    void setVideoSource(std::shared_ptr<vfg::core::AbstractVideoSource> newAvs);

    /**
     * @brief Get last requested frame number
     *
     * @return Last requested frame number + \link vfg::FirstFrame \endlink
     */
    int lastFrame() const;

    /**
     * @brief Get total number of frames in video source
     * @return Total number of frames
     */
    int totalFrames() const;

    /**
     * @brief Get frame from video source
     *
     * Requests are between range \link vfg::FirstFrame \endlink - \link totalFrames() \endlink
     *
     * @param frameNum Frame to request
     * @return Frame
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
     * Requests are between range \link vfg::FirstFrame \endlink - \link totalFrames() \endlink
     *
     * This is a slot equivalent of \link getFrame \endlink
     * It emits \link frameGrabbed \endlink instead of returning QImage
     *
     * @param frameNum
     */
    void requestFrame(int frameNum);

private slots:
    /**
     * @brief Update internal state after video source has updated
     */
    void videoSourceUpdated();

signals:
    /**
     * @brief Emit grabbed frame
     * @param frame Frame number and frame image
     */
    void frameGrabbed(QPair<int, QImage> frame);

    /**
     * @brief Emit errors
     * @param msg Error message
     */
    void errorOccurred(QString msg);
};

} // namespace core
} // namespace vfg

#endif // VIDEOFRAMEGRABBER_H
