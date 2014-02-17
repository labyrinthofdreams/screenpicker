#ifndef ABSTRACTVIDEOSOURCE_H
#define ABSTRACTVIDEOSOURCE_H

#include <stdexcept>
#include <string>
#include <QString>
#include <QImage>

namespace vfg{
namespace exception {

class VideoSourceError : public std::runtime_error
{
public:
    VideoSourceError(const char* msg) : std::runtime_error(msg) {}
    VideoSourceError(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace exception



class AbstractVideoSource
{
public:
    AbstractVideoSource() {}
    virtual ~AbstractVideoSource() {}
    virtual void load(QString fileName) = 0;
    virtual bool hasVideo() const = 0;
    virtual int getNumFrames() const = 0;
    virtual QImage getFrame(int frameNumber) = 0;
    virtual QString getSupportedFormats() = 0;
};

} // namespace vfg

#endif // ABSTRACTVIDEOSOURCE_H
