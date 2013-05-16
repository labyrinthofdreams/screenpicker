#include "videoframegrabber.h"
#include "abstractvideosource.h"
#include <QDebug>
#include <QThread>
#include <QMutexLocker>

vfg::VideoFrameGrabber::VideoFrameGrabber(QSharedPointer<vfg::AbstractVideoSource> avs,
                                          QObject *parent) :
    QObject(parent),
    avs(avs),
    numFrames(0),
    currentFrame(0),
    running(false),
    ctr(0)
{
}

vfg::VideoFrameGrabber::~VideoFrameGrabber()
{
    qDebug() << "VFG destructed from thread " << thread()->currentThreadId();
}

//void vfg::VideoFrameGrabber::load(QString filename)
//{
//    try
//    {
//        avs->load(filename);
//        numFrames = avs->getNumFrames();
//        currentFrame = 0;

//        emit videoReady();
//    }
//    catch(std::exception& ex)
//    {
//        emit errorOccurred(ex.what());
//    }
//}

bool vfg::VideoFrameGrabber::hasVideo() const
{
    return avs->hasVideo();
}

void vfg::VideoFrameGrabber::setVideoSource(QSharedPointer<vfg::AbstractVideoSource> newAvs)
{
    avs = newAvs;
    numFrames = avs->getNumFrames();
    currentFrame = 0;

    emit videoReady();
}

const vfg::AbstractVideoSource* vfg::VideoFrameGrabber::getVideoSource() const
{
    return avs.data();
}

unsigned vfg::VideoFrameGrabber::lastFrame() const
{
    return currentFrame + 1;
}

void vfg::VideoFrameGrabber::requestFrame(unsigned frameNum)
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start REQUEST_FRAME VFG " << ctr;
    // Because frame requests are between range 1 - n
    --frameNum;
    currentFrame = frameNum;

    QImage frame = avs->getFrame(frameNum);

    emit frameGrabbed(frame);
    qDebug() << "End REQUEST_FRAME VFG " << ctr;
    ++ctr;
}

void vfg::VideoFrameGrabber::requestNextFrame()
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start NEXT_FRAME VFG " << ctr;
    bool frameIsLast = (currentFrame + 1) == numFrames;
    if(frameIsLast)
    {
        emit errorOccurred(tr("Reached last frame"));
        return;
    }

    ++currentFrame;
    QImage frame = avs->getFrame(currentFrame);

    emit frameGrabbed(frame);
    qDebug() << "End NEXT_FRAME VFG " << ctr;
    ++ctr;
}

void vfg::VideoFrameGrabber::requestPreviousFrame()
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start PREV_FRAME VFG " << ctr;
    if(currentFrame == 0)
    {
        emit errorOccurred(tr("Reached first frame"));
        return;
    }

    --currentFrame;
    QImage frame = avs->getFrame(currentFrame);

    emit frameGrabbed(frame);
    qDebug() << "End PREV_FRAME VFG " << ctr;
    ++ctr;
}

QImage vfg::VideoFrameGrabber::getFrame(unsigned frameNum)
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start GET_FRAME VFG " << ctr;
    --frameNum;

    QImage frame = avs->getFrame(frameNum);
    qDebug() << "End GET_FRAME VFG Thread " << ctr;
    ++ctr;
    return frame;
}

unsigned vfg::VideoFrameGrabber::totalFrames() const
{
    return numFrames;
}
