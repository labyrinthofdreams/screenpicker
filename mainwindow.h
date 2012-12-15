#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>

// Forward declarations
class QPoint;

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

    void videoLoaded();

    void videoError(QString msg);

    void thumbnailDoubleClicked(vfg::VideoFrameThumbnail* thumbnail);

    void handleUnsavedMenu(const QPoint& pos);

    void handleSavedMenu(const QPoint& pos);

    void on_nextButton_clicked();

    void on_previousButton_clicked();

    void on_generateButton_clicked();

    void on_originalResolutionCheckBox_toggled(bool checked);
    void on_seekSlider_valueChanged(int value);

    void on_seekSlider_sliderMoved(int position);

    void on_grabButton_clicked();

    void on_actionSave_thumbnails_triggered();

private:
    Ui::MainWindow *ui;

    vfg::VideoFrameGrabber* frameGrabber;
    vfg::VideoFrameWidget* frameWidget;

    QMap<int, QImage> unsaved;
    QMap<int, QImage> saved;
};

#endif // MAINWINDOW_H
