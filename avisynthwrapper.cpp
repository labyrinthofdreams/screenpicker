#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include "avisynthwrapper.hpp"
#include "ptrutil.hpp"

/**
 * @brief Call Deleter D on object T on scope exit
 *
 * RaiiDeleter wraps an object of type T and calls
 * Deleter D when the object is destructed
 */
template <class T, class D>
class RaiiDeleter {
    T _object;
    D _deleter;

public:
    RaiiDeleter(T object, D deleter) :
        _object(object),
        _deleter(deleter) {
    }

    ~RaiiDeleter() {
        _deleter(_object);
    }

    RaiiDeleter& operator=(const T& other) {
        _object = other;
        return *this;
    }

    RaiiDeleter& operator=(T&& other) {
        _object = std::move(other);
        return *this;
    }

    T& get() {
        return _object;
    }
};

vfg::avisynth::VideoFrame::VideoFrame(std::unique_ptr<AVS_VideoFrame, Deleter> frame) :
    videoFrame(std::move(frame)) {
}

bool vfg::avisynth::VideoFrame::isValid() const {
    return !!videoFrame;
}

int vfg::avisynth::VideoFrame::pitch() const {
    return avs_get_pitch(videoFrame.get());
}

vfg::avisynth::VideoFrame::DataReadPtr
vfg::avisynth::VideoFrame::data() const {
    return avs_get_read_ptr(videoFrame.get());
}

vfg::exception::AvisynthError::AvisynthError(const char *msg) :
    std::runtime_error(msg) {
}

vfg::exception::AvisynthError::AvisynthError(const std::string& msg) :
    std::runtime_error(msg) {
}

vfg::avisynth::AvisynthWrapper::AvisynthWrapper() :
    avsHandle(),
    info()
{
    if(internal_avs_load_library(&avsHandle) < 0) {
        throw vfg::exception::AvisynthError("Failed to load AviSynth library");
    }

    avsHandle.env = avsHandle.func.avs_create_script_environment(AVS_INTERFACE_25);
    if(avsHandle.func.avs_get_error) {
        const auto error(avsHandle.func.avs_get_error(avsHandle.env));
        if(error) {
            throw vfg::exception::AvisynthError(
                        std::string("Failed to create script environment: ") + error);
        }
    }
}

vfg::avisynth::AvisynthWrapper::~AvisynthWrapper() {
    if(hasVideo()) {
        avsHandle.func.avs_release_clip(avsHandle.clip);
    }
    if(avsHandle.func.avs_delete_script_environment) {
        avsHandle.func.avs_delete_script_environment(avsHandle.env);
    }
    FreeLibrary(avsHandle.library);
}

void vfg::avisynth::AvisynthWrapper::load(const std::string& path) {
    RaiiDeleter<AVS_Value, decltype(avsHandle.func.avs_release_value)> res(
                avs_void, avsHandle.func.avs_release_value);
    {
        // Initialize res by attempting to import the script
        RaiiDeleter<AVS_Value, decltype(avsHandle.func.avs_release_value)> arg(
                    avs_void, avsHandle.func.avs_release_value);
        arg = avs_new_value_string(path.c_str());
        res = avsHandle.func.avs_invoke(avsHandle.env, "Import", arg.get(), nullptr);
    }

    if(avs_is_error(res.get())) {
        throw vfg::exception::AvisynthError(avs_as_string(res.get()));
    }

    if(!avs_is_clip(res.get())) {
        throw vfg::exception::AvisynthError("Imported script did not return a video clip");
    }

    avsHandle.clip = avsHandle.func.avs_take_clip(res.get(), avsHandle.env);
    info = avsHandle.func.avs_get_video_info(avsHandle.clip);
    if(!avs_has_video(info.get())) {
        avsHandle.func.avs_release_clip(avsHandle.clip);
        info.reset();
        throw vfg::exception::AvisynthError("Imported script does not have video data");
    }

    openFilePath = path;
}

vfg::avisynth::VideoFrame
vfg::avisynth::AvisynthWrapper::getFrame(const int frameNum) {
    if(!hasVideo()) {
        throw vfg::exception::AvisynthError("No video");
    }

    if(frameNum < 0 || frameNum >= numFrames()) {
        throw vfg::exception::AvisynthError("Requested frame number is out of range");
    }

    std::unique_ptr<AVS_VideoFrame,
                    decltype(avsHandle.func.avs_release_video_frame)> frame(
                avsHandle.func.avs_get_frame(avsHandle.clip, frameNum),
                avsHandle.func.avs_release_video_frame);
    const auto error = avsHandle.func.avs_clip_get_error(avsHandle.clip);
    if(error) {
        throw vfg::exception::AvisynthError(error);
    }

    return {std::move(frame)};
}

bool vfg::avisynth::AvisynthWrapper::hasVideo() const {
    return info && avs_has_video(info.get());
}

int vfg::avisynth::AvisynthWrapper::numFrames() const {
    return hasVideo() ? info->num_frames : 0;
}

vfg::avisynth::PixelFormat
vfg::avisynth::AvisynthWrapper::pixelFormat() const {
    if(!hasVideo()) {
        return PixelFormat::Unknown;
    }

    const auto& vi = info.get();
    if(avs_is_rgb24(vi)) {
        return PixelFormat::BGR24;
    }
    else if(avs_is_rgb32(vi)) {
        return PixelFormat::BGR32;
    }
    else if(avs_is_yuy2(vi)) {
        return PixelFormat::YUY2;
    }
    else if(avs_is_yv24(vi)) {
        return PixelFormat::YV24;
    }
    else if(avs_is_yv16(vi)) {
        return PixelFormat::YV16;
    }
    else if(avs_is_yv12(vi)) {
        return PixelFormat::YV12;
    }
    else if(avs_is_yv411(vi)) {
        return PixelFormat::YV411;
    }
    else if(avs_is_y8(vi)) {
        return PixelFormat::Y8;
    }
    else {
        return PixelFormat::Unknown;
    }
}

bool vfg::avisynth::AvisynthWrapper::isRGB() const {
    return hasVideo() && avs_is_rgb(info.get());
}

bool vfg::avisynth::AvisynthWrapper::isYUV() const {
    return hasVideo() && avs_is_yuv(info.get());
}

int vfg::avisynth::AvisynthWrapper::width() const {
    return hasVideo() ? info->width : 0;
}

int vfg::avisynth::AvisynthWrapper::height() const {
    return hasVideo() ? info->height : 0;
}

std::string vfg::avisynth::AvisynthWrapper::fileName() const {
    return openFilePath;
}
