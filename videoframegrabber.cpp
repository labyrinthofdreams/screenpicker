#include <stdexcept>
#include <utility>
#include <QDebug>
#include <QImage>
#include <QMutexLocker>
#include <QPair>
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
    if(!newAvs) {
        throw std::runtime_error("Video source must be a valid object");
    }

    QMutexLocker lock(&mutex);

    disconnect(avs.get(), 0, this, 0);

    avs = std::move(newAvs);
    currentFrame = 0;
    numFrames = avs->getNumFrames();

    connect(avs.get(),  SIGNAL(videoLoaded()),
            this,       SLOT(videoSourceUpdated()));
}

int vfg::core::VideoFrameGrabber::lastFrame() const
{
    QMutexLocker lock(&mutex);
    return currentFrame;
}

void vfg::core::VideoFrameGrabber::requestFrame(const int frameNum)
{
    if(!isValidFrame(frameNum)) {
        emit errorOccurred(tr("Frame number out of range: %1")
                           .arg(QString::number(frameNum)));
        return;
    }

    QMutexLocker ml(&mutex);

    qDebug() << Q_FUNC_INFO << QThread::currentThreadId();
    currentFrame = frameNum;

    QImage frame = avs->getFrame(frameNum);

    //ml.unlock();
    emit frameGrabbed(QPair<int, QImage>(frameNum, frame));
}

void vfg::core::VideoFrameGrabber::videoSourceUpdated()
{
    QMutexLocker ml(&mutex);
    numFrames = avs->getNumFrames();
}

void vfg::core::VideoFrameGrabber::requestNextFrame()
{
    qDebug() << "Start NEXT_FRAME VFG ";

    QMutexLocker ml(&mutex);

    if(currentFrame == numFrames) {
        emit errorOccurred(tr("Reached last frame"));
        return;
    }

    QImage frame = avs->getFrame(++currentFrame);
    emit frameGrabbed(QPair<int, QImage>(currentFrame, frame));

    qDebug() << "End NEXT_FRAME VFG ";
}

void vfg::core::VideoFrameGrabber::requestPreviousFrame()
{
    qDebug() << "Start PREV_FRAME VFG ";

    QMutexLocker ml(&mutex);

    if(currentFrame == 0)
    {
        emit errorOccurred(tr("Reached first frame"));
        return;
    }

    QImage frame = avs->getFrame(--currentFrame);
    emit frameGrabbed(QPair<int, QImage>(currentFrame, frame));

    qDebug() << "End PREV_FRAME VFG ";
}

QImage vfg::core::VideoFrameGrabber::getFrame(const int frameNum)
{
    if(!isValidFrame(frameNum)) {
        emit errorOccurred(tr("Frame number out of range: %1")
                           .arg(QString::number(frameNum)));
        return QImage();
    }

    QMutexLocker ml(&mutex);
    qDebug() << "Start GET_FRAME VFG " << frameNum;

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
