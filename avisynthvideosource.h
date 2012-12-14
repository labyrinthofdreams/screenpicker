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
        QImage getFrame(unsigned frameNumber);

        static QString getSupportedFormats();
    };
}

#endif // AVISYNTHVIDEOSOURCE_H
