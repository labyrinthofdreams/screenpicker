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
    waitToContinue(),
    halt(false),
    active(false),
    paused(false)
{
}

void VideoFrameGenerator::start()
{
    QMutexLocker lock(&mutex);
    paused = false;
    while(!frames.empty())
    {
        active = true;
        const unsigned current = frames.takeFirst();
        lock.unlock();
        if(!frameGrabber) {
            break;
        }
        QImage frame = frameGrabber->getFrame(current);
        emit frameReady(QPair<unsigned, QImage>(current, frame));
        lock.relock();
        // Without the wait condition all the frames may be processed
        // faster in the loop than they can be processed in the connected slot for
        // the signal frameReady (due to e.g. caching in frameGrabber)
        // causing potentially unexpected behavior
        waitToContinue.wait(&mutex);
        if(halt) {
            break;
        }
    }

    lock.relock();
    active = false;
    halt = false;
}

void VideoFrameGenerator::fetchNext()
{
    waitToContinue.wakeOne();
}

void VideoFrameGenerator::pause()
{
    QMutexLocker lock(&mutex);
    if(active) {
        halt = true;
        paused = true;
    }
}

void VideoFrameGenerator::resume()
{
    QMutexLocker lock(&mutex);
    if(!active)
    {
        halt = false;
        lock.unlock();
        start();
    }
}

void VideoFrameGenerator::stop()
{
    QMutexLocker lock(&mutex);
    if(active) {
        halt = true;
    }
    lock.unlock();
    clear();
}

bool VideoFrameGenerator::isRunning() const
{
    QMutexLocker lock(&mutex);
    return active;
}

bool VideoFrameGenerator::isPaused() const
{
    QMutexLocker lock(&mutex);
    return paused;
}

void VideoFrameGenerator::enqueue(const unsigned frame)
{
    QMutexLocker lock(&mutex);
    bool exists = frames.contains(frame);
    if(!exists)
    {
        frames.append(frame);
    }
}

unsigned VideoFrameGenerator::remaining() const
{
    QMutexLocker lock(&mutex);
    return frames.count();
}

void VideoFrameGenerator::clear()
{
    QMutexLocker lock(&mutex);
    frames.clear();
}

} // namespace vfg
