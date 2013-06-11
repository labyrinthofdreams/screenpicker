#ifndef VFG_VIDEOFRAMEGENERATOR_H
#define VFG_VIDEOFRAMEGENERATOR_H

#include <QObject>
#include <QList>

// Forward declarations
namespace vfg {
    class VideoFrameGrabber;
}

class QImage;

namespace vfg {

class VideoFrameGenerator : public QObject
{
    Q_OBJECT
public:
    explicit VideoFrameGenerator(vfg::VideoFrameGrabber *frameGrabber, QObject *parent = 0);

    void start();
    void pause();
    void resume();
    void reset();
    bool isRunning() const;
    bool enqueue(unsigned frame);
    
signals:
    void frameReady(QImage frame);
    
public slots:
    
private:
    vfg::VideoFrameGrabber *frameGrabber;
    QList<unsigned> frames;
};

} // namespace vfg

#endif // VFG_VIDEOFRAMEGENERATOR_H
