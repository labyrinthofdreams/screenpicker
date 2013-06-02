#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <QObject>
#include <QPair>
#include <QImage>
#include <QString>
#include <QSharedPointer>
#include <QMutex>

// TODO: Replace getVideoSource() return type with QWeakPointer

// Forward declarations
namespace vfg {
    class AbstractVideoSource;
}

namespace vfg
{
    static const unsigned FirstFrame = 1;

    // Class declaration
    class VideoFrameGrabber : public QObject
    {
        Q_OBJECT
    private:
        QSharedPointer<vfg::AbstractVideoSource> avs;

        // Last frame - 1
        unsigned numFrames;

        // Between range 0 - (last frame - 1)
        unsigned currentFrame;

        QMutex mutex;

        bool frameOutOfRange(unsigned frameNum) const;
    public:
        explicit VideoFrameGrabber(QObject *parent = 0);
        explicit VideoFrameGrabber(QSharedPointer<vfg::AbstractVideoSource> avs, QObject *parent = 0);
        ~VideoFrameGrabber();

        bool hasVideo() const;
        void setVideoSource(QSharedPointer<vfg::AbstractVideoSource> newAvs);

        // Returns last captured frame number + 1
        unsigned lastFrame() const;
        unsigned totalFrames() const;

        QImage getFrame(unsigned frameNum);
    public slots:
        // Load video file
        //void load(QString filename);
        void requestNextFrame();
        void requestPreviousFrame();
        // Captures the exact frame between range 1 - n
        void requestFrame(unsigned frameNum);
    signals:
        // Fired when video has been loaded
        // Video properties are passed in the signal
        void videoReady();
        // Fired when frame is available
        void frameGrabbed(QPair<unsigned, QImage> frame);
        // Fired in the event that an error happens
        void errorOccurred(QString msg);
    public slots:
    };
}

#endif // VIDEOFRAMEGRABBER_H
