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
namespace core {
    class AbstractVideoSource;
}
}

namespace vfg
{
    // TODO: This is not clever
    static const int FirstFrame = 1;

    inline static bool validRange(const int value, const int rangeMax)
    {
        return (value - vfg::FirstFrame) < rangeMax;
    }

    // Class declaration
    class VideoFrameGrabber : public QObject
    {
        Q_OBJECT
    private:
        std::shared_ptr<vfg::core::AbstractVideoSource> avs;

        // Last frame - FirstFrame
        int numFrames;

        // Between range 0 - (last frame - FirstFrame)
        int currentFrame;

        mutable QMutex mutex;
    public:
        explicit VideoFrameGrabber(QObject *parent = 0);
        explicit VideoFrameGrabber(std::shared_ptr<vfg::core::AbstractVideoSource> avs,
                                   QObject *parent = 0);
        ~VideoFrameGrabber();

        bool hasVideo() const;
        void setVideoSource(std::shared_ptr<vfg::core::AbstractVideoSource> newAvs);

        // Returns last captured frame number + FirstFrame
        int lastFrame() const;
        int totalFrames();

        QImage getFrame(int frameNum);
    public slots:
        // Load video file
        //void load(QString filename);
        void requestNextFrame();
        void requestPreviousFrame();
        // Captures the exact frame between range FirstFrame - totalFrames()
        void requestFrame(int frameNum);

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
