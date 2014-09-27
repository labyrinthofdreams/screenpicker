#include <stdexcept>
#include <utility>
#include <QImage>
#include <QLoggingCategory>
#include <QMutexLocker>
#include "videoframegrabber.h"
#include "videoframegenerator.h"

Q_LOGGING_CATEGORY(GENERATOR, "videoframegenerator")

using vfg::core::VideoFrameGenerator;

VideoFrameGenerator::VideoFrameGenerator(std::shared_ptr<vfg::core::VideoFrameGrabber> newFrameGrabber,
                                         QObject *parent) :
    QObject(parent),
    frameGrabber(std::move(newFrameGrabber)),
    frames(),
    mutex(),
    state(State::Stopped)
{
    if(!frameGrabber) {
        qCCritical(GENERATOR) << "Invalid frame grabber passed to generator";

        throw std::runtime_error("Frame grabber must be a valid object");
    }
}

void VideoFrameGenerator::start()
{
    QMutexLocker lock(&mutex);

    if(state == State::Running || frames.empty()) {
        return;
    }
    state = State::Running;

    qCDebug(GENERATOR) << "Starting frame generator with" << frames.size() << "frames";

    while(true) {
        lock.relock();

        if(state != State::Running || frames.empty()) {
            break;
        }
        const int current = frames.first();

        lock.unlock();
        QImage frame = frameGrabber->getFrame(current);

        lock.relock();
        if(state == State::Paused || state == State::Stopped) {
            // If paused or stopped, discard the frame
            // and keep the frame for when it's resumed
            // If the generator is stopped the frame is already removed
            break;
        }

        frames.takeFirst();
        lock.unlock();

        emit frameReady(current, std::move(frame));
    }

    if(state != State::Paused) {
        state = State::Stopped;

        emit finished();
    }
}

void VideoFrameGenerator::pause()
{
    QMutexLocker lock(&mutex);
    if(state == State::Running && !frames.empty()) {
        qCDebug(GENERATOR) << "Pausing frame generator";

        state = State::Paused;
    }
}

void VideoFrameGenerator::resume()
{
    QMutexLocker lock(&mutex);
    if(state == State::Paused) {
        qCDebug(GENERATOR) << "Resuming frame generator";

        lock.unlock();

        start();
    }
}

void VideoFrameGenerator::stop()
{
    QMutexLocker lock(&mutex);

    qCDebug(GENERATOR) << "Stopping frame generator";

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

void VideoFrameGenerator::enqueue(const QList<int>& newFrames)
{
    for(const int frame : newFrames) {
        frames.append(frame);
    }

    qCDebug(GENERATOR) << "Enqueued" << newFrames.size() << "frames";
}

void VideoFrameGenerator::enqueue(const int frame)
{
    QMutexLocker lock(&mutex);

    qCDebug(GENERATOR) << "Enqueueing frame" << frame;

    if(!frameGrabber->isValidFrame(frame)) {
        qCCritical(GENERATOR) << "Invalid frame";

        return;
    }

    // Let duplicate frames be added
    frames.append(frame);
}

int VideoFrameGenerator::remaining() const
{
    QMutexLocker lock(&mutex);
    return frames.count();
}
