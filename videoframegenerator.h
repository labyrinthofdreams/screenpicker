#ifndef VFG_VIDEOFRAMEGENERATOR_H
#define VFG_VIDEOFRAMEGENERATOR_H

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
 * Provides an interface for a \link vfg::VideoFrameGrabber frame grabber \endlink
 * to automate the generation of frames
 */
class VideoFrameGenerator : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     * @param frameGrabber Non-owning pointer to a frame grabber
     * @param parent Owner of the object
     */
    explicit VideoFrameGenerator(vfg::core::VideoFrameGrabber *frameGrabber, QObject *parent = 0);

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

    /**
     * @brief Clear the generator queue
     */
    void clear();
    
signals:
    /**
     * @brief Emits a grabbed frame number and the image
     * @param frame The frame number and the grabbed image
     */
    void frameReady(QPair<int, QImage> frame);
    
public slots:
    void start();
    void pause();
    void resume();
    void stop();
    void fetchNext();
    
private:
    vfg::core::VideoFrameGrabber *frameGrabber;
    QList<int> frames;
    mutable QMutex mutex;
    QWaitCondition waitToContinue;
    bool halt;
    bool active;
    bool paused;
};

} // namespace core
} // namespace vfg

#endif // VFG_VIDEOFRAMEGENERATOR_H
