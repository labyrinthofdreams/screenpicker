#ifndef VFG_VIDEOFRAMEGENERATOR_H
#define VFG_VIDEOFRAMEGENERATOR_H

#include <QObject>

namespace vfg {
    class VideoFrameGrabber;
}

namespace vfg {

class VideoFrameGenerator : public QObject
{
    Q_OBJECT
public:
    explicit VideoFrameGenerator(vfg::VideoFrameGrabber *frameGrabber, QObject *parent = 0);
    
signals:
    
public slots:
    
private:
    vfg::VideoFrameGrabber *frameGrabber;
};

} // namespace vfg

#endif // VFG_VIDEOFRAMEGENERATOR_H
