#ifndef THUMBNAILCONTAINER_H
#define THUMBNAILCONTAINER_H

#include <QWidget>

// Forward declarations
class FlowLayout;
class QMouseEvent;

namespace vfg {
namespace ui {
    class VideoFrameThumbnail;
}
}

namespace vfg {
namespace ui {

class ThumbnailContainer : public QWidget
{
    Q_OBJECT
private:
    FlowLayout* layout;

    vfg::ui::VideoFrameThumbnail* activeWidget;

    unsigned maxThumbnails;
public:
    explicit ThumbnailContainer(QWidget *parent = 0);

    void addThumbnail(vfg::ui::VideoFrameThumbnail* thumbnail);
    void clearThumbnails();
    void resizeThumbnails(const unsigned width);
    vfg::ui::VideoFrameThumbnail* takeSelected();
    vfg::ui::VideoFrameThumbnail* selected();
    int numThumbnails() const;
    void setMaxThumbnails(const unsigned max);
    bool isFull() const;

protected:
    void mousePressEvent(QMouseEvent *ev);

signals:
    void thumbnailDoubleClicked(unsigned frameNumber);
    void maximumChanged(int newMaximum);

public slots:

private slots:
    void handleThumbnailSelection(vfg::ui::VideoFrameThumbnail* thumbnail);

};

} // namespace ui
} // namespace vfg

#endif // THUMBNAILCONTAINER_H
