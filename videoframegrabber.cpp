#include <stdexcept>
#include <utility>
#include <QDebug>
#include <QImage>
#include <QLoggingCategory>
#include <QMutexLocker>
#include <QSize>
#include <QThread>
#include "videoframegrabber.h"
#include "abstractvideosource.h"

Q_LOGGING_CATEGORY(GRABBER, "videoframegrabber")

namespace vfg {
namespace core {

VideoFrameGrabber::VideoFrameGrabber(
        std::shared_ptr<vfg::core::AbstractVideoSource> newAvs,
        QObject *parent) :
    QObject(parent),
    avs(std::move(newAvs))
{
    if(!avs) {
        qCCritical(GRABBER) << "Invalid video source";
        throw std::runtime_error("Video source must be a valid object");
    }
}

bool VideoFrameGrabber::hasVideo() const
{
    QMutexLocker lock(&mutex);

    return avs && avs->hasVideo();
}

void VideoFrameGrabber::setVideoSource(
        std::shared_ptr<vfg::core::AbstractVideoSource> newAvs)
{
    QMutexLocker lock(&mutex);

    if(!newAvs) {
        qCCritical(GRABBER) << "Invalid video source";
        throw std::runtime_error("Video source must be a valid object");
    }

    disconnect(avs.get(), 0);
    avs = std::move(newAvs);
    currentFrame = 0;
}

int VideoFrameGrabber::lastFrame() const
{
    QMutexLocker lock(&mutex);

    return currentFrame;
}

void VideoFrameGrabber::requestFrame(const int frameNum)
{
    QMutexLocker ml(&mutex);

    if(!avs->isValidFrame(frameNum)) {
        qCCritical(GRABBER) << "Frame number out of range:" << frameNum;
        emit errorOccurred(tr("Frame number out of range: %1")
                           .arg(QString::number(frameNum)));
        return;
    }

    currentFrame = frameNum;
    emit frameGrabbed(frameNum, avs->getFrame(frameNum));
}

void VideoFrameGrabber::requestNextFrame()
{
    QMutexLocker ml(&mutex);

    const auto nextFrame = currentFrame + 1;
    if(nextFrame >= avs->getNumFrames()) {
        qCDebug(GRABBER) << "Reached last frame";
        emit errorOccurred(tr("Reached last frame"));
        return;
    }

    ++currentFrame;
    emit frameGrabbed(nextFrame, avs->getFrame(nextFrame));
}

void VideoFrameGrabber::requestPreviousFrame()
{
    QMutexLocker ml(&mutex);

    if(currentFrame == 0) {
        qCDebug(GRABBER) << "Reached first frame";
        emit errorOccurred(tr("Reached first frame"));
        return;
    }

    --currentFrame;
    emit frameGrabbed(currentFrame, avs->getFrame(currentFrame));
}

QImage VideoFrameGrabber::getFrame(const int frameNum)
{
    QMutexLocker ml(&mutex);

    if(!avs->isValidFrame(frameNum)) {
        qCCritical(GRABBER) << "Frame out of range:" << frameNum << "(" << avs->getNumFrames() << ")";
        return {};
    }

    return avs->getFrame(frameNum);
}

bool VideoFrameGrabber::isValidFrame(const int frameNum) const
{
    QMutexLocker lock(&mutex);

    return frameNum >= 0 && frameNum < avs->getNumFrames();
}

int VideoFrameGrabber::totalFrames() const
{
    QMutexLocker lock(&mutex);

    return avs->getNumFrames();
}

QSize VideoFrameGrabber::resolution() const
{
    QMutexLocker lock(&mutex);

    return avs->resolution();
}

} // namespace core
} // namespace vfg
