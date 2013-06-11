#ifndef VFG_VIDEOFRAMEGENERATOR_H
#define VFG_VIDEOFRAMEGENERATOR_H

#include <QObject>

namespace vfg {

class VideoFrameGenerator : public QObject
{
    Q_OBJECT
public:
    explicit VideoFrameGenerator(QObject *parent = 0);
    
signals:
    
public slots:
    
};

} // namespace vfg

#endif // VFG_VIDEOFRAMEGENERATOR_H
