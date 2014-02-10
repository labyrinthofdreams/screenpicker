#ifndef AVISYNTHVIDEOSOURCE_H
#define AVISYNTHVIDEOSOURCE_H

#include "abstractvideosource.h"
#include "avs_internal.c"

namespace vfg
{
    class AvisynthVideoSource : public vfg::AbstractVideoSource
    {
    private:
        avs_hnd_t avsHandle;
        const AVS_VideoInfo* info;
    public:
        AvisynthVideoSource();
        ~AvisynthVideoSource();

        void load(QString fileName);
        bool hasVideo() const;
        int getNumFrames() const;
        QImage getFrame(int frameNumber);
        QString getSupportedFormats();
    };
}

#endif // AVISYNTHVIDEOSOURCE_H
