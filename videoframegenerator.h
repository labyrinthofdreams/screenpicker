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
    void enqueue(const unsigned frame);
    unsigned remaining() const;
    
signals:
    void frameReady(QPair<unsigned, QImage> frame);
    
public slots:
    void start();
    void pause();
    void resume();
    
private:
    vfg::VideoFrameGrabber *frameGrabber;
    QList<const unsigned> frames;
    mutable QMutex mutex;
    bool halt;
    bool active;
};

} // namespace vfg

#endif // VFG_VIDEOFRAMEGENERATOR_H
