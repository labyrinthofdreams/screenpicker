#include <stdexcept>
#include <utility>
#include <QDebug>
#include <QImage>
#include <QMutexLocker>
#include <QSize>
#include <QThread>
#include "videoframegrabber.h"
#include "abstractvideosource.h"

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
        throw std::runtime_error("Video source must be a valid object");
    }

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
        emit errorOccurred(tr("Frame number out of range: %1")
                           .arg(QString::number(frameNum)));
        return;
    }

    qDebug() << Q_FUNC_INFO << QThread::currentThreadId();

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

    qDebug() << "Start NEXT_FRAME VFG ";

    if(currentFrame == numFrames) {
        emit errorOccurred(tr("Reached last frame"));

        return;
    }

    QImage frame = avs->getFrame(++currentFrame);
    emit frameGrabbed(currentFrame, std::move(frame));

    qDebug() << "End NEXT_FRAME VFG ";
}

void vfg::core::VideoFrameGrabber::requestPreviousFrame()
{
    QMutexLocker ml(&mutex);

    qDebug() << "Start PREV_FRAME VFG ";

    if(currentFrame == 0) {
        emit errorOccurred(tr("Reached first frame"));

        return;
    }

    QImage frame = avs->getFrame(--currentFrame);
    emit frameGrabbed(currentFrame, std::move(frame));

    qDebug() << "End PREV_FRAME VFG ";
}

QImage vfg::core::VideoFrameGrabber::getFrame(const int frameNum)
{
    QMutexLocker ml(&mutex);

    qDebug() << "Start GET_FRAME VFG " << frameNum;

    if(!avs->isValidFrame(frameNum)) {
        throw std::runtime_error(tr("Frame number out of range: %1")
                           .arg(QString::number(frameNum)).toStdString());
    }

    QImage frame = avs->getFrame(frameNum);

    qDebug() << "End GET_FRAME VFG Thread ";

    return frame;
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
