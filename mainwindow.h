#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QSettings>
#include <QtContainerFwd>
#include "ptrutil.hpp"

// Forward declarations
class QAction;
class QActionGroup;
class QCloseEvent;
class QDragEvent;
class QDropEvent;
class QMenu;
class QImage;
class QPoint;
class QProgressDialog;
class QString;
class QThread;

namespace vfg {
    class DvdProcessor;
namespace core {
    class AbstractVideoSource;
    class VideoFrameGenerator;
    class VideoFrameGrabber;
}
namespace ui {
    class GifMakerWidget;
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
     * @brief Handles a received frame from a generator
     * and updates widgets
     * @param frameNum Frame number
     * @param frame Received frame
     */
    void frameReceived(int frameNum, const QImage& frame);

    /**
     * @brief Triggered when a script in the editor is saved
     */
    void scriptEditorUpdated();

    /**
     * @brief Triggered when video settings are saved
     */
    void videoSettingsUpdated();

    /**
     * @brief Triggered after the dvd processor has finished
     * @param path Path to the processed file returned by the dvd processor
     */
    void dvdProcessorFinished(const QString& path);

    /**
     * @brief Triggered after a video source has finished loading
     */
    void videoLoaded();

    /**
     * @brief Handles various errors from video related sources
     * @param msg Error message
     */
    void videoError(const QString& msg);

    /**
     * @brief Triggered after a thumbnail has been clicked
     * @param frameNumber Frame number of the clicked thumbnail
     */
    void thumbnailDoubleClicked(int frameNumber);

    /**
     * @brief Triggered when a thumbnail is right-clicked in the unsaved tab
     * @param pos Position of the click event
     */
    void handleUnsavedMenu(const QPoint& pos);

    /**
     * @brief Triggered when a thumbnail is right-clicked in the saved tab
     * @param pos Position of the click event
     */
    void handleSavedMenu(const QPoint& pos);

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
     * @brief Displays a context menu on the video preview widget
     * @param pos Position of the right-click event
     */
    void contextMenuOnPreview(const QPoint& pos);

    /**
     * @brief Update DVD Progress Dialog
     * @param progress Current progress
     */
    void updateDvdProgressDialog(int progress);

    /**
     * @brief Generate and display GIF preview
     * @param args ImageMagick arguments
     * @param optArgs Optimization arguments
     */
    void displayGifPreview(QString args, QString optArgs);

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

    /**
     * @brief Handle GIF menu clicks
     */
    void gifContextMenuTriggered(QAction*);

    void on_actionSave_as_PNG_triggered();

    void on_actionX264_Encoder_triggered();

private:
    Ui::MainWindow* ui;

    std::unique_ptr<QThread> frameGrabberThread;
    std::unique_ptr<QThread> frameGeneratorThread;

    std::unique_ptr<QActionGroup> videoZoomGroup;

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

    //! Application wide configuration settings
    QSettings config;

    /**
     * @brief Resets widgets and member variables back to default
     */
    void resetState();

    void activateGifMaker();

protected:
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);
    void closeEvent(QCloseEvent *ev);
};

#endif // MAINWINDOW_H
