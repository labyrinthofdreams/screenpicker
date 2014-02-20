#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QSettings>
#include <QString>

// Forward declarations
class QAction;
class QActionGroup;
class QCloseEvent;
class QDragEvent;
class QDropEvent;
class QImage;
class QPoint;
class QThread;

namespace vfg {
    class DvdProcessor;
namespace core {
    class AbstractVideoSource;
    class VideoFrameGenerator;
    class VideoFrameGrabber;
}
namespace ui {
    class ScriptEditor;
    class VideoFrameThumbnail;
    class VideoSettingsWidget;
}
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

    void frameReceived(QPair<int, QImage> frame);
    void scriptEditorUpdated();
    void videoSettingsUpdated();
    void dvdProcessorFinished(QString path);
    void videoLoaded();
    void videoError(QString msg);
    void thumbnailDoubleClicked(int frameNumber);
    void handleUnsavedMenu(const QPoint& pos);
    void handleSavedMenu(const QPoint& pos);
    void loadFile(QString path);

    void videoZoomChanged(QAction *action);
    void contextMenuOnPreview(const QPoint& pos);

    void on_actionOpen_triggered();
    void on_nextButton_clicked();
    void on_previousButton_clicked();
    void on_generateButton_clicked();
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

    void on_actionVideo_Settings_triggered();

private:
    Ui::MainWindow *ui;

    QThread *frameGrabberThread;
    QThread *frameGeneratorThread;

    std::shared_ptr<vfg::core::AbstractVideoSource> videoSource;
    vfg::core::VideoFrameGenerator* frameGenerator;
    vfg::core::VideoFrameGrabber* frameGrabber;
    vfg::ui::ScriptEditor* scriptEditor;
    vfg::DvdProcessor* dvdProcessor;
    vfg::ui::VideoSettingsWidget *videoSettingsWindow;

    QList<int> framesToSave;

    QString lastOpenedFile;
    QString lastSaveDirectory;

    QActionGroup *videoZoomGroup;

    QSettings config;

    // Keeps track of the last requested frame which is used to
    // return to that frame after reloading the script via editor
    int lastRequestedFrame;

    void resetState();

protected:
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);
    void closeEvent(QCloseEvent *ev);
};

#endif // MAINWINDOW_H
