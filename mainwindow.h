#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QSettings>
#include <QtContainerFwd>
#include "ptrutil.hpp"

class QAction;
class QCloseEvent;
class QDragEvent;
class QDropEvent;
class QMediaPlayer;
class QMenu;
class QNetworkRequest;
class QImage;
class QPlainTextEdit;
class QPoint;
class QProgressDialog;
class QString;
class QStringList;
class QThread;
class QUrl;

namespace vfg {
    class DvdProcessor;
namespace core {
    class AbstractVideoSource;
    class VideoFrameGenerator;
    class VideoFrameGrabber;
}
namespace extractor {
    class BaseExtractor;
}
namespace ui {
    class DownloadsDialog;
    class GifMakerWidget;
    class OpenDialog;
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
    /**
     * @brief Constructor
     * @param parent Owner of the object
     * @throws std::exception on critical failure
     */
    explicit MainWindow(QWidget *parent = 0);

    /**
     * @brief Destructor
     *
     * The destructor is required for the pointers
     * to have complete type
     */
    ~MainWindow();
    
private slots:
    /**
     * @brief Triggered when a script in the editor is saved
     */
    void scriptEditorUpdated();

    /**
     * @brief Triggered when video settings are saved
     */
    void videoSettingsUpdated();

    /**
     * @brief Triggered after a video source has finished loading
     */
    void videoLoaded();

    /**
     * @brief Load the given file with the current video source
     * @param path Path to the file to load
     */
    void loadFile(const QString& path);

    /**
     * @brief Triggered when the video zoom mode is changed
     * @param action The action that was performed
     */
    void videoZoomChanged(QAction *action);

    /**
     * @brief Generate and display GIF preview
     * @param args ImageMagick arguments
     * @param optArgs Optimization arguments
     */
    void displayGifPreview(QString args, QString optArgs);

    /**
     * @brief When screenshots tab signals full signal
     */
    void screenshotsFull();

    void processDiscFiles(const QStringList& files);

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
    void on_cbUnlimitedScreens_clicked(bool checked);
    void on_btnPauseGenerator_clicked();
    void on_btnStopGenerator_clicked();
    void on_actionOpen_DVD_triggered();
    void on_actionVideo_Settings_triggered();

    void on_actionSave_as_PNG_triggered();

    void on_actionX264_Encoder_triggered();

    void on_actionDebugOn_triggered(bool checked);

    void on_actionDebugOff_triggered(bool checked);

    void on_buttonPlay_clicked();

    void on_playbackSpeed_currentIndexChanged(const QString &arg1);

    void on_actionOpen_URL_triggered();

    void on_actionDownloads_triggered();

    void on_actionJump_to_triggered();

    void on_saveGridButton_clicked();

private:
    Ui::MainWindow* ui;

    std::unique_ptr<QThread> frameGrabberThread;
    std::unique_ptr<QThread> frameGeneratorThread;

    //! Display DVD loading progress in a dialog
    std::unique_ptr<QProgressDialog> dvdProgress;

    std::shared_ptr<vfg::core::AbstractVideoSource> videoSource;
    std::shared_ptr<vfg::core::VideoFrameGrabber> frameGrabber;
    std::unique_ptr<vfg::core::VideoFrameGenerator> frameGenerator;
    std::unique_ptr<vfg::ui::ScriptEditor> scriptEditor;
    std::unique_ptr<vfg::ui::VideoSettingsWidget> videoSettingsWindow;
    std::unique_ptr<vfg::DvdProcessor> dvdProcessor;

    //! Current context menu for preview widget
    util::observer_ptr<QMenu> previewContext;

    std::unique_ptr<vfg::ui::GifMakerWidget> gifMaker;

    //! Media player for video playback
    std::unique_ptr<QMediaPlayer> mediaPlayer;

    //! Downloads window
    std::unique_ptr<vfg::ui::DownloadsDialog> downloadsWindow;

    //! Open dialog
    std::unique_ptr<vfg::ui::OpenDialog> openDialog;

    //! Did user move the slider or was it updated by playing video?
    bool userMovedSlider {false};

    //! Application wide configuration settings
    QSettings config {"config.ini", QSettings::IniFormat};

    /**
     * @brief Setup widgets to default values
     */
    void resetUi();

    /**
     * @brief Setup internal states to default values
     */
    void setupInternal();

    void activateGifMaker();

    /**
     * @brief Pause frame generator and update UI
     * @pre Frame generator must be running
     */
    void pauseFrameGenerator();

    /**
     * @brief Resume frame generator and update UI
     * @pre Frame generator must be paused
     */
    void resumeFrameGenerator();

    /**
     * @brief Append new item to recent menu items
     * @param item Item to append
     */
    void appendRecentMenu(const QString& item);

    /**
     * @brief Remove item from recent menu items
     * @param item Item to remove
     */
    void removeRecentMenu(const QString& item);

    /**
     * @brief Build the recent menu item
     */
    void buildRecentMenu();

    /**
     * @brief Convert frame number to milliseconds
     * @param frameNumber Frame number to convert
     */
    unsigned convertFrameToMs(unsigned frameNumber) const;

    /**
     * @brief Convert milliseconds to frame number
     * @param milliSecond Milliseconds to convert
     */
    unsigned convertMsToFrame(unsigned milliSecond) const;

protected:
    void dragEnterEvent(QDragEnterEvent *ev) override;
    void dropEvent(QDropEvent *ev) override;
    void closeEvent(QCloseEvent *ev) override;
};

#endif // MAINWINDOW_H
