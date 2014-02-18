#ifndef ABSTRACTVIDEOSOURCE_H
#define ABSTRACTVIDEOSOURCE_H

#include <stdexcept>
#include <string>
#include <QImage>
#include <QObject>
#include <QString>

namespace vfg {
namespace exception {

/**
 * @brief The VideoSourceError class
 *
 * General exception relating to video source errors
 */
class VideoSourceError : public std::runtime_error
{
public:
    VideoSourceError(const char* msg) : std::runtime_error(msg) {}
    VideoSourceError(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace exception

namespace core {

/**
 * @brief The AbstractVideoSource class
 *
 * This base class is responsible for loading video sources
 * and returning frames from the source
 */
class AbstractVideoSource : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     */
    AbstractVideoSource() {}

    /**
     * @brief Destructor
     */
    virtual ~AbstractVideoSource() {}

    /**
     * @brief Load given file
     * @param fileName Path to video
     */
    virtual void load(QString fileName) = 0;

    /**
     * @brief Get video status
     * @return True if video is available, otherwise false
     */
    virtual bool hasVideo() const = 0;

    /**
     * @brief Get number of frames in the video
     * @return Number of frames
     */
    virtual int getNumFrames() const = 0;

    /**
     * @brief Get frame from the video source
     *
     * This function does not throw
     *
     * @param frameNumber Frame to request
     * @return The requested frame. Empty QImage on error.
     */
    virtual QImage getFrame(int frameNumber) = 0;

    /**
     * @brief Get supported files by extension
     *
     * Example: "Avisynth files (*.avs,*.avsi)"
     * Example2: "Video files (*.mkv,*.avi)"
     *
     * "*" matches all
     *
     * @return Supported file extensions as a string
     */
    virtual QString getSupportedFormats() = 0;

signals:    
    /**
     * @brief Signals when the video has been loaded
     *
     * This signal is emitted from \link load(QString fileName) load \endlink
     */
    void videoLoaded();
};

} // namespace internal
} // namespace vfg

#endif // ABSTRACTVIDEOSOURCE_H
