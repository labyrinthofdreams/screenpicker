#include <stdexcept>
#include <QDebug>
#include <QImage>
#include <QMutexLocker>
#include <QPair>
#include <QThread>
#include "videoframegrabber.h"
#include "abstractvideosource.h"

vfg::core::VideoFrameGrabber::VideoFrameGrabber(QObject *parent) :
    QObject(parent),
    avs(),
    numFrames(0),
    currentFrame(0),
    mutex()
{
}

vfg::core::VideoFrameGrabber::VideoFrameGrabber(
        std::shared_ptr<vfg::core::AbstractVideoSource> avs,
        QObject *parent) :
    QObject(parent),
    avs(avs),
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
    avs = newAvs;
    currentFrame = 0;
    numFrames = avs->getNumFrames();

    connect(avs.get(),  SIGNAL(videoLoaded()),
            this,       SLOT(videoSourceUpdated()));
}

int vfg::core::VideoFrameGrabber::lastFrame() const
{
    QMutexLocker lock(&mutex);
    return currentFrame + vfg::FirstFrame;
}

void vfg::core::VideoFrameGrabber::requestFrame(int frameNum)
{
    if(!isValidFrame(frameNum)) {
        emit errorOccurred(tr("Frame number out of range: %1")
                           .arg(QString::number(frameNum)));
        return;
    }

    QMutexLocker ml(&mutex);

    qDebug() << Q_FUNC_INFO << QThread::currentThreadId();
    // Because frame requests are between range 1 - n
    frameNum -= vfg::FirstFrame;
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
    QMutexLocker ml(&mutex);
    qDebug() << "Start NEXT_FRAME VFG ";
    const bool frameIsLast = (currentFrame + vfg::FirstFrame) == numFrames;
    if(frameIsLast)
    {
        emit errorOccurred(tr("Reached last frame"));
        return;
    }

    QImage frame = avs->getFrame(++currentFrame);

    emit frameGrabbed(QPair<int, QImage>(currentFrame, frame));
    qDebug() << "End NEXT_FRAME VFG ";
}

void vfg::core::VideoFrameGrabber::requestPreviousFrame()
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start PREV_FRAME VFG ";
    if(currentFrame == 0)
    {
        emit errorOccurred(tr("Reached first frame"));
        return;
    }

    QImage frame = avs->getFrame(--currentFrame);

    emit frameGrabbed(QPair<int, QImage>(currentFrame, frame));
    qDebug() << "End PREV_FRAME VFG ";
}

QImage vfg::core::VideoFrameGrabber::getFrame(int frameNum)
{
    if(!isValidFrame(frameNum)) {
        emit errorOccurred(tr("Frame number out of range: %1")
                           .arg(QString::number(frameNum)));
        return QImage();
    }

    QMutexLocker ml(&mutex);
    qDebug() << "Start GET_FRAME VFG " << frameNum;
    frameNum -= vfg::FirstFrame;

    QImage frame = avs->getFrame(frameNum);
    qDebug() << "End GET_FRAME VFG Thread ";
    return frame;
}

bool vfg::core::VideoFrameGrabber::isValidFrame(const int frameNum) const
{
    QMutexLocker lock(&mutex);
    return frameNum >= 0 && (frameNum - vfg::FirstFrame) < avs->getNumFrames();
}

int vfg::core::VideoFrameGrabber::totalFrames()
{
    QMutexLocker lock(&mutex);
    numFrames = avs->getNumFrames();
    return numFrames;
}
