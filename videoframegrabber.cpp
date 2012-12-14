#include "videoframegrabber.h"
#include "abstractvideosource.h"

vfg::VideoFrameGrabber::VideoFrameGrabber(vfg::AbstractVideoSource *avs,
                                          QObject *parent) :
    QObject(parent),
    avs(avs),
    numFrames(0),
    currentFrame(0)
{
}

vfg::VideoFrameGrabber::~VideoFrameGrabber()
{
    delete avs;
}

void vfg::VideoFrameGrabber::load(QString filename)
{
    try
    {
        avs->load(filename);
        numFrames = avs->getNumFrames();
        currentFrame = 0;
    }
    catch(std::exception& ex)
    {
        throw;
    }

    emit videoReady();
}

bool vfg::VideoFrameGrabber::hasVideo() const
{
    return (avs != NULL);
}

unsigned vfg::VideoFrameGrabber::lastFrame() const
{
    return currentFrame + 1;
}

void vfg::VideoFrameGrabber::requestFrame(unsigned frameNum)
{
    // Because frame requests are between range 1 - n
    --frameNum;

    QImage frame = avs->getFrame(frameNum);

    emit frameGrabbed(frame);
}

void vfg::VideoFrameGrabber::requestNextFrame()
{
    bool frameIsLast = (currentFrame + 1) == numFrames;
    if(frameIsLast)
    {
        emit errorOccurred(tr("Reached last frame"));
        return;
    }

    ++currentFrame;
    QImage frame = avs->getFrame(currentFrame);

    emit frameGrabbed(frame);
}

void vfg::VideoFrameGrabber::requestPreviousFrame()
{
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
