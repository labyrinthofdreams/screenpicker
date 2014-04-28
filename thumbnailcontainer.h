#ifndef THUMBNAILCONTAINER_H
#define THUMBNAILCONTAINER_H

#include <memory>
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

    int maxThumbnails;

    void removeFirst();
public:
    explicit ThumbnailContainer(QWidget *parent = 0);

    void addThumbnail(std::unique_ptr<vfg::ui::VideoFrameThumbnail> thumbnail);
    void clearThumbnails();
    void resizeThumbnails(int width);
    std::unique_ptr<vfg::ui::VideoFrameThumbnail> takeSelected();
    vfg::ui::VideoFrameThumbnail* selected();
    int numThumbnails() const;
    void setMaxThumbnails(int max);
    bool isFull() const;

protected:
    void mousePressEvent(QMouseEvent *ev);

signals:
    void thumbnailDoubleClicked(int frameNumber);
    void maximumChanged(int newMaximum);

public slots:

private slots:
    void handleThumbnailSelection(vfg::ui::VideoFrameThumbnail* thumbnail);

};

} // namespace ui
} // namespace vfg

#endif // THUMBNAILCONTAINER_H
