#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>

// Forward declarations
class FlowLayout;

struct FFMS_VideoProperties;
struct FFMS_Frame;

namespace vfg {
    class VideoFrameGrabber;
    class VideoFrameWidget;
    class VideoFrameThumbnail;
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:

    void on_actionOpen_triggered();

    void videoLoaded(const FFMS_VideoProperties* videoProps);

    void videoError(QString msg);

    void on_nextButton_clicked();

    void on_previousButton_clicked();

    void on_generateButton_clicked();

    void on_originalResolutionCheckBox_toggled(bool checked);
    void on_seekSlider_valueChanged(int value);

    void on_seekSlider_sliderMoved(int position);

    void on_grabButton_clicked();

private:
    Ui::MainWindow *ui;

    FlowLayout *unsavedLayout;
    FlowLayout *savedLayout;

    vfg::VideoFrameGrabber* frameGrabber;
    vfg::VideoFrameWidget* frameWidget;

    QMap<int, vfg::VideoFrameThumbnail*> unsaved;
    QMap<int, QImage> saved;
};

#endif // MAINWINDOW_H
