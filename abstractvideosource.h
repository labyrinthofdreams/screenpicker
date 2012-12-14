#ifndef ABSTRACTVIDEOSOURCE_H
#define ABSTRACTVIDEOSOURCE_H

#include <QString>
#include <QImage>

namespace vfg
{
    class AbstractVideoSource
    {
    public:
        AbstractVideoSource() {}
        virtual ~AbstractVideoSource() {}
        virtual void load(QString fileName) = 0;
        virtual unsigned getNumFrames() const = 0;
        virtual QImage getFrame(unsigned frameNumber) = 0;
        virtual QString getSupportedFormats() = 0;
    };
}

#endif // ABSTRACTVIDEOSOURCE_H
