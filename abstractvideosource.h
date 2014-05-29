#ifndef ABSTRACTVIDEOSOURCE_H
#define ABSTRACTVIDEOSOURCE_H

#include <stdexcept>
#include <string>
#include <QObject>

// Forward declarations
namespace vfg {
    class ScriptParser;
} // namespace vfg

class QFileInfo;
class QImage;
class QString;

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
    AbstractVideoSource() = default;

    /**
     * @brief Destructor
     */
    virtual ~AbstractVideoSource() = default;

    /**
     * @brief Load given file
     * @param fileName Path to video
     */
    virtual void load(const QString& fileName) = 0;

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
     * @param frameNumber Frame to request
     * @exception vfg::exception::VideoSourceError
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

    /**
     * @brief Check if frame is in valid range
     * @param frameNum Frame to check
     * @return True if in range, otherwise False
     */
    virtual bool isValidFrame(int frameNum) const = 0;

    /**
     * @brief Get a script parser for the derived video source and filename
     * @param info File to return the parser for
     * @return Script parser for a file
     */
    virtual vfg::ScriptParser getParser(const QFileInfo& info) const = 0;

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
