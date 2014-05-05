#include <QImage>
#include <QMutexLocker>
#include <QPair>
#include "videoframegrabber.h"
#include "videoframegenerator.h"

using vfg::core::VideoFrameGenerator;

VideoFrameGenerator::VideoFrameGenerator(vfg::core::VideoFrameGrabber *frameGrabber,
                                         QObject *parent) :
    QObject(parent),
    frameGrabber(frameGrabber),
    mutex()
{
}

void VideoFrameGenerator::start()
{
    QMutexLocker lock(&mutex);
    state = State::Running;
    lock.unlock();

    fetchNext();
}

void VideoFrameGenerator::fetchNext()
{
    QMutexLocker lock(&mutex);
    if(!frameGrabber || state != State::Running || frames.empty()) {
        return;
    }

    const auto current = frames.takeFirst();
    QImage frame = frameGrabber->getFrame(current);
    lock.unlock();

    emit frameReady(QPair<int, QImage>(current, frame));
}

void VideoFrameGenerator::pause()
{
    QMutexLocker lock(&mutex);
    if(state == State::Running) {
        state = State::Paused;
    }
}

void VideoFrameGenerator::resume()
{
    QMutexLocker lock(&mutex);
    if(state == State::Paused)
    {
        lock.unlock();
        start();
    }
}

void VideoFrameGenerator::stop()
{
    QMutexLocker lock(&mutex);

    state = State::Stopped;
    frames.clear();
}

bool VideoFrameGenerator::isRunning() const
{
    QMutexLocker lock(&mutex);
    return state == State::Running;
}

bool VideoFrameGenerator::isPaused() const
{
    QMutexLocker lock(&mutex);
    return state == State::Paused;
}

void VideoFrameGenerator::enqueue(const int frame)
{
    QMutexLocker lock(&mutex);
    if(!frames.contains(frame))
    {
        frames.append(frame);
    }
}

int VideoFrameGenerator::remaining() const
{
    QMutexLocker lock(&mutex);
    return frames.count();
}
