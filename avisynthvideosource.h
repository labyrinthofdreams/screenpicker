#ifndef AVISYNTHVIDEOSOURCE_H
#define AVISYNTHVIDEOSOURCE_H

#include "abstractvideosource.h"
#include "avisynthwrapper.hpp"

namespace vfg {
namespace core {

class AvisynthVideoSource : public vfg::core::AbstractVideoSource
{
private:
    vfg::avisynth::AvisynthWrapper avs {};

public:
    AvisynthVideoSource();
    ~AvisynthVideoSource() override = default;

    /**
     * @brief Load file
     * @param fileName File to load
     * @throws vfg::exception::VideoSourceError If video is not RGB32
     * @throws vfg::exception::VideoSourceError If loading video fails
     */
    void load(const QString& fileName) override;
    bool hasVideo() const override;
    int getNumFrames() const override;
    QImage getFrame(int frameNumber) override;
    QString getSupportedFormats() override;
    bool isValidFrame(int frameNum) const override;
    vfg::ScriptParser getParser(const QFileInfo &info) const override;
    QSize resolution() const override;
    QString fileName() const override;
};

} // namespace core
} // namespace vfg

#endif // AVISYNTHVIDEOSOURCE_H
