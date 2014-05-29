#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <QFileInfo>
#include <QImage>
#include <QString>
#include "avisynthvideosource.h"
#include "ptrutil.hpp"
#include "scriptparser.h"

/**
 * @brief Convert AVS_VideoFrame to QImage (32-bit ARGB)
 * @param frame Unique pointer to AVS_VideoFrame
 * @param width Pixel width of the frame
 * @param height Pixel height of the frame
 * @exception std::runtime_error If frame is nullptr
 * @return Frame as QImage
 */
template <class Deleter>
static
QImage videoFrameToQImage(const std::unique_ptr<AVS_VideoFrame, Deleter>& frame,
                          const std::size_t width,
                          const std::size_t height) {
    if(!frame) {
        throw std::runtime_error("Frame must be a valid object");
    }

    QImage image(width, height, QImage::Format_ARGB32);
    const std::size_t scanlineLength = width * 4;
    const std::size_t pitch = avs_get_pitch(frame.get());
    const util::observer_ptr<const BYTE> data = avs_get_read_ptr(frame.get());

    for(int y = 0, sy = height - 1; y < height; ++y, --sy) {
        // If we used y instead of sy in the scanLine function
        // the image would be created upside-down
        std::memcpy(image.scanLine(sy), data.get() + y * pitch, scanlineLength);
    }

    return image;
}

/**
 * @brief Call Deleter D on object T on scope exit
 *
 * RaiiDeleter will hold a reference to the object being deleted
 */
template <class T, class D>
class RaiiDeleter {
    T& _object;
    D _deleter;

public:
    RaiiDeleter(T& object, D deleter) :
        _object(object),
        _deleter(deleter) {
    }

    ~RaiiDeleter() {
        _deleter(_object);
    }
};

vfg::core::AvisynthVideoSource::AvisynthVideoSource() :
    AbstractVideoSource(),
    avsHandle(),
    info()
{
    if(internal_avs_load_library(&avsHandle) < 0) {
        throw vfg::exception::VideoSourceError("Failed to load AviSynth library");
    }

    avsHandle.env = avsHandle.func.avs_create_script_environment(AVS_INTERFACE_25);
    if(avsHandle.func.avs_get_error) {
        const util::observer_ptr<const char> error(
                    avsHandle.func.avs_get_error(avsHandle.env));
        if(error) {
            throw vfg::exception::VideoSourceError(
                        std::string("Failed to create script environment: ") + error.get());
        }
    }
}

vfg::core::AvisynthVideoSource::~AvisynthVideoSource()
{
    if(hasVideo()) {
        avsHandle.func.avs_release_clip(avsHandle.clip);
    }
    if(avsHandle.func.avs_delete_script_environment) {
        avsHandle.func.avs_delete_script_environment(avsHandle.env);
    }
    FreeLibrary(avsHandle.library);
}

void vfg::core::AvisynthVideoSource::load(const QString& fileName)
{
    auto&& hf = avsHandle.func;
    AVS_Value arg = avs_new_value_string(fileName.toLocal8Bit().constData());
    AVS_Value res = hf.avs_invoke(avsHandle.env, "Import", arg, NULL);
    RaiiDeleter<AVS_Value, decltype(hf.avs_release_value)> cleanup(res, hf.avs_release_value);

    if(avs_is_error(res)) {
        throw vfg::exception::VideoSourceError(avs_as_string(res));
    }
    if(!avs_is_clip(res)) {
        throw vfg::exception::VideoSourceError("Imported script did not return a video clip");
    }

    avsHandle.clip = hf.avs_take_clip(res, avsHandle.env);
    info = hf.avs_get_video_info(avsHandle.clip);
    if(!avs_has_video(info.get())) {
        throw vfg::exception::VideoSourceError("Imported script does not have video data");
    }

    // Video must be RGB32
    if(!avs_is_rgb32(info.get())) {
        throw vfg::exception::VideoSourceError("Video is not RGB32. Add ConvertToRGB32() to your script.");
    }

    emit videoLoaded();
}

bool vfg::core::AvisynthVideoSource::hasVideo() const
{
    return info && avs_has_video(info.get());
}

int vfg::core::AvisynthVideoSource::getNumFrames() const
{
    if(!info) {
        return 0;
    }

    return info->num_frames;
}

QImage vfg::core::AvisynthVideoSource::getFrame(const int frameNumber)
{
    if(!info) {
        throw vfg::exception::VideoSourceError("Invalid video source handle");
    }

    if(frameNumber < 0 || frameNumber >= info->num_frames) {
        throw vfg::exception::VideoSourceError("Frame out of range");
    }

    auto&& hf = avsHandle.func;
    const std::unique_ptr<AVS_VideoFrame, decltype(hf.avs_release_video_frame)> frame(
                hf.avs_get_frame(avsHandle.clip, frameNumber),
                hf.avs_release_video_frame);
    const util::observer_ptr<const char> error = hf.avs_clip_get_error(avsHandle.clip);
    if(error) {
        throw vfg::exception::VideoSourceError(error.get());
    }

    return videoFrameToQImage(frame, info->width, info->height);
}

QString vfg::core::AvisynthVideoSource::getSupportedFormats()
{
    static const QString formats = "Avisynth files (*.avs,*.avsi)";
    return formats;
}

bool vfg::core::AvisynthVideoSource::isValidFrame(const int frameNum) const
{
    return frameNum >= 0 && frameNum < info->num_frames;
}

vfg::ScriptParser vfg::core::AvisynthVideoSource::getParser(const QFileInfo& info) const
{
    const QString absPath = info.absoluteFilePath();
    const QString suffix = info.suffix();
    vfg::ScriptParser parser(absPath);
    if(suffix == "avs" || suffix == "avsi") {
        parser.setTemplate(absPath);
    }
    else if(suffix == "d2v") {
        parser.setTemplate(":/scripts/d2v_template.avs");
    }
    else {
        parser.setTemplate(":/scripts/default_template.avs");
    }

    return parser;
}
