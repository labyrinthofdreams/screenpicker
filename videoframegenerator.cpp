#include "videoframegrabber.h"
#include "videoframegenerator.h"

namespace vfg {

VideoFrameGenerator::VideoFrameGenerator(vfg::VideoFrameGrabber *frameGrabber,
                                         QObject *parent) :
    QObject(parent),
    frameGrabber(frameGrabber)
{
}

} // namespace vfg
