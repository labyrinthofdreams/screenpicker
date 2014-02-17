#ifndef ABSTRACTVIDEOSOURCE_H
#define ABSTRACTVIDEOSOURCE_H

#include <stdexcept>
#include <string>
#include <QImage>
#include <QObject>
#include <QString>

namespace vfg {
namespace exception {

class VideoSourceError : public std::runtime_error
{
public:
    VideoSourceError(const char* msg) : std::runtime_error(msg) {}
    VideoSourceError(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace exception

namespace core {

class AbstractVideoSource : public QObject
{
    Q_OBJECT
public:
    AbstractVideoSource() {}
    virtual ~AbstractVideoSource() {}
    virtual void load(QString fileName) = 0;
    virtual bool hasVideo() const = 0;
    virtual int getNumFrames() const = 0;
    virtual QImage getFrame(int frameNumber) = 0;
    virtual QString getSupportedFormats() = 0;

signals:
    void videoLoaded();
};

} // namespace internal
} // namespace vfg

#endif // ABSTRACTVIDEOSOURCE_H
