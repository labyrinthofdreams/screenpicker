#include "videoframegrabber.h"
#include "videoframegenerator.h"

namespace vfg {

VideoFrameGenerator::VideoFrameGenerator(vfg::VideoFrameGrabber *frameGrabber,
                                         QObject *parent) :
    QObject(parent),
    frameGrabber(frameGrabber)
{
}

void VideoFrameGenerator::start()
{

}

void VideoFrameGenerator::pause()
{

}

void VideoFrameGenerator::resume()
{

}

void VideoFrameGenerator::reset()
{

}

bool VideoFrameGenerator::isRunning() const
{

}

void VideoFrameGenerator::enqueue(unsigned frame)
{

}



} // namespace vfg
