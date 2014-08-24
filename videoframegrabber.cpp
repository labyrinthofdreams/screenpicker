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

vfg::core::VideoFrameGrabber::VideoFrameGrabber(
        std::shared_ptr<vfg::core::AbstractVideoSource> newAvs,
        QObject *parent) :
    QObject(parent),
    avs(std::move(newAvs)),
    numFrames(0),
    currentFrame(0),
    mutex()
{
    if(!avs) {
        qCCritical(GRABBER) << "Invalid video source";
        throw std::runtime_error("Video source must be a valid object");
    }

    connect(avs.get(),  SIGNAL(videoLoaded()),
            this,       SLOT(videoSourceUpdated()));
}

bool vfg::core::VideoFrameGrabber::hasVideo() const
{
    QMutexLocker lock(&mutex);

    return avs && avs->hasVideo();
}

void vfg::core::VideoFrameGrabber::setVideoSource(
        std::shared_ptr<vfg::core::AbstractVideoSource> newAvs)
{
    QMutexLocker lock(&mutex);

    if(!newAvs) {
        qCCritical(GRABBER) << "Invalid video source";
        throw std::runtime_error("Video source must be a valid object");
    }

    qCDebug(GRABBER) << "Set video source";

    disconnect(avs.get(), 0, this, 0);
    avs = std::move(newAvs);

    connect(avs.get(),  SIGNAL(videoLoaded()),
            this,       SLOT(videoSourceUpdated()));

    currentFrame = 0;
    numFrames = avs->getNumFrames();
}

int vfg::core::VideoFrameGrabber::lastFrame() const
{
    QMutexLocker lock(&mutex);

    return currentFrame;
}

void vfg::core::VideoFrameGrabber::requestFrame(const int frameNum)
{
    QMutexLocker ml(&mutex);

    if(!avs->isValidFrame(frameNum)) {
        qCCritical(GRABBER) << "Frame number out of range:" << frameNum;
        emit errorOccurred(tr("Frame number out of range: %1")
                           .arg(QString::number(frameNum)));
        return;
    }

    qCDebug(GRABBER) << "Request frame" << frameNum << "from" << QThread::currentThreadId();

    currentFrame = frameNum;
    QImage frame = avs->getFrame(frameNum);

    emit frameGrabbed(frameNum, std::move(frame));
}

void vfg::core::VideoFrameGrabber::videoSourceUpdated()
{
    QMutexLocker ml(&mutex);

    numFrames = avs->getNumFrames();
}

void vfg::core::VideoFrameGrabber::requestNextFrame()
{
    QMutexLocker ml(&mutex);

    if(currentFrame == numFrames) {
        qCDebug(GRABBER) << "Reached last frame";

        emit errorOccurred(tr("Reached last frame"));

        return;
    }

    qCDebug(GRABBER) << "Requesting next frame" << currentFrame + 1;

    QImage frame = avs->getFrame(++currentFrame);
    emit frameGrabbed(currentFrame, std::move(frame));
}

void vfg::core::VideoFrameGrabber::requestPreviousFrame()
{
    QMutexLocker ml(&mutex);

    qDebug() << "Start PREV_FRAME VFG ";

    if(currentFrame == 0) {
        qCDebug(GRABBER) << "Reached first frame";

        emit errorOccurred(tr("Reached first frame"));

        return;
    }

    qCDebug(GRABBER) << "Requesting previous frame" << currentFrame - 1;

    QImage frame = avs->getFrame(--currentFrame);
    emit frameGrabbed(currentFrame, std::move(frame));
}

QImage vfg::core::VideoFrameGrabber::getFrame(const int frameNum)
{
    QMutexLocker ml(&mutex);

    qCDebug(GRABBER) << "Get frame" << frameNum;

    if(!avs->isValidFrame(frameNum)) {
        qCCritical(GRABBER) << "Frame out of range:" << frameNum << "(" << avs->getNumFrames() << ")";
        return {};
    }

    return avs->getFrame(frameNum);
}

bool vfg::core::VideoFrameGrabber::isValidFrame(const int frameNum) const
{
    QMutexLocker lock(&mutex);

    return frameNum >= 0 && frameNum < avs->getNumFrames();
}

int vfg::core::VideoFrameGrabber::totalFrames()
{
    QMutexLocker lock(&mutex);

    numFrames = avs->getNumFrames();
    return numFrames;
}

QSize vfg::core::VideoFrameGrabber::resolution() const
{
    QMutexLocker lock(&mutex);

    return avs->resolution();
}
