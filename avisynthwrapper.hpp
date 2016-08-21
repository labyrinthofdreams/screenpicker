#ifndef VFG_CORE_AVISYNTHWRAPPER_HPP
#define VFG_CORE_AVISYNTHWRAPPER_HPP

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include "avs_internal.c"
#include "ptrutil.hpp"

namespace vfg {
namespace avisynth {

/**
 * @brief Avisynth exception error class
 */
struct AvisynthError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/**
 * @brief Pixel formats as defined by Avisynth
 */
enum class PixelFormat : int {
    Unknown = AVS_CS_UNKNOWN,
    BGR24 = AVS_CS_BGR24,
    BGR32 = AVS_CS_BGR32,
    YUY2 = AVS_CS_YUY2,
    RAW32 = AVS_CS_RAW32,
    YV24 = AVS_CS_YV24,
    YV16 = AVS_CS_YV16,
    YV12 = AVS_CS_YV12,
    I420 = AVS_CS_I420,
    IYUV = AVS_CS_IYUV,
    YV411 = AVS_CS_YV411,
    YUV9 = AVS_CS_YUV9,
    Y8 = AVS_CS_Y8
};

/**
 * @brief VideoFrame represents a single frame in a video
 *
 * The class wraps AVS_VideoFrame and provides a few
 * required member functions to copy it to another container
 */
class VideoFrame
{
public:
    using DataReadPtr = const BYTE*;
    using Deleter = decltype(avs_hnd_t::func.avs_release_video_frame);

private:
    std::unique_ptr<AVS_VideoFrame, Deleter> videoFrame;

public:
    VideoFrame(std::unique_ptr<AVS_VideoFrame, Deleter> frame);

    /**
     * @brief Check that the object contains valid data
     * @return True if valid, otherwise false
     */
    bool isValid() const;

    /**
     * @brief Get frame pitch (stride)
     * @return Frame pitch
     */
    int pitch() const;

    /**
     * @brief Get read-only data pointer to the frame data
     * @return Read-only data pointer to the frame data
     */
    DataReadPtr data() const;
};

/**
 * @brief A wrapper for the Avisynth C-library
 */
class AvisynthWrapper
{
private:
    avs_hnd_t avsHandle {};
    vfg::observer_ptr<const AVS_VideoInfo> info {};
    std::string openFilePath {};

public:
    /**
     * @brief Constructor
     * @throws AvisynthError If fails to load avisynth dll
     * @throws AvisynthError If fails to create script environment
     */
    AvisynthWrapper();
    ~AvisynthWrapper();

    /**
     * @brief Load file from path
     * @param path Path to file to load
     * @throws AvisynthError If error occurs during script import
     * @throws AvisynthError If imported script doesn't return clip
     * @throws AvisynthError If import script doesn't return video data
     */
    void load(const std::string& path);

    /**
     * @brief Get frame from video
     * @pre 0 <= frameNum < numFrames()
     * @param frameNum Frame number
     * @throws AvisynthError If there is no video
     * @throws AvisynthError If frameNum is out of range
     * @throws AvisynthError If the C library returns an error
     * when getting frame
     * @return Captured frame
     */
    VideoFrame getFrame(int frameNum) const;

    /**
     * @brief Check that video has been loaded successfully
     * @return True if video is available, otherwise false
     */
    bool hasVideo() const;

    /**
     * @brief Get number of frames in the loaded video
     * @return Number of frames or 0 if no video
     */
    int numFrames() const;

    /**
     * @brief Get pixel format for the loaded video
     * @return Pixel format, Unknown if no video
     */
    PixelFormat pixelFormat() const;

    /**
     * @brief Check if video is in RGB color format
     * @return True if RGB, otherwise false
     */
    bool isRGB() const;

    /**
     * @brief Check if video is in YUV color format
     * @return True if YUV, otherwise false
     */
    bool isYUV() const;

    /**
     * @brief Get video width
     * @return Video width or 0
     */
    int width() const;

    /**
     * @brief Get video height
     * @return Video height or 0
     */
    int height() const;

    /**
     * @brief Get opened filename
     * @return Opened filename
     */
    std::string fileName() const;
};

} // namespace avisynth
} // namespace vfg

#endif // VFG_CORE_AVISYNTHWRAPPER_HPP
