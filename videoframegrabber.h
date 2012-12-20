#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <QObject>
#include <QImage>
#include <QString>

namespace vfg
{
    enum {
        FirstFrame = 1
    };

    class AbstractVideoSource;

    // Class declaration
    class VideoFrameGrabber : public QObject
    {
        Q_OBJECT
    private:
        vfg::AbstractVideoSource* avs;

        // Last frame - 1
        unsigned numFrames;

        // Between range 0 - (last frame - 1)
        unsigned currentFrame;
    public:
        explicit VideoFrameGrabber(vfg::AbstractVideoSource* avs, QObject *parent = 0);
        ~VideoFrameGrabber();
        // Load video file
        void load(QString filename);
        bool hasVideo() const;
        const vfg::AbstractVideoSource* getVideoSource() const;
        // Captures the exact frame between range 1 - n
        void requestFrame(unsigned frameNum);
        void requestNextFrame();
        void requestPreviousFrame();
        QImage getFrame(unsigned frameNum);
        // Returns last captured frame number + 1
        unsigned lastFrame() const;
        unsigned totalFrames() const;
    signals:
        // Fired when video has been loaded
        // Video properties are passed in the signal
        void videoReady();
        // Fired when frame is available
        void frameGrabbed(QImage frame);
        // Fired in the event that an error happens
        void errorOccurred(QString msg);
    public slots:
    };
}

#endif // VIDEOFRAMEGRABBER_H
