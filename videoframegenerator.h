#ifndef VFG_VIDEOFRAMEGENERATOR_H
#define VFG_VIDEOFRAMEGENERATOR_H

#include <QObject>
#include <QList>
#include <QImage>
#include <QMutex>
#include <QPair>

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
    void enqueue(const unsigned frame);
    unsigned remaining() const;
    void clear();
    
signals:
    void frameReady(QPair<unsigned, QImage> frame);
    
public slots:
    void start();
    void pause();
    void resume();
    void stop();
    
private:
    vfg::VideoFrameGrabber *frameGrabber;
    QList<const unsigned> frames;
    mutable QMutex mutex;
    bool halt;
    bool active;
    bool paused;
};

} // namespace vfg

#endif // VFG_VIDEOFRAMEGENERATOR_H
