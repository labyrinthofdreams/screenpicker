#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QSet>

// Forward declarations
class QPoint;
class QDragEvent;
class QDropEvent;
class QCloseEvent;

namespace vfg {
    class VideoFrameGrabber;
    class VideoFrameThumbnail;
    class ScriptEditor;
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

    void loadFromAvisynthScript(QString path);

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

    void on_clearThumbsButton_clicked();

    void on_thumbnailSizeSlider_sliderMoved(int position);

    void on_thumbnailSizeSlider_valueChanged(int value);

    void on_saveThumbnailsButton_clicked();

    void on_actionAvisynth_Script_Editor_triggered();

    void on_actionQuit_triggered();

    void on_actionOptions_triggered();

    void on_screenshotsSpinBox_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;

    vfg::VideoFrameGrabber* frameGrabber;

    vfg::ScriptEditor* scriptEditor;

    QSet<unsigned> framesToSave;

    unsigned lastRequestedFrame;

    void createAvisynthScriptFile();
    void createConfig();
    void resetState();

protected:
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);
    void closeEvent(QCloseEvent *ev);
};

#endif // MAINWINDOW_H
