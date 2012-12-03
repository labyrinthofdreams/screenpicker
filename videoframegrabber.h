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

        // Captures the exact frame number between range 0 - n
        const FFMS_Frame* internalGetFrame(unsigned frameNum);
    public:
        explicit VideoFrameGrabber(QObject *parent = 0);
        ~VideoFrameGrabber();
        // Load video file
        void load(QString filename);
        // Return frame by frame number between range 1 - n
        void requestFrame(unsigned frameNum);
        void requestNextFrame();
        void requestPreviousFrame();
        const FFMS_Frame* getFrame(unsigned frameNum);

        bool hasVideo() const;
        unsigned lastFrame() const;
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
