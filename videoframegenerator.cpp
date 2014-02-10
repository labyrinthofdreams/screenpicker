#include <QMutexLocker>
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
    active = true;
    while(!frames.empty())
    {
        const auto current = frames.takeFirst();
        lock.unlock();
        if(!frameGrabber) {
            break;
        }
        QImage frame = frameGrabber->getFrame(current);
        emit frameReady(QPair<int, QImage>(current, frame));
        // Without the wait condition all the frames may be processed
        // faster in the loop than they can be processed in the connected slot for
        // the signal frameReady (due to e.g. caching in frameGrabber)
        // causing potentially unexpected behavior
        lock.relock();
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
        active = false;
        waitToContinue.wakeOne();
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

void VideoFrameGenerator::enqueue(int frame)
{
    QMutexLocker lock(&mutex);
    bool exists = frames.contains(frame);
    if(!exists)
    {
        frames.append(frame);
    }
}

int VideoFrameGenerator::remaining() const
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
