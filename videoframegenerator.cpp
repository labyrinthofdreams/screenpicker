#include <QMutexLocker>
#include "videoframegrabber.h"
#include "videoframegenerator.h"

namespace vfg {

VideoFrameGenerator::VideoFrameGenerator(vfg::VideoFrameGrabber *frameGrabber,
                                         QObject *parent) :
    QObject(parent),
    frameGrabber(frameGrabber),
    mutex()
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
    return false;
}

void VideoFrameGenerator::enqueue(unsigned frame)
{

}

} // namespace vfg
