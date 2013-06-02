#include "videoframegrabber.h"
#include "abstractvideosource.h"
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include <QPair>
#include <QApplication>

vfg::VideoFrameGrabber::VideoFrameGrabber(QObject *parent) :
    QObject(parent),
    avs(),
    numFrames(0),
    currentFrame(0),
    mutex()
{
}

vfg::VideoFrameGrabber::VideoFrameGrabber(QSharedPointer<vfg::AbstractVideoSource> avs,
                                          QObject *parent) :
    QObject(parent),
    avs(avs),
    numFrames(0),
    currentFrame(0),
    mutex()
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
    QMutexLocker lock(&mutex);
    avs = newAvs;
    numFrames = avs->getNumFrames();
    currentFrame = 0;
    lock.unlock();

    emit videoReady();
}

unsigned vfg::VideoFrameGrabber::lastFrame() const
{
    return currentFrame + 1;
}

bool vfg::VideoFrameGrabber::frameOutOfRange(unsigned frameNum) const
{
    return (frameNum + 1) == numFrames;
}

void vfg::VideoFrameGrabber::requestFrame(unsigned frameNum)
{
    QMutexLocker ml(&mutex);

    qDebug() << Q_FUNC_INFO << QThread::currentThreadId();
    // Because frame requests are between range 1 - n
    --frameNum;
    currentFrame = frameNum;

    QImage frame = avs->getFrame(frameNum);

    //ml.unlock();
    emit frameGrabbed(QPair<unsigned, QImage>(frameNum, frame));
}

void vfg::VideoFrameGrabber::requestNextFrame()
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start NEXT_FRAME VFG ";
    bool frameIsLast = (currentFrame + 1) == numFrames;
    if(frameIsLast)
    {
        emit errorOccurred(tr("Reached last frame"));
        return;
    }

    ++currentFrame;
    QImage frame = avs->getFrame(currentFrame);

    emit frameGrabbed(QPair<unsigned, QImage>(currentFrame, frame));
    qDebug() << "End NEXT_FRAME VFG ";
}

void vfg::VideoFrameGrabber::requestPreviousFrame()
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

    emit frameGrabbed(QPair<unsigned, QImage>(currentFrame, frame));
    qDebug() << "End PREV_FRAME VFG ";
}

QImage vfg::VideoFrameGrabber::getFrame(unsigned frameNum)
{
    QMutexLocker ml(&mutex);
    qDebug() << "Start GET_FRAME VFG ";
    --frameNum;

    if(frameOutOfRange(frameNum))
    {
        emit errorOccurred(tr("Out of range"));
        return QImage();
    }

    // TODO: current frame was used only for the request methods
    // check if it can break anything...
    //currentFrame = frameNum;
    QImage frame = avs->getFrame(frameNum);
    qDebug() << "End GET_FRAME VFG Thread ";
    return frame;
}

unsigned vfg::VideoFrameGrabber::totalFrames() const
{
    return numFrames;
}
