#ifndef VIDEOFRAMETHUMBNAIL_H
#define VIDEOFRAMETHUMBNAIL_H

#include <QWidget>
#include <QPixmap>

class QVBoxLayout;
class QLabel;

namespace vfg
{
    class VideoFrameThumbnail : public QWidget
    {
        Q_OBJECT
    public:
        explicit VideoFrameThumbnail(QWidget *parent = 0);
        void setThumbnail(QPixmap thumbnail);

    private:
        QVBoxLayout *layout;
        QLabel *pixmapLabel;

    protected:
        void keyPressEvent(QKeyEvent *event);

    signals:
        void selected(int frameId);

    public slots:

    };
}
#endif // VIDEOFRAMETHUMBNAIL_H
