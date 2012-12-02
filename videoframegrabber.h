#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <QObject>
#include <QImage>
#include <QString>
#include "ffms.h"

namespace vfg
{
    // Convert FFMS_Frame to QImage
    QImage convertToQImage(const FFMS_Frame* frame);

    // Class declaration
    class VideoFrameGrabber : public QObject
    {
        Q_OBJECT
    private:
        bool com_inited;
        char errorBuffer[1024];
        FFMS_ErrorInfo errorInfo;

        FFMS_VideoSource *videoSource;     

        int pixelFormats[2];
        unsigned numFrames;
        unsigned currentFrame;
    public:
        explicit VideoFrameGrabber(QObject *parent = 0);
        ~VideoFrameGrabber();
        // Load video file
        void load(QString filename);
        // Return frame by frame number
        void grabFrame(unsigned frameNum);
        void grabNextFrame();
        void grabPreviousFrame();

        bool hasVideo() const;
        unsigned lastFrame() const;

        const FFMS_Frame* getFrame(unsigned frameNum);
    signals:
        // Fired when video has been loaded
        // Video properties are passed in the signal
        void videoReady(const FFMS_VideoProperties* videoProps);
        // Fired when frame is available
        void frameGrabbed(const FFMS_Frame* frame);
        // Fired in the event that an error happens
        void errorOccurred(QString msg);
    public slots:
    };
}

#endif // VIDEOFRAMEGRABBER_H
