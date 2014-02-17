#ifndef VIDEOFRAMEGRABBER_H
#define VIDEOFRAMEGRABBER_H

#include <memory>
#include <QObject>
#include <QImage>
#include <QString>
#include <QMutex>
#include <QPair>

// Forward declarations
namespace vfg {
namespace internal {
    class AbstractVideoSource;
}
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
        std::shared_ptr<vfg::internal::AbstractVideoSource> avs;

        // Last frame - FirstFrame
        unsigned numFrames;

        // Between range 0 - (last frame - FirstFrame)
        unsigned currentFrame;

        mutable QMutex mutex;
    public:
        explicit VideoFrameGrabber(QObject *parent = 0);
        explicit VideoFrameGrabber(std::shared_ptr<vfg::internal::AbstractVideoSource> avs,
                                   QObject *parent = 0);
        ~VideoFrameGrabber();

        bool hasVideo() const;
        void setVideoSource(std::shared_ptr<vfg::internal::AbstractVideoSource> newAvs);

        // Returns last captured frame number + FirstFrame
        unsigned lastFrame() const;
        unsigned totalFrames();

        QImage getFrame(unsigned frameNum);
    public slots:
        // Load video file
        //void load(QString filename);
        void requestNextFrame();
        void requestPreviousFrame();
        // Captures the exact frame between range FirstFrame - totalFrames()
        void requestFrame(unsigned frameNum);

    private slots:
        void videoSourceUpdated();

    signals:
        // Fired when frame is available
        void frameGrabbed(QPair<int, QImage> frame);
        // Fired in the event that an error happens
        void errorOccurred(QString msg);
    public slots:
    };
}

#endif // VIDEOFRAMEGRABBER_H
