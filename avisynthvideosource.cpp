#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <QFileInfo>
#include <QImage>
#include <QSize>
#include <QString>
#include "avisynthvideosource.h"
#include "ptrutil.hpp"
#include "scriptparser.h"

// TODO: Re-throw AvisynthErrors as VideoSourceErrors

/**
 * @brief Convert AVS_VideoFrame to QImage (32-bit ARGB)
 * @param frame VideoFrame
 * @exception std::runtime_error If frame is nullptr
 * @return Frame as QImage
 */
static
QImage videoFrameToQImage(const vfg::avisynth::VideoFrame& frame,
                          const std::size_t width,
                          const std::size_t height) {
    if(!frame.isValid()) {
        throw std::runtime_error("Frame must be a valid object");
    }

    const auto scanlineLength = width * 4;
    const auto pitch = frame.pitch();
    const auto data = frame.data();
    QImage image(width, height, QImage::Format_ARGB32);
    for(std::size_t y = 0, sy = height - 1; y < height; ++y, --sy) {
        // If we used y instead of sy in the scanLine function
        // the image would be created upside-down
        std::memcpy(image.scanLine(sy), data + y * pitch, scanlineLength);
    }

    return image;
}

vfg::core::AvisynthVideoSource::AvisynthVideoSource() :
    AbstractVideoSource(),
    avs()
{

}

void vfg::core::AvisynthVideoSource::load(const QString& fileName)
{
    avs.load(fileName.toStdString());

    // Video must be RGB32
    if(avs.pixelFormat() != vfg::avisynth::PixelFormat::BGR32) {
        throw vfg::exception::VideoSourceError("Video is not RGB32. Add ConvertToRGB32() to your script.");
    }

    emit videoLoaded();
}

bool vfg::core::AvisynthVideoSource::hasVideo() const
{
    return avs.hasVideo();
}

int vfg::core::AvisynthVideoSource::getNumFrames() const
{
    return avs.numFrames();
}

QImage vfg::core::AvisynthVideoSource::getFrame(const int frameNumber)
{
    const auto frame = avs.getFrame(frameNumber);

    return videoFrameToQImage(frame, avs.width(), avs.height());
}

QString vfg::core::AvisynthVideoSource::getSupportedFormats()
{
    static const QString formats = "Avisynth files (*.avs,*.avsi)";
    return formats;
}

bool vfg::core::AvisynthVideoSource::isValidFrame(const int frameNum) const
{
    return avs.hasVideo() && frameNum >= 0 && frameNum < avs.numFrames();
}

vfg::ScriptParser
vfg::core::AvisynthVideoSource::getParser(const QFileInfo& info) const
{
    const QString absPath = info.absoluteFilePath();
    const QString suffix = info.suffix();
    vfg::ScriptParser parser(absPath);
    if(suffix == "avs" || suffix == "avsi") {
        parser.setTemplate(absPath);
    }
    else if(suffix == "d2v") {
        parser.setTemplate("scripts/d2v_template.avs");
    }
    else {
        parser.setTemplate("scripts/default_template.avs");
    }

    return parser;
}

QSize vfg::core::AvisynthVideoSource::resolution() const
{
    return QSize(avs.width(), avs.height());
}
