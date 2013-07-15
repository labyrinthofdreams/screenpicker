#include <QString>
#include <QImage>
#include <stdexcept>
#include "avisynthvideosource.h"

using namespace vfg;

AvisynthVideoSource::AvisynthVideoSource() :
    AbstractVideoSource(),
    info(NULL)
{
    if(internal_avs_load_library(&avsHandle) < 0)
    {
        // Throw if AviSynth library loading fails
        throw std::runtime_error("Failed to load AviSynth library");
    }

    avsHandle.env = avsHandle.func.avs_create_script_environment(AVS_INTERFACE_25);
    if(avsHandle.func.avs_get_error)
    {
        const char *error = avsHandle.func.avs_get_error(avsHandle.env);
        if(error)
        {
            // Throw if AviSynth script environment fails
            throw std::runtime_error(error);
        }
    }

    avsHandle.func.avs_set_memory_max(avsHandle.env, 128);
}

AvisynthVideoSource::~AvisynthVideoSource()
{
    internal_avs_close_library(&avsHandle, hasVideo());
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
        throw VideoSourceException(avs_as_string(res));
    }
    if(!avs_is_clip(res))
    {
        throw VideoSourceException("Imported script did not return a video clip");
    }

    avsHandle.clip = avsHandle.func.avs_take_clip(res, avsHandle.env);
    info = avsHandle.func.avs_get_video_info(avsHandle.clip);
    if(!avs_has_video(info))
    {
        avsHandle.func.avs_release_value(res);
        throw VideoSourceException("Imported script does not have video data");
    }

    // Video must be RGB32
    if(!avs_is_rgb32(info))
    {
        avsHandle.func.avs_release_value(res);
        throw VideoSourceException("Video is not RGB32. Add ConvertToRGB32() to your script.");
    }

    // Convert input to RGB32
//    const char* arg_name[2] = {NULL, "interlaced"};
//    AVS_Value arg_arr[2] = {res, avs_new_value_bool(0)};
//    AVS_Value conv_tmp = avsHandle.func.avs_invoke(avsHandle.env, "ConvertToRGB",
//                                                   avs_new_value_array(arg_arr, 2), arg_name);
//    if(avs_is_error(conv_tmp))
//    {
//        avsHandle.func.avs_release_value(res);
//        throw std::runtime_error("Failed to convert input clip to RGB32");
//    }

//    res = internal_avs_update_clip(&avsHandle, &info, conv_tmp, res);
    avsHandle.func.avs_release_value(res);
}

bool AvisynthVideoSource::hasVideo() const
{
    return (info != NULL) && avs_has_video(info);
}

unsigned AvisynthVideoSource::getNumFrames() const
{
    return info->num_frames;
}

QImage AvisynthVideoSource::getFrame(unsigned frameNumber)
{
    if(info == NULL)
    {
        return QImage();
    }

    if(frameNumber >= info->num_frames)
    {
        return QImage();
    }

    AVS_VideoFrame *frame = avsHandle.func.avs_get_frame(avsHandle.clip, frameNumber);
    const char* error = avsHandle.func.avs_clip_get_error(avsHandle.clip);
    if(error)
    {
        return QImage();
    }

    QImage image(info->width, info->height, QImage::Format_ARGB32);
    const unsigned scanlineLength = info->width * 4;
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
