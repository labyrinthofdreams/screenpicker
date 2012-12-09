#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <QObject>
#include <QImage>
#include <QString>
#include "ffms.h"

namespace vfg
{
    enum {
        FirstFrame = 1
    };

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

        // Last frame - 1
        unsigned numFrames;

        // Between range 0 - (last frame - 1)
        unsigned currentFrame;

        // Captures the exact frame between range 0 - n
        const FFMS_Frame* internalGetFrame(unsigned frameNum);
    public:
        explicit VideoFrameGrabber(QObject *parent = 0);
        ~VideoFrameGrabber();
        // Load video file
        void load(QString filename);
        bool hasVideo() const;
        // Captures the exact frame between range 1 - n
        void requestFrame(unsigned frameNum);
        void requestNextFrame();
        void requestPreviousFrame();
        const FFMS_Frame* getFrame(unsigned frameNum);
        // Returns last captured frame number + 1
        unsigned lastFrame() const;
        unsigned totalFrames() const;
    signals:
        // Fired when video has been loaded
        // Video properties are passed in the signal
        void videoReady(const FFMS_VideoProperties* videoProps);
        // Fired when frame is available
        void frameGrabbed(QImage frame);
        // Fired in the event that an error happens
        void errorOccurred(QString msg);
    public slots:
    };
}

#endif // VIDEOFRAMEGRABBER_H
