#ifndef VFG_VIDEOFRAMEGENERATOR_H
#define VFG_VIDEOFRAMEGENERATOR_H

#include <memory>
#include <QImage>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QPair>
#include <QWaitCondition>

// Forward declarations
namespace vfg {
namespace core {
    class VideoFrameGrabber;
}
}

namespace vfg {
namespace core {

/**
 * @brief The VideoFrameGenerator class
 *
 * Provides an interface for a \link vfg::core::VideoFrameGrabber frame grabber \endlink
 * to automate the generation of frames
 */
class VideoFrameGenerator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param frameGrabber Shared pointer to frame grabber
     * @param parent Owner of the object
     * @exception std::runtime_error If frameGrabber is nullptr
     */
    explicit VideoFrameGenerator(std::shared_ptr<vfg::core::VideoFrameGrabber> frameGrabber, QObject *parent = 0);

    /**
     * @brief Get generator running status
     *
     * The generator is considered running when it's fetching
     * a frame from the grabber and is not paused or stopped
     *
     * @return True if running, otherwise false
     */
    bool isRunning() const;

    /**
     * @brief Get generator paused status
     *
     * The generator is considered paused when it's not fetching
     * a frame from the grabber and is not stopped.
     *
     * Call \link resume \endlink to start the generator.
     *
     * @return True if paused, otherwise false
     */
    bool isPaused() const;

    /**
     * @brief Add a frame number in the queue
     * @param frame Frame to fetch from the grabber
     */
    void enqueue(int frame);

    /**
     * @brief Get number of frames in the generator queue
     * @return Number of frames
     */
    int remaining() const;
    
signals:
    /**
     * @brief Emits a grabbed frame number and the image
     * @param frame The frame number and the grabbed image
     */
    void frameReady(QPair<int, QImage> frame);
    
public slots:
    /**
     * @brief Starts the generator and begins to emit grabbed frames
     *
     * Note that if start is called on a paused generator and a
     * stored grabbed frame is waiting to be emitted, it is discarded
     */
    void start();

    /**
     * @brief Pauses the generator and stops emitting grabbed frames
     *
     * Note that the last frame that is grabbed after pause() is called
     * is stored temporarily and emitted after resume() is called
     */
    void pause();

    /**
     * @brief Resumes the generator and begins to emit grabbed frames
     *
     * Note that the last frame grabbed after pausing is emitted
     *
     * No action is taken if the generator state is not paused
     */
    void resume();

    /**
     * @brief Stops the generator and stops emitting grabbed frames
     *
     * Remaining frames in the queue are removed and the last frame
     * grabbed is discarded
     */
    void stop();

private:
    /**
     * @brief State of the generator
     */
    enum class State {
        Running,
        Paused,
        Stopped
    };
    
private:
    std::shared_ptr<vfg::core::VideoFrameGrabber> frameGrabber;
    QList<int> frames;
    mutable QMutex mutex;
    State state {State::Stopped};

    //!< The last frame grabbed after calling pause()
    QPair<int, QImage> last;
};

} // namespace core
} // namespace vfg

#endif // VFG_VIDEOFRAMEGENERATOR_H
