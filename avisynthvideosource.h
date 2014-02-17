#ifndef AVISYNTHVIDEOSOURCE_H
#define AVISYNTHVIDEOSOURCE_H

#include "abstractvideosource.h"
#include "avs_internal.c"

namespace vfg {
namespace core {

class AvisynthVideoSource : public vfg::internal::AbstractVideoSource
{
private:
    avs_hnd_t avsHandle;
    const AVS_VideoInfo* info;
public:
    AvisynthVideoSource();
    ~AvisynthVideoSource() override;

    void load(QString fileName) override;
    bool hasVideo() const override;
    int getNumFrames() const override;
    QImage getFrame(int frameNumber) override;
    QString getSupportedFormats() override;
};

} // namespace core
} // namespace vfg

#endif // AVISYNTHVIDEOSOURCE_H
