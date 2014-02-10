#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QSharedPointer>
#include <QMutex>
#include <QPair>

// Forward declarations
namespace vfg {
    class AbstractVideoSource;
}

namespace vfg
{
    // TODO: This is not clever
    static const unsigned FirstFrame = 1;

    inline static bool validRange(const unsigned value, const int rangeMax)
    {
        return (value - vfg::FirstFrame) < rangeMax;
    }

    // Class declaration
    class VideoFrameGrabber : public QObject
    {
        Q_OBJECT
    private:
        QSharedPointer<vfg::AbstractVideoSource> avs;

        // Last frame - FirstFrame
        unsigned numFrames;

        // Between range 0 - (last frame - FirstFrame)
        unsigned currentFrame;

        mutable QMutex mutex;
    public:
        explicit VideoFrameGrabber(QObject *parent = 0);
        explicit VideoFrameGrabber(QSharedPointer<vfg::AbstractVideoSource> avs, QObject *parent = 0);
        ~VideoFrameGrabber();

        bool hasVideo() const;
        void setVideoSource(QSharedPointer<vfg::AbstractVideoSource> newAvs);

        // Returns last captured frame number + FirstFrame
        unsigned lastFrame() const;
        unsigned totalFrames() const;

        QImage getFrame(unsigned frameNum);
    public slots:
        // Load video file
        //void load(QString filename);
        void requestNextFrame();
        void requestPreviousFrame();
        // Captures the exact frame between range FirstFrame - totalFrames()
        void requestFrame(unsigned frameNum);
    signals:
        // Fired when video has been loaded
        // Video properties are passed in the signal
        void videoReady();
        // Fired when frame is available
        void frameGrabbed(QPair<int, QImage> frame);
        // Fired in the event that an error happens
        void errorOccurred(QString msg);
    public slots:
    };
}

#endif // VIDEOFRAMEGRABBER_H
