#include <QMutexLocker>
#include <QListIterator>
#include <QImage>
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
    QMutexLocker lock(&mutex);
    QListIterator<unsigned> iter(&frames);
    while(iter.hasNext())
    {
        const unsigned current = iter.next();
        lock.unlock();
        if(!frameGrabber) {
            return;
        }
        QImage frame = frameGrabber->getFrame(current);
        emit frameReady(frame);
        lock.relock();
    }
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
    QMutexLocker lock(&mutex);
    bool exists = frames.contains(frame);
    if(!exists)
    {
        frames.prepend(frame);
    }
}

} // namespace vfg
