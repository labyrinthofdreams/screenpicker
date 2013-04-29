#include "videoframegrabber.h"
#include "abstractvideosource.h"
#include <QDebug>
#include <QThread>

vfg::VideoFrameGrabber::VideoFrameGrabber(QSharedPointer<vfg::AbstractVideoSource> avs,
                                          QObject *parent) :
    QObject(parent),
    avs(avs),
    numFrames(0),
    currentFrame(0)
{
}

vfg::VideoFrameGrabber::~VideoFrameGrabber()
{
    qDebug() << "VFG destructed from thread " << thread()->currentThreadId();
}

void vfg::VideoFrameGrabber::load(QString filename)
{
    try
    {
        avs->load(filename);
        numFrames = avs->getNumFrames();
        currentFrame = 0;

        emit videoReady();
    }
    catch(std::exception& ex)
    {
        emit errorOccurred(ex.what());
    }
}

bool vfg::VideoFrameGrabber::hasVideo() const
{
    return avs->hasVideo();
}

void vfg::VideoFrameGrabber::setVideoSource(QSharedPointer<vfg::AbstractVideoSource> newAvs)
{
    avs = newAvs;
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
    // Because frame requests are between range 1 - n
    --frameNum;
    currentFrame = frameNum;

    QImage frame = avs->getFrame(frameNum);

    emit frameGrabbed(frame);
}

void vfg::VideoFrameGrabber::requestNextFrame()
{
    qDebug() << "Start VFG Thread " << thread()->currentThreadId();
    bool frameIsLast = (currentFrame + 1) == numFrames;
    if(frameIsLast)
    {
        emit errorOccurred(tr("Reached last frame"));
        return;
    }

    ++currentFrame;
    QImage frame = avs->getFrame(currentFrame);

    emit frameGrabbed(frame);
    qDebug() << "End VFG Thread " << thread()->currentThreadId();
}

void vfg::VideoFrameGrabber::requestPreviousFrame()
{
    qDebug() << "Thread " << thread()->currentThreadId();
    if(currentFrame == 0)
    {
        emit errorOccurred(tr("Reached first frame"));
        return;
    }

    --currentFrame;
    QImage frame = avs->getFrame(currentFrame);

    emit frameGrabbed(frame);
}

QImage vfg::VideoFrameGrabber::getFrame(unsigned frameNum)
{
    --frameNum;

    QImage frame = avs->getFrame(frameNum);
    return frame;
}

unsigned vfg::VideoFrameGrabber::totalFrames() const
{
    return numFrames;
}
