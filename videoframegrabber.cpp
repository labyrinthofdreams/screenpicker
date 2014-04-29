#include <QDebug>
#include <QMutexLocker>
#include <QPair>
#include <QThread>
#include "videoframegrabber.h"
#include "abstractvideosource.h"

using vfg::core::VideoFrameGrabber;

VideoFrameGrabber::VideoFrameGrabber(QObject *parent) :
    QObject(parent),
    avs(),
    numFrames(0),
    currentFrame(0),
    mutex()
{
}

VideoFrameGrabber::VideoFrameGrabber(std::shared_ptr<vfg::core::AbstractVideoSource> avs,
                                          QObject *parent) :
    QObject(parent),
    avs(avs),
    numFrames(0),
    currentFrame(0),
    mutex()
{
    connect(avs.get(),  SIGNAL(videoLoaded()),
            this,       SLOT(videoSourceUpdated()));
}

bool VideoFrameGrabber::hasVideo() const
{
    QMutexLocker lock(&mutex);
    return avs && avs->hasVideo();
}

void VideoFrameGrabber::setVideoSource(std::shared_ptr<vfg::core::AbstractVideoSource> newAvs)
{
    QMutexLocker lock(&mutex);
    avs = newAvs;
    currentFrame = 0;
    numFrames = avs->getNumFrames();

    connect(avs.get(),  SIGNAL(videoLoaded()),
            this,       SLOT(videoSourceUpdated()));
}

int VideoFrameGrabber::lastFrame() const
{
    QMutexLocker lock(&mutex);
    return currentFrame + vfg::FirstFrame;
}

void VideoFrameGrabber::requestFrame(int frameNum)
{
    QMutexLocker ml(&mutex);

    qDebug() << Q_FUNC_INFO << QThread::currentThreadId();
    // Because frame requests are between range 1 - n
    frameNum -= vfg::FirstFrame;
    currentFrame = frameNum;

    QImage frame = avs->getFrame(frameNum);

    //ml.unlock();
    emit frameGrabbed(QPair<int, QImage>(frameNum, frame));
}

void VideoFrameGrabber::videoSourceUpdated()
{
    QMutexLocker ml(&mutex);
    numFrames = avs->getNumFrames();
}

void VideoFrameGrabber::requestNextFrame()
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start NEXT_FRAME VFG ";
    bool frameIsLast = (currentFrame + vfg::FirstFrame) == numFrames;
    if(frameIsLast)
    {
        emit errorOccurred(tr("Reached last frame"));
        return;
    }

    ++currentFrame;
    QImage frame = avs->getFrame(currentFrame);

    emit frameGrabbed(QPair<int, QImage>(currentFrame, frame));
    qDebug() << "End NEXT_FRAME VFG ";
}

void VideoFrameGrabber::requestPreviousFrame()
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start PREV_FRAME VFG ";
    if(currentFrame == 0)
    {
        emit errorOccurred(tr("Reached first frame"));
        return;
    }

    --currentFrame;
    QImage frame = avs->getFrame(currentFrame);

    emit frameGrabbed(QPair<int, QImage>(currentFrame, frame));
    qDebug() << "End PREV_FRAME VFG ";
}

QImage VideoFrameGrabber::getFrame(int frameNum)
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start GET_FRAME VFG " << frameNum;
    frameNum -= vfg::FirstFrame;

    const bool invalidRange = frameNum >= numFrames;
    if(invalidRange)
    {
        emit errorOccurred(tr("Out of range"));
        return QImage();
    }

    QImage frame = avs->getFrame(frameNum);
    qDebug() << "End GET_FRAME VFG Thread ";
    return frame;
}

bool VideoFrameGrabber::isValidFrame(int frameNum) const
{
    QMutexLocker lock(&mutex);
    return frameNum >= 0 && (frameNum - vfg::FirstFrame) < avs->getNumFrames();
}

int VideoFrameGrabber::totalFrames()
{
    QMutexLocker lock(&mutex);
    numFrames = avs->getNumFrames();
    return numFrames;
}
