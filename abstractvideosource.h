#ifndef ABSTRACTVIDEOSOURCE_H
#define ABSTRACTVIDEOSOURCE_H

#include <stdexcept>
#include <string>
#include <QString>
#include <QImage>

namespace vfg
{
    class VideoSourceException : public std::runtime_error
    {
    public:
        VideoSourceException(const char* msg) : std::runtime_error(msg) {}
        VideoSourceException(const std::string& msg) : std::runtime_error(msg) {}
    };

    class AbstractVideoSource
    {
    public:
        AbstractVideoSource() {}
        virtual ~AbstractVideoSource() {}
        virtual void load(QString fileName) = 0;
        virtual bool hasVideo() const = 0;
        virtual unsigned getNumFrames() const = 0;
        virtual QImage getFrame(unsigned frameNumber) = 0;
        virtual QString getSupportedFormats() = 0;
    };
}

#endif // ABSTRACTVIDEOSOURCE_H
