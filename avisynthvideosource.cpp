#include <QString>
#include <QImage>
#include <stdexcept>
#include "avisynthvideosource.h"

using namespace vfg::core;
using namespace vfg::exception;

AvisynthVideoSource::AvisynthVideoSource() :
    AbstractVideoSource(),
    info(nullptr)
{
    if(internal_avs_load_library(&avsHandle) < 0)
    {
        // Throw if AviSynth library loading fails
        throw VideoSourceError("Failed to load AviSynth library");
    }

    avsHandle.env = avsHandle.func.avs_create_script_environment(AVS_INTERFACE_25);
    if(avsHandle.func.avs_get_error)
    {
        const char *error = avsHandle.func.avs_get_error(avsHandle.env);
        if(error)
        {
            // Throw if AviSynth script environment fails
            throw VideoSourceError(error);
        }
    }
}

AvisynthVideoSource::~AvisynthVideoSource()
{
    if(hasVideo()) {
        avsHandle.func.avs_release_clip(avsHandle.clip);
    }
    if(avsHandle.func.avs_delete_script_environment) {
        avsHandle.func.avs_delete_script_environment(avsHandle.env);
    }
    FreeLibrary(avsHandle.library);
}

void AvisynthVideoSource::load(QString fileName)
{
    // If you don't store the converted data temporarily,
    // you will have a memory leak (or so they say)
    QByteArray tmp = fileName.toLocal8Bit();
    AVS_Value arg = avs_new_value_string(tmp.constData());
    AVS_Value res = avsHandle.func.avs_invoke(avsHandle.env, "Import", arg, NULL);
    if(avs_is_error(res))
    {
        // Throw if our imported script fails
        throw VideoSourceError(avs_as_string(res));
    }
    if(!avs_is_clip(res))
    {
        throw VideoSourceError("Imported script did not return a video clip");
    }

    avsHandle.clip = avsHandle.func.avs_take_clip(res, avsHandle.env);
    info = avsHandle.func.avs_get_video_info(avsHandle.clip);
    if(!avs_has_video(info))
    {
        avsHandle.func.avs_release_value(res);
        throw VideoSourceError("Imported script does not have video data");
    }

    // Video must be RGB32
    if(!avs_is_rgb32(info))
    {
        avsHandle.func.avs_release_value(res);
        throw VideoSourceError("Video is not RGB32. Add ConvertToRGB32() to your script.");
    }

    avsHandle.func.avs_release_value(res);

    emit videoLoaded();
}

bool AvisynthVideoSource::hasVideo() const
{
    return (info != nullptr) && avs_has_video(info);
}

int AvisynthVideoSource::getNumFrames() const
{
    if(info == nullptr) {
        return 0;
    }

    return info->num_frames;
}

QImage AvisynthVideoSource::getFrame(int frameNumber)
{
    if(info == nullptr)
    {
        return {};
    }

    if(frameNumber < 0 || frameNumber >= info->num_frames)
    {
        return {};
    }

    AVS_VideoFrame *frame = avsHandle.func.avs_get_frame(avsHandle.clip, frameNumber);
    const char* error = avsHandle.func.avs_clip_get_error(avsHandle.clip);
    if(error)
    {
        return {};
    }

    QImage image(info->width, info->height, QImage::Format_ARGB32);
    const int scanlineLength = info->width * 4;
    const int pitch = avs_get_pitch(frame);
    const BYTE* data = avs_get_read_ptr(frame);
    // If we used y instead of sy the image would be created upside-down
    for(int y = 0, sy = info->height - 1; y < info->height; ++y, --sy)
    {
        memcpy(image.scanLine(sy), data + y * pitch, scanlineLength);
    }

    avsHandle.func.avs_release_video_frame(frame);

    return image;
}

QString AvisynthVideoSource::getSupportedFormats()
{
    static const QString formats = "Avisynth files (*.avs,*.avsi)";
    return formats;
}

bool AvisynthVideoSource::isValidFrame(int frameNum) const
{
    return frameNum >= 0 && frameNum < info->num_frames;
}
