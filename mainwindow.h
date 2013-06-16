#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QList>
#include <QScopedPointer>
#include <QMutex>
#include <QPair>

// Forward declarations
class QPoint;
class QDragEvent;
class QDropEvent;
class QCloseEvent;
class QImage;
class QThread;

namespace vfg {
    class VideoFrameGrabber;
    class VideoFrameThumbnail;
    class ScriptEditor;
    class VideoFrameGenerator;
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

    void frameReceived(QPair<unsigned, QImage> frame);
    void scriptEditorUpdated(QString path);
    void videoLoaded();
    void videoError(QString msg);
    void thumbnailDoubleClicked(unsigned frameNumber);
    void handleUnsavedMenu(const QPoint& pos);
    void handleSavedMenu(const QPoint& pos);

    void on_actionOpen_triggered();
    void on_nextButton_clicked();
    void on_previousButton_clicked();
    void on_generateButton_clicked();
    void on_originalResolutionCheckBox_toggled(bool checked);
    void on_seekSlider_valueChanged(int value);
    void on_seekSlider_sliderMoved(int position);
    void on_grabButton_clicked();
    void on_clearThumbsButton_clicked();
    void on_thumbnailSizeSlider_sliderMoved(int position);
    void on_thumbnailSizeSlider_valueChanged(int value);
    void on_saveThumbnailsButton_clicked();
    void on_actionAvisynth_Script_Editor_triggered();
    void on_actionQuit_triggered();
    void on_actionOptions_triggered();
    void on_screenshotsSpinBox_valueChanged(int arg1);
    void on_frameStepSpinBox_valueChanged(int arg1);
    void on_actionAbout_triggered();
    void on_saveSingleButton_clicked();
    void on_cbUnlimitedScreens_clicked(bool checked);
    void on_btnPauseGenerator_clicked();

    void on_btnStopGenerator_clicked();

    void on_actionOpen_DVD_triggered();

private:
    Ui::MainWindow *ui;

    QThread *frameGrabberThread;
    QThread *frameGeneratorThread;

    vfg::VideoFrameGenerator* frameGenerator;
    vfg::VideoFrameGrabber* frameGrabber;
    vfg::ScriptEditor* scriptEditor;

    QList<unsigned> framesToSave;

    unsigned lastRequestedFrame;

    void resetState();
    void loadFile(QString path);

protected:
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);
    void closeEvent(QCloseEvent *ev);
};

#endif // MAINWINDOW_H
