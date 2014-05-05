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

    last = {};
    if(state == State::Running || !frameGrabber || frames.empty()) {
        return;
    }
    state = State::Running;

    while(true) {
        lock.relock();

        if(state != State::Running || frames.empty()) {
            break;
        }
        const int current = frames.takeFirst();

        lock.unlock();
        const QImage frame = frameGrabber->getFrame(current);

        // If paused, store the frame temporarily until
        // the generator is resumed and the frame is emitted
        // If it's stopped, discard it
        lock.relock();
        if(state == State::Paused) {
            last = qMakePair(current, frame);
            break;
        }
        else if(state == State::Stopped) {
            break;
        }
        lock.unlock();

        emit frameReady(qMakePair(current, frame));
    }

    if(state != State::Paused) {
        state = State::Stopped;
    }
}

void VideoFrameGenerator::pause()
{
    QMutexLocker lock(&mutex);
    if(state == State::Running && !frames.empty()) {
        state = State::Paused;
    }
}

void VideoFrameGenerator::resume()
{
    QMutexLocker lock(&mutex);
    if(state == State::Paused)
    {
        lock.unlock();

        // Emit last grabbed frame after pausing and re-emit it
        if(!last.second.isNull()) {
            emit frameReady(last);
        }
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
