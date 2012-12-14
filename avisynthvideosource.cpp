#include <QtGui>
#include <stdexcept>
#include "avisynthvideosource.h"

using namespace vfg;

AvisynthVideoSource::AvisynthVideoSource() :
    AbstractVideoSource()
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
}

AvisynthVideoSource::~AvisynthVideoSource()
{
    internal_avs_close_library(&avsHandle);
}

void AvisynthVideoSource::load(QString fileName)
{
    // If you don't store the converted data temporarily,
    // you will have a memory leak
    QByteArray tmp = fileName.toLocal8Bit();
    AVS_Value arg = avs_new_value_string(tmp.constData());
    AVS_Value res = avsHandle.func.avs_invoke(avsHandle.env, "Import", arg, NULL);
    if(avs_is_error(res))
    {
        // Throw if our imported script fails
        throw std::runtime_error(avs_as_string(res));
    }
    if(!avs_is_clip(res))
    {
        throw std::runtime_error("Imported script did not return a video clip");
    }

    avsHandle.clip = avsHandle.func.avs_take_clip(res, avsHandle.env);
    info = avsHandle.func.avs_get_video_info(avsHandle.clip);
    if(!avs_has_video(info))
    {
        avsHandle.func.avs_release_value(res);
        throw std::runtime_error("Imported script does not have video data");
    }

    // Convert input to RGB32
    const char* arg_name[2] = {NULL, "interlaced"};
    AVS_Value arg_arr[2] = {res, avs_new_value_bool(0)};
    AVS_Value conv_tmp = avsHandle.func.avs_invoke(avsHandle.env, "ConvertToRGB",
                                                   avs_new_value_array(arg_arr, 2), arg_name);
    if(avs_is_error(conv_tmp))
    {
        avsHandle.func.avs_release_value(res);
        throw std::runtime_error("Failed to convert input clip to RGB24");
    }

    res = internal_avs_update_clip(&avsHandle, &info, conv_tmp, res);
    avsHandle.func.avs_release_value(res);
}

QImage AvisynthVideoSource::getFrame(unsigned frameNumber)
{
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

    qDebug() << frame->pitch << frame->row_size;

    QImage image(info->width, info->height, QImage::Format_ARGB32);
    const unsigned scanlineLength = info->width * 4;
    const int pitch = avs_get_pitch(frame);
    const BYTE* data = avs_get_read_ptr(frame);
    for(int y = 0; y < info->height; ++y)
    {
        memcpy(image.scanLine(y), data + y * pitch, scanlineLength);
    }

    avsHandle.func.avs_release_video_frame(frame);

    return image;
}

QString AvisynthVideoSource::getSupportedFormats()
{
    static QString formats = "Avisynth files (*.avs)";
    return formats;
}
