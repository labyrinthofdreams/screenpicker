#ifndef VFG_VIDEOFRAMEGENERATOR_H
#define VFG_VIDEOFRAMEGENERATOR_H

#include <QImage>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QPair>
#include <QWaitCondition>

// Forward declarations
namespace vfg {
    class VideoFrameGrabber;
}

namespace vfg {

class VideoFrameGenerator : public QObject
{
    Q_OBJECT
public:
    explicit VideoFrameGenerator(vfg::VideoFrameGrabber *frameGrabber, QObject *parent = 0);

    bool isRunning() const;
    bool isPaused() const;
    void enqueue(int frame);
    int remaining() const;
    void clear();
    
signals:
    void frameReady(QPair<int, QImage> frame);
    
public slots:
    void start();
    void pause();
    void resume();
    void stop();
    void fetchNext();
    
private:
    vfg::VideoFrameGrabber *frameGrabber;
    QList<int> frames;
    mutable QMutex mutex;
    QWaitCondition waitToContinue;
    bool halt;
    bool active;
    bool paused;
};

} // namespace vfg

#endif // VFG_VIDEOFRAMEGENERATOR_H
