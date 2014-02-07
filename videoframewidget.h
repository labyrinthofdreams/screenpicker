#ifndef VIDEOFRAMEWIDGET_H
#define VIDEOFRAMEWIDGET_H

#include <QScrollArea>
#include <QImage>
#include <QPixmap>
#include <QPair>

// Forward declarations
class QVBoxLayout;
class QLabel;
class QResizeEvent;
class QSize;

namespace vfg
{
    /*
     * Implements a simple video frame widget for displaying images (frames)
     */
    class VideoFrameWidget : public QWidget
    {
        Q_OBJECT
    private:
        QVBoxLayout* layout;
        QLabel* frameLabel;
        QPixmap framePixmap;
        QPixmap original;
        bool fullsize;

        void updateFrameSize();
    public:
        explicit VideoFrameWidget(QWidget *parent = 0);

        void setFullsize(bool value);
        QSize getFrameSize() const;
    protected:
        void resizeEvent(QResizeEvent *event);

    signals:

    public slots:
        void setFrame(QImage img);
        void setFrame(QPair<unsigned, QImage> img);
    };
}

#endif // VIDEOFRAMEWIDGET_H
