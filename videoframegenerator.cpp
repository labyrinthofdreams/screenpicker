#include <QMutexLocker>
#include <QListIterator>
#include <QImage>
#include <QPair>
#include "videoframegrabber.h"
#include "videoframegenerator.h"

namespace vfg {

VideoFrameGenerator::VideoFrameGenerator(vfg::VideoFrameGrabber *frameGrabber,
                                         QObject *parent) :
    QObject(parent),
    frameGrabber(frameGrabber),
    mutex(),
    halt(false),
    active(false)
{
}

void VideoFrameGenerator::start()
{
    QMutexLocker lock(&mutex);
    QListIterator<const unsigned> iter(frames);
    while(iter.hasNext())
    {
        active = true;
        const unsigned current = iter.next();
        lock.unlock();
        if(!frameGrabber) {
            break;
        }
        QImage frame = frameGrabber->getFrame(current);
        emit frameReady(QPair<unsigned, QImage>(current, frame));
        lock.relock();
        if(halt) {
            break;
        }
    }

    lock.relock();
    active = false;
    halt = false;
}

void VideoFrameGenerator::pause()
{
    QMutexLocker lock(&mutex);
    halt = true;
}

void VideoFrameGenerator::resume()
{
    QMutexLocker lock(&mutex);
    halt = false;
    lock.unlock();
    start();
}

bool VideoFrameGenerator::isRunning() const
{
    QMutexLocker lock(&mutex);
    return active;
}

void VideoFrameGenerator::enqueue(const unsigned frame)
{
    QMutexLocker lock(&mutex);
    bool exists = frames.contains(frame);
    if(!exists)
    {
        frames.prepend(frame);
    }
}

} // namespace vfg
