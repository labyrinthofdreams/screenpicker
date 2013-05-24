#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QQueue>
#include <QScopedPointer>
#include <QThread>
#include <QMutex>
#include <QPair>

// Forward declarations
class QPoint;
class QDragEvent;
class QDropEvent;
class QCloseEvent;
class QImage;

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

    void onFrameGrabbed(QPair<unsigned, QImage> frame);

    void on_actionOpen_triggered();

    void scriptEditorUpdated(QString path);

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

    void on_frameStepSpinBox_valueChanged(int arg1);

    void on_actionAbout_triggered();

    void on_saveSingleButton_clicked();

private:
    Ui::MainWindow *ui;

    QThread frameGrabberThread;
    QMutex frameReceivedMtx;

    QScopedPointer<vfg::VideoFrameGrabber> frameGrabber;
    QScopedPointer<vfg::ScriptEditor> scriptEditor;

    QQueue<unsigned> framesToSave;

    unsigned lastRequestedFrame;

    void createAvisynthScriptFile();
    void createConfig();
    void resetState();
    void loadFile(QString path);
    QString parseScript(QString filepath);
    void saveScript(QString path, QString script);

protected:
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);
    void closeEvent(QCloseEvent *ev);
};

#endif // MAINWINDOW_H
