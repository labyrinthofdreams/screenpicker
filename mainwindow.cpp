#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <utility>
#include <QtCore>
#include <QtMultimedia>
#include <QtMultimediaWidgets>
#include <QtWidgets>
#include "mainwindow.h"
#include "aboutwidget.hpp"
#include "avisynthvideosource.h"
#include "configdialog.h"
#include "downloadsdialog.hpp"
#include "dvdprocessor.h"
#include "extractorfactory.hpp"
#include "extractors/baseextractor.hpp"
#include "gifmakerwidget.hpp"
#include "jumptoframedialog.hpp"
#include "opendialog.hpp"
#include "ptrutil.hpp"
#include "savegriddialog.hpp"
#include "scripteditor.h"
#include "scriptparser.h"
#include "videoframegenerator.h"
#include "videoframegrabber.h"
#include "videoframethumbnail.h"
#include "videosettingswidget.h"
#include "x264encoderdialog.hpp"

Q_LOGGING_CATEGORY(MAINWINDOW, "mainwindow")

namespace {

QString QProcessErrorToString(const QProcess::ProcessError errorCode,
                              const QString& errorString) {
    QString error;

    switch(errorCode) {
    case QProcess::FailedToStart:
        error = QCoreApplication::tr("Process failed to start. Either the program is missing "
                    "or you have insufficient execution permissions.");
    case QProcess::Crashed:
        error = QCoreApplication::tr("Process crashed.");
    case QProcess::Timedout:
        error = QCoreApplication::tr("Process took too long to execute.");
    default:
        error = QCoreApplication::tr("Unspecified error (code %1): %2.")
                .arg(errorCode).arg(errorString);
    }

    return error;
}

/**
 * @brief Get MediaInfo video parameter
 * @param path Path to the video file
 * @param param Parameter to get
 * @return Parameter output from MediaInfo
 */
QString getMediaInfoParameter(const QString& path, const QString& param) {
    QProcess mediaInfo;
    mediaInfo.start(QDir::current().absoluteFilePath("mediainfo.exe"), QStringList() << path << QString("--output=%1").arg(param));
    if(!mediaInfo.waitForFinished()) {
        throw std::runtime_error("Mediainfo.exe failed to run. Try again.");
    }

    return mediaInfo.readAllStandardOutput();
}

} // namespace

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    ui.setupUi(this);

    setupInternal();

    ui.unsavedWidget->setMaxThumbnails(config.value("maxthumbnails").toInt());
    ui.unsavedProgressBar->setMaximum(config.value("maxthumbnails").toInt());

    ui.screenshotsSpinBox->setValue(config.value("numscreenshots").toInt());

    ui.frameStepSpinBox->setValue(config.value("framestep").toInt());

    ui.action25->setData("25");
    ui.action50->setData("50");
    ui.action100->setData("100");
    ui.action200->setData("200");
    ui.actionScaleToWindow->setData("scale");
    // Scale by default
    ui.actionScaleToWindow->setChecked(true);

    // Set default thumbnail sizes for the containers
    const auto thumbnailSize = ui.thumbnailSizeSlider->value();
    ui.unsavedWidget->resizeThumbnails(thumbnailSize);
    ui.savedWidget->resizeThumbnails(thumbnailSize);

    const auto logging = config.value("enable_logging", false).toBool();
    ui.actionDebugOn->setChecked(logging);
    ui.actionDebugOff->setChecked(!logging);

    // Open recent
    buildRecentMenu();

    //
    // Video zoom
    //
    auto videoZoomGroup = new QActionGroup(this);
    videoZoomGroup->addAction(ui.action25);
    videoZoomGroup->addAction(ui.action50);
    videoZoomGroup->addAction(ui.action100);
    videoZoomGroup->addAction(ui.action200);
    videoZoomGroup->addAction(ui.actionScaleToWindow);

    // When user changes zoom mode...
    connect(videoZoomGroup, &QActionGroup::triggered, [this](QAction *action) {
        static const QMap<QString, vfg::ZoomMode> modes {
            {"25", vfg::ZoomMode::Zoom_25},
            {"50", vfg::ZoomMode::Zoom_50},
            {"100", vfg::ZoomMode::Zoom_100},
            {"200", vfg::ZoomMode::Zoom_200},
            {"scale", vfg::ZoomMode::Zoom_Scale}
        };
        ui.videoPreviewWidget->setZoom(modes.value(action->data().toString()));
    });

    //
    // Show context menu on video preview widget
    //
    previewContext = ui.menuVideo;

    connect(ui.videoPreviewWidget, &vfg::ui::VideoPreviewWidget::customContextMenuRequested,
            [this](const QPoint &pos) {
        previewContext->exec(ui.videoPreviewWidget->mapToGlobal(pos));
    });

    // Update maximum value for progress bar
    connect(ui.unsavedWidget,      &vfg::ui::ThumbnailContainer::maximumChanged,
            ui.unsavedProgressBar, &QProgressBar::setMaximum);

    // Handle double-click on unsaved screenshot
    connect(ui.unsavedWidget,  &vfg::ui::ThumbnailContainer::thumbnailDoubleClicked,
            [this](const int frameNumber) {
        updateSeekSlider(frameNumber, SeekSlider::UpdateAll);
    });

    // Handle when unsaved screenshot container gets full
    connect(ui.unsavedWidget,  &vfg::ui::ThumbnailContainer::full, [this]() {
        if(config.value("removeoldestafterlimit").toBool()) {
            ui.unsavedWidget->removeFirst();
        }
        else {
            if(frameGenerator->remaining() > 0) {
                pauseFrameGenerator();
            }

            if(config.value("jumptolastonreachingmax").toBool()) {
                updateSeekSlider(config.value("last_received_frame").toInt(),
                                 SeekSlider::UpdateAll);
            }
        }
    });

    // When a thumbnail is added or removed from unsaved container update progress bar value
    connect(ui.unsavedWidget,      &vfg::ui::ThumbnailContainer::countChanged,
            ui.unsavedProgressBar, &QProgressBar::setValue);

    // Move thumbnail from unsaved to saved
    connect(ui.unsavedWidget,  &vfg::ui::ThumbnailContainer::requestMove, [this]() {
        ui.savedWidget->addThumbnail(ui.unsavedWidget->takeSelected());
    });

    // Move thumbnail from saved to unsaved
    connect(ui.savedWidget,    &vfg::ui::ThumbnailContainer::requestMove, [this]() {
        ui.unsavedWidget->addThumbnail(ui.savedWidget->takeSelected());
    });

    // Handle double-click on saved screenshot
    connect(ui.savedWidget,    &vfg::ui::ThumbnailContainer::thumbnailDoubleClicked,
            [this](const int frameNumber) {
        updateSeekSlider(frameNumber, SeekSlider::UpdateAll);
    });

    // Handle when GIF menu action is clicked
    connect(ui.menuCreateGIFImage, &QMenu::triggered, [this](QAction *action) {
        const auto objName = action->objectName();
        if(objName == "actionShowEditor") {
            activateGifMaker();
        }
        else if(objName == "actionSetStartFrame") {
            gifMaker->updateStartFrame(ui.seekSlider->value());
        }
        else if(objName == "actionSetEndFrame") {
            gifMaker->updateLastFrame(ui.seekSlider->value());
        }
    });
}

MainWindow::~MainWindow()
{
    qCDebug(MAINWINDOW) << "Destructor";

    config.remove("video");
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    qCDebug(MAINWINDOW) << "Close event";

    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();
    }

    const QMessageBox::StandardButton response =
            QMessageBox::question(this, tr("Quit?"), tr("Are you sure?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);
    if(response == QMessageBox::No) {
        qCDebug(MAINWINDOW) << "Close event cancelled";
        ev->ignore();
        return;
    }

    frameGeneratorThread->quit();
    frameGrabberThread->quit();

    scriptEditor.reset();
    videoSettingsWindow.reset();
    downloadsWindow.reset();
    gifMaker.reset();

    ev->accept();
}

void MainWindow::appendRecentMenu(const QString& item)
{
    auto recent = config.value("recent").toStringList();
    // Make sure (existing) item is at the top of the list
    const auto pos = recent.indexOf(item);
    if(pos != -1) {
        recent.removeAt(pos);
    }

    recent.prepend(item);
    if(recent.size() > 10) {
        recent.removeLast();
    }

    config.setValue("recent", recent);

    buildRecentMenu();
}

void MainWindow::removeRecentMenu(const QString &item)
{
    auto recent = config.value("recent").toStringList();
    // Remove item from list
    const auto pos = recent.indexOf(item);
    if(pos != -1) {
        recent.removeAt(pos);
    }

    config.setValue("recent", recent);

    buildRecentMenu();
}

void MainWindow::buildRecentMenu()
{
    auto menu = ui.actionRecent->menu();
    if(!menu) {
        menu = new QMenu;
        connect(menu,   &QMenu::triggered, [this](QAction *action) {
            const auto path = action->text();
            if(!QFile::exists(path)) {
                QMessageBox::critical(this, tr("File missing"), tr("Selected file is missing"));
                removeRecentMenu(path);
            }
            else {
                resetUi();
                loadFile(path);
            }
        });

        ui.actionRecent->setMenu(menu);
    }

    // Remove old menu items
    const auto actions = menu->actions();
    for(const auto& action : actions) {
        menu->removeAction(action);
    }

    // Add new menu items
    const auto recent = config.value("recent").toStringList();
    for(const auto& it : recent) {
        menu->addAction(it);
    }

    auto clearRecent = new QAction("Clear", menu);
    connect(clearRecent,    &QAction::triggered, [this]() {
        config.setValue("recent", {});
        buildRecentMenu();
    });

    menu->addSeparator();
    menu->addAction(clearRecent);
}

unsigned MainWindow::convertMsToFrame(const unsigned milliSecond) const
{
    assert(mediaPlayer);

    const auto duration = mediaPlayer->duration();
    const auto completed = static_cast<double>(milliSecond)/duration;
    return ui.totalFramesLabel->text().toInt() * completed;
}

void MainWindow::updateSeekSlider(const int value, const MainWindow::SeekSlider update)
{
    qCDebug(MAINWINDOW) << "Updating seek slider @" << value;

    ui.seekSlider->setValue(value);
    ui.currentFrameLabel->setText(QString::number(value));

    if(update == SeekSlider::UpdateAll) {
        auto mediaPlayer = getMediaPlayer();
        if(mediaPlayer->state() == QMediaPlayer::PlayingState) {
            mediaPlayer->setPosition(convertFrameToMs(value));
        }
        else {
            frameGrabber->requestFrame(value);
        }
    }
}

vfg::ui::DownloadsDialog *MainWindow::getDownloadsWindow()
{
    if(!downloadsWindow) {
        downloadsWindow = vfg::make_unique<vfg::ui::DownloadsDialog>();

        // When user requests to play file in downloads window, load it
        connect(downloadsWindow.get(),  &vfg::ui::DownloadsDialog::play, [this](const QString &path) {
            resetUi();
            loadFile(path);
        });
    }

    return downloadsWindow.get();
}

vfg::ui::OpenDialog *MainWindow::getOpenDialog()
{
    if(!openDialog) {
        openDialog = vfg::make_unique<vfg::ui::OpenDialog>();

        // When user wants to open URL via open dialog, add it to downloads
        connect(openDialog.get(), &vfg::ui::OpenDialog::openUrl, [this](const QNetworkRequest &req) {
            auto downloadsWindow = getDownloadsWindow();
            downloadsWindow->addDownload(req);
            downloadsWindow->show();
        });

        // When user wants to load DVD/BR files, process them
        connect(openDialog.get(),   &vfg::ui::OpenDialog::processDiscFiles,
                this,               &MainWindow::processDiscFiles);
    }

    return openDialog.get();
}

vfg::ui::VideoSettingsWidget *MainWindow::getVideoSettingsWindow()
{
    if(!videoSettingsWindow) {
        videoSettingsWindow = vfg::make_unique<vfg::ui::VideoSettingsWidget>();

        // When video settings are changed update video
        connect(videoSettingsWindow.get(),  &vfg::ui::VideoSettingsWidget::settingsChanged,
                [this]() {
            // TODO: Should this check be somewhere else?
            if(!videoSource->hasVideo()) {
                QMessageBox::warning(this, tr("No video"), tr("This operation requires a video"));
            }
            else {
                loadFile(config.value("last_opened").toString());
            }
        });

        // Draw crop border on video preview when crop changes in video settings
        connect(videoSettingsWindow.get(),  &vfg::ui::VideoSettingsWidget::cropChanged,
                ui.videoPreviewWidget,     &vfg::ui::VideoPreviewWidget::setCrop);
    }

    return videoSettingsWindow.get();
}

vfg::ui::ScriptEditor *MainWindow::getScriptEditor()
{
    if(!scriptEditor) {
        scriptEditor = vfg::make_unique<vfg::ui::ScriptEditor>();

        // Update video when script is changed
        connect(scriptEditor.get(), &vfg::ui::ScriptEditor::scriptUpdated, [this]() {
            loadFile(scriptEditor->path());
        });
    }

    return scriptEditor.get();
}

QMediaPlayer *MainWindow::getMediaPlayer()
{
    if(!mediaPlayer) {
        mediaPlayer = vfg::make_unique<QMediaPlayer>();
        mediaPlayer->setVideoOutput(ui.videoPreviewWidget->videoWidget.get());
        mediaPlayer->setVolume(ui.volumeSlider->value());

        // Adjust video volume level when changing the volume slider
        connect(ui.volumeSlider,   &QSlider::sliderMoved,
                mediaPlayer.get(),  &QMediaPlayer::setVolume);

        connect(mediaPlayer.get(), &QMediaPlayer::mediaStatusChanged,
                [this](const QMediaPlayer::MediaStatus status) {
            if(status == QMediaPlayer::BufferedMedia) {
                // After play() has been called
                // for the first time and video has buffered and starts playing...

                ui.videoPreviewWidget->showVideo();
                // Start playing from where the seek slider is
                mediaPlayer->setPosition(convertFrameToMs(ui.seekSlider->value()));
            }
        });

        // While video is playing the position of the video changes, so move the slider accordingly
        connect(mediaPlayer.get(),  &QMediaPlayer::positionChanged,
                [this](const qint64 position) {
            updateSeekSlider(convertMsToFrame(position), SeekSlider::UpdateText);
        });

        connect(mediaPlayer.get(),  &QMediaPlayer::stateChanged,
                [this](const QMediaPlayer::State state) {
            if(state == QMediaPlayer::PlayingState) {
                ui.buttonPlay->setIcon(QIcon(":/icon/pause2.png"));
                if(mediaPlayer->mediaStatus() == QMediaPlayer::BufferedMedia) {
                    // When calling play() for the first time the media player emits "playing" state
                    // while the video is still buffering.
                    // This check prevents the code below from running prematurely
                    // so it only runs when play is called second, third, etc. time

                    ui.videoPreviewWidget->showVideo();
                    // Start playing from where the seek slider is
                    mediaPlayer->setPosition(convertFrameToMs(ui.seekSlider->value()));
                }
            }
            else {
                ui.videoPreviewWidget->hideVideo();
                ui.buttonPlay->setIcon(QIcon(":/icon/play.png"));
                updateSeekSlider(convertMsToFrame(mediaPlayer->position()), SeekSlider::UpdateText);
                frameGrabber->requestFrame(convertMsToFrame(mediaPlayer->position()));
            }
        });
    }

    return mediaPlayer.get();
}

vfg::DvdProcessor *MainWindow::getDvdProcessor()
{
    if(!dvdProcessor) {
        dvdProcessor = vfg::make_unique<vfg::DvdProcessor>(config.value("dgindexexecpath").toString());

        // When DVD processor finishes, hide dialog window and load the processed file
        connect(dvdProcessor.get(), &vfg::DvdProcessor::finished, [this](const QString& filename) {
            auto dvdProgress = getDvdProgress();
            dvdProgress->accept();
            loadFile(filename);
        });

        // When DVD processor emits an error, hide dialog window and show error
        connect(dvdProcessor.get(), &vfg::DvdProcessor::error, [this](const QString &msg) {
            auto dvdProgress = getDvdProgress();
            dvdProgress->cancel();
            QMessageBox::warning(this, tr("Video error"), msg);
        });

        // When DVD processor emits an update value, update the dialog window progress
        connect(dvdProcessor.get(), &vfg::DvdProcessor::progressUpdate, [this](const int progress) {
            auto dvdProgress = getDvdProgress();
            dvdProgress->setValue(progress);
        });
    }

    return dvdProcessor.get();
}

QProgressDialog *MainWindow::getDvdProgress()
{
    if(!dvdProgress) {
        dvdProgress = vfg::make_unique<QProgressDialog>(tr("Processing DVD..."), tr("Abort"), 0, 100);

        // When user wants to cancel DVD loading...
        auto dvdProcessor = getDvdProcessor();
        connect(dvdProgress.get(),  &QProgressDialog::canceled,
                dvdProcessor, &vfg::DvdProcessor::handleAbortProcess);
    }

    return dvdProgress.get();
}

unsigned MainWindow::convertFrameToMs(const unsigned frameNumber) const
{
    assert(mediaPlayer);

    const auto totalFrames = ui.totalFramesLabel->text().toInt();
    const auto progress = static_cast<double>(frameNumber) / totalFrames;
    const auto videoTime = mediaPlayer->duration();
    return videoTime * progress;
}

void MainWindow::resetUi()
{
    qCDebug(MAINWINDOW) << "Setting up UI";

    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();
    }

    // Prevent loading old settings when loading new video
    config.remove("video");

    auto videoSettingsWindow = getVideoSettingsWindow();
    videoSettingsWindow->resetSettings();

    auto scriptEditor = getScriptEditor();
    scriptEditor->reset();

    ui.unsavedWidget->clearThumbnails();
    ui.savedWidget->clearThumbnails();

    ui.unsavedProgressBar->setValue(0);

    ui.totalFramesLabel->setText("0");

    ui.seekSlider->setValue(ui.seekSlider->minimum());

    // Select first tab
    ui.tabWidget->setCurrentIndex(0);

    // Disable buttons
    ui.seekSlider->setEnabled(false);
    ui.previousButton->setEnabled(false);
    ui.nextButton->setEnabled(false);
    ui.grabButton->setEnabled(false);
    ui.generateButton->setEnabled(false);
    ui.btnPauseGenerator->setEnabled(false);
    ui.btnStopGenerator->setEnabled(false);
    ui.generatorProgressBar->setValue(0);
    ui.generatorProgressBar->setTextVisible(false);
    ui.buttonPlay->setEnabled(false);

    ui.actionSave_as_PNG->setEnabled(false);
    ui.actionX264_Encoder->setEnabled(false);

    ui.actionJump_to->setEnabled(false);
}

void MainWindow::setupInternal()
{
    qCDebug(MAINWINDOW) << "Setting up internal state";

    // Set Avisynth as the default video source
    videoSource = std::make_shared<vfg::core::AvisynthVideoSource>();

    // Once the video source has loaded the video successfully
    connect(videoSource.get(),  &vfg::core::AbstractVideoSource::videoLoaded,
            this,               &MainWindow::videoLoaded);

    frameGrabber = std::make_shared<vfg::core::VideoFrameGrabber>(videoSource);

    // When frame grabber emits an error, display it to user
    connect(frameGrabber.get(), &vfg::core::VideoFrameGrabber::errorOccurred,
            [this](const QString &msg) {
        QMessageBox::warning(this, tr("Video error"), msg);
    });

    // Display frame emitted by frame grabber
    connect(frameGrabber.get(),     &vfg::core::VideoFrameGrabber::frameGrabbed,
            ui.videoPreviewWidget, static_cast<void(vfg::ui::VideoPreviewWidget::*)(int, const QImage&)>(&vfg::ui::VideoPreviewWidget::setFrame),
            Qt::QueuedConnection);

    frameGenerator = vfg::make_unique<vfg::core::VideoFrameGenerator>(frameGrabber);

    // When frame generator finishes, update UI, and conditionally go to last generated frame
    connect(frameGenerator.get(),   &vfg::core::VideoFrameGenerator::finished, this, [this]() {
        ui.btnPauseGenerator->setEnabled(false);
        ui.btnStopGenerator->setEnabled(false);
        ui.generateButton->setEnabled(true);

        // Jump to last generated frame
        if(config.value("jumptolastonfinish").toBool()) {
            updateSeekSlider(config.value("last_received_frame").toInt(), SeekSlider::UpdateAll);
        }
    });

    // Add frame emitted by frame generator to the unsaved screenshot widget
    // Note: this is necessary as context so that widget is created in GUI thread
    connect(frameGenerator.get(),   &vfg::core::VideoFrameGenerator::frameReady,
            this, [this](const int frameNum, const QImage& frame) {
        config.setValue("last_received_frame", frameNum);
        ui.unsavedWidget->addThumbnail(vfg::make_unique<vfg::ui::VideoFrameThumbnail>(frameNum, frame));
        ui.generatorProgressBar->setValue(ui.generatorProgressBar->value() + 1);
    });

    qCDebug(MAINWINDOW) << "Creating frame grabber thread";
    frameGrabberThread = vfg::make_unique<QThread>();

    qCDebug(MAINWINDOW) << "Starting frame grabber thread";
    frameGrabber->moveToThread(frameGrabberThread.get());
    frameGrabberThread->start();

    qCDebug(MAINWINDOW) << "Creating frame generator thread";
    frameGeneratorThread = vfg::make_unique<QThread>();

    qCDebug(MAINWINDOW) << "Starting frame generator thread";
    frameGenerator->moveToThread(frameGeneratorThread.get());
    frameGeneratorThread->start();
}

void MainWindow::loadFile(const QString& path)
{
    try
    {      
        if(frameGenerator->isRunning()) {
            pauseFrameGenerator();
        }

        auto mediaPlayer = getMediaPlayer();
        if(mediaPlayer->state() != QMediaPlayer::StoppedState) {
            mediaPlayer->stop();
        }

        const QFileInfo info {path};

        qCDebug(MAINWINDOW) << "Opening file" << info.absoluteFilePath();
        config.setValue("last_opened", info.absoluteFilePath());

        QMap<QString, QVariant> videoSettings;
        videoSettings.insert("resize", config.value("video/resize", QSize{}));
        videoSettings.insert("crop", config.value("video/crop", QRect{}));
        videoSettings.insert("deinterlace", config.value("video/deinterlace", false));
        videoSettings.insert("ivtc", config.value("video/ivtc", false));
        videoSettings.insert("avisynthpluginspath", config.value("avisynthpluginspath"));

        // When loading video for the first time we must
        // override the resize values since they're 0
        const QSize resize = videoSettings.value("resize").toSize();
        if(resize.width() < 1 && resize.height() < 1) {
            const auto width = getMediaInfoParameter(info.absoluteFilePath(), "Video;%Width%").toInt();
            const auto height = getMediaInfoParameter(info.absoluteFilePath(), "Video;%Height%").toInt();
            videoSettings.insert("resize", QSize{width, height});
        }

        const vfg::ScriptParser parser = videoSource->getParser(path);
        const QString parsedScript = parser.parse(videoSettings);

        auto scriptEditor = getScriptEditor();
        scriptEditor->setContent(parsedScript);
        scriptEditor->save();
        const QString saveTo = scriptEditor->path();

        // Attempt to load the (parsed) Avisynth script
        qCDebug(MAINWINDOW) << "Loading file" << saveTo;
        videoSource->load(saveTo);        
    }
    catch(const vfg::ScriptParserError& ex)
    {
        qCCritical(MAINWINDOW) << "Script template error:" << ex.what();
        QMessageBox::warning(this, tr("Script template error"), QString(ex.what()));
    }
    catch(const vfg::core::VideoSourceError& ex)
    {
        qCCritical(MAINWINDOW) << "Script processing error:" << ex.what();
        QMessageBox::warning(this, tr("Error while processing script"), QString(ex.what()));
    }
    catch(const std::exception& ex)
    {
        qCCritical(MAINWINDOW) << "Generic error:" << ex.what();
        QMessageBox::warning(this, tr("Error while loading file"),
                             QString(ex.what()));

        auto scriptEditor = getScriptEditor();
        scriptEditor->show();
        scriptEditor->setWindowState(Qt::WindowActive);
    }
}

void MainWindow::displayGifPreview(QString args, QString optArgs)
{
    qCDebug(MAINWINDOW) << "Displaying GIF preview";

    const auto imageMagickPath = config.value("imagemagickpath").toString();
    if(imageMagickPath.isEmpty()) {
        qCWarning(MAINWINDOW) << "ImageMagick path is not set";

        QMessageBox::critical(this, tr("Missing ImageMagick path"),
                              tr("Set path to ImageMagick and try again."));
        return;
    }

    const auto gifsiclePath = config.value("gifsiclepath").toString();
    if(!optArgs.isEmpty() && gifsiclePath.isEmpty()) {
        qCWarning(MAINWINDOW) << "Gifsicle path is not set";
        QMessageBox::critical(this, tr("Missing Gifsicle path"),
                              tr("Set path to Gifsicle and try again."));
        return;
    }

    QList<int> frames;
    const auto start_frame = config.value("gif/startframe").toInt();
    const auto end_frame = config.value("gif/endframe").toInt();
    const auto skip_frames = config.value("gif/skipframes", 0).toInt() + 1;
    //const auto delay = config.value("gif/delay", 4).toInt();
    QProgressDialog progress(tr("Generating frames"), tr("Cancel"), start_frame, end_frame + 2);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    QDir cacheDir(config.value("cachedirectory", "cache").toString());
    for(auto current = start_frame; current <= end_frame; current += skip_frames) {
        if(progress.wasCanceled()) {
            progress.setLabelText(tr("Cancelling"));
            // Remove saved images
            for(const auto frame : frames) {
                QFile::remove(cacheDir.absoluteFilePath(QString("%1.png")).arg(frame));
            }

            return;
        }
        progress.setValue(current);

        QImage frame = frameGrabber->getFrame(current);
        if(frame.isNull()) {
            continue;
        }

        // Save as uncompressed PNG images
        frame.save(cacheDir.absoluteFilePath(QString("%1.png").arg(current)), "PNG", 100);
        frames.append(current);

        QCoreApplication::processEvents();
    }

    qCDebug(MAINWINDOW) << "Extracted" << frames.size() << "frames";
    qCDebug(MAINWINDOW) << "Generating GIF";

    progress.setLabelText(tr("Generating GIF"));
    QCoreApplication::processEvents();

    QStringList newArgs;
    newArgs << args.split(" ") << cacheDir.absoluteFilePath("*.png")
            << cacheDir.absoluteFilePath("preview.gif");

    QProcess imageMagick;
    imageMagick.start(imageMagickPath, newArgs);
    const auto imTimeout = 1000 * config.value("imagemagicktimeout").toInt();
    if(!imageMagick.waitForFinished(imTimeout)) {
        const auto error = QProcessErrorToString(imageMagick.error(), imageMagick.errorString());
        qCCritical(MAINWINDOW) << "ImageMagick error:" << error;

        QMessageBox::critical(this, tr("ImageMagick error"), error);
    }

    // Remove saved images
    for(const auto frame : frames) {
        QFile::remove(cacheDir.absoluteFilePath(QString("%1.png").arg(frame)));
    }

    progress.setValue(end_frame + 1);

    if(!optArgs.isEmpty()) {
        qCDebug(MAINWINDOW) << "Optimizing GIF";

        progress.setLabelText(tr("Optimizing GIF"));
        QCoreApplication::processEvents();

        const auto curDir = QDir::current();
        QStringList newOptArgs;
        newOptArgs << "--batch" << optArgs.split(" ")
                   << cacheDir.absoluteFilePath("preview.gif");

        QProcess gifsicle;
        gifsicle.start(gifsiclePath, newOptArgs);
        const auto gifsicleTimeout = 1000 * config.value("gifsicletimeout").toInt();
        if(!gifsicle.waitForFinished(gifsicleTimeout)) {
            const auto error = QProcessErrorToString(gifsicle.error(), gifsicle.errorString());
            qCCritical(MAINWINDOW) << "Gifsicle error:" << error;

            QMessageBox::critical(this, tr("Gifsicle error"), error);
        }
    }

    progress.setValue(end_frame + 2);

    gifMaker->showPreview(cacheDir.absoluteFilePath("preview.gif"));
}

void MainWindow::on_actionOpen_triggered()
{
    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();
    }

    const auto filename = QFileDialog::getOpenFileName(this, tr("Open video"),
                                         config.value("last_opened").toString(),
                                         "All (*.*);;Avisynth (*.avs, *.avsi);;DGIndex (*.d2v)");
    if(filename.isEmpty()) {
        return;
    }

    resetUi();
    loadFile(filename);           
}

void MainWindow::on_actionOpen_DVD_triggered()
{
    auto openDialog = getOpenDialog();
    openDialog->setActiveTab(vfg::ui::OpenDialog::Tab::OpenDisc);
    openDialog->exec();
}

void MainWindow::videoLoaded()
{
    qCDebug(MAINWINDOW) << "Video loaded";

    setWindowTitle(config.value("last_opened").toString());
    appendRecentMenu(config.value("last_opened").toString());

    const auto resolution = videoSource->resolution();
    config.setValue("video/resolution", resolution);
    ui.labelVideoResolution->setText(QString{"[%1x%2]"}
                                     .arg(resolution.width()).arg(resolution.height()));
    auto videoSettingsWindow = getVideoSettingsWindow();
    videoSettingsWindow->refresh();

    auto mediaPlayer = getMediaPlayer();
    mediaPlayer->setMedia(QUrl::fromLocalFile(videoSource->fileName()));

    config.setValue("last_opened_script", videoSource->fileName());

    ui.menuCreateGIFImage->setEnabled(true);
    ui.actionSetEndFrame->setEnabled(false);
    ui.actionSetStartFrame->setEnabled(false);

    const int numFrames = frameGrabber->totalFrames() - 1;
    ui.totalFramesLabel->setText(QString::number(numFrames));

    ui.seekSlider->setEnabled(true);
    ui.seekSlider->setMaximum(numFrames);

    ui.previousButton->setEnabled(true);
    ui.nextButton->setEnabled(true);
    ui.grabButton->setEnabled(true);
    ui.generateButton->setEnabled(true);
    ui.buttonPlay->setEnabled(true);

    ui.actionSave_as_PNG->setEnabled(true);
    ui.actionX264_Encoder->setEnabled(true);
    ui.actionJump_to->setEnabled(true);

    frameGrabber->requestFrame(std::min(frameGrabber->lastFrame(),
                                        videoSource->getNumFrames() - 1));

    if(config.value("showscripteditor").toBool()) {
        auto scriptEditor = getScriptEditor();
        scriptEditor->show();
    }

    if(config.value("showvideosettings").toBool()) {
        auto videoSettingsWindow = getVideoSettingsWindow();
        videoSettingsWindow->show();
    }
}

void MainWindow::on_nextButton_clicked()
{
    qCDebug(MAINWINDOW) << "Clicked next button";

    frameGrabber->requestNextFrame();
    updateSeekSlider(frameGrabber->lastFrame(), SeekSlider::UpdateText);
}

void MainWindow::on_previousButton_clicked()
{
    qCDebug(MAINWINDOW) << "Clicked previous button";

    frameGrabber->requestPreviousFrame();
    updateSeekSlider(frameGrabber->lastFrame(), SeekSlider::UpdateText);
}

void MainWindow::on_seekSlider_sliderReleased()
{
    qCDebug(MAINWINDOW) << "Seek slider released @" << ui.seekSlider->sliderPosition();

    const auto frameNumber = ui.seekSlider->sliderPosition();
    ui.currentFrameLabel->setText(QString::number(frameNumber));

    auto mediaPlayer = getMediaPlayer();
    if(mediaPlayer->state() == QMediaPlayer::PlayingState) {
        mediaPlayer->setPosition(convertFrameToMs(frameNumber));
    }
    else {
        frameGrabber->requestFrame(frameNumber);
    }
}

void MainWindow::on_seekSlider_sliderMoved(const int position)
{
    ui.currentFrameLabel->setText(QString::number(position));
}

void MainWindow::on_generateButton_clicked()
{
    qCDebug(MAINWINDOW) << "Clicked generate button";

    if(frameGenerator->isRunning()) {
        qCWarning(MAINWINDOW) << "Frame generator is running";
        return;
    }

    if(frameGenerator->isPaused()) {
        qCDebug(MAINWINDOW) << "Restarting frame generator";
        frameGenerator->stop();
    }

    const bool pauseAfterLimit = config.value("pauseafterlimit").toBool();
    if(pauseAfterLimit && ui.unsavedWidget->isFull()) {
        // Can't start the generator if the unsaved widget container is full
        // and user has selected to pause after the container is full
        qCWarning(MAINWINDOW) << "Thumbnail container is full";
        QMessageBox::information(this, tr("Thumbnail container is full"),
                tr("Click 'Clear' or raise the max thumbnail limit."));
        return;
    }

    // Compute list of frame numbers to grab
    const auto selectedFrame = ui.seekSlider->value();
    const auto frameStep = ui.frameStepSpinBox->value();
    const auto numToGenerate = ui.screenshotsSpinBox->value();
    const auto totalFrames = frameGrabber->totalFrames();
    const auto unlimited = ui.cbUnlimitedScreens->isChecked();
    const auto lastFrame = unlimited ? totalFrames :
                           std::min(selectedFrame + frameStep * numToGenerate, totalFrames);
    QList<int> queue;
    for(auto currentFrame = selectedFrame; currentFrame <= lastFrame;
            currentFrame += frameStep) {
        queue.append(currentFrame);
    }

    frameGenerator->enqueue(queue);

    // Update generator widgets
    ui.generateButton->setEnabled(false);
    ui.btnPauseGenerator->setEnabled(true);
    ui.btnPauseGenerator->setText(tr("Pause"));
    ui.btnPauseGenerator->setIcon(QIcon(":/icon/pause.png"));
    ui.btnStopGenerator->setEnabled(true);
    ui.generatorProgressBar->setValue(0);
    ui.generatorProgressBar->setMaximum(frameGenerator->remaining());
    ui.generatorProgressBar->setTextVisible(true);

    QMetaObject::invokeMethod(frameGenerator.get(), "start", Qt::QueuedConnection);
}

void MainWindow::on_grabButton_clicked()
{
    qCDebug(MAINWINDOW) << "Clicked grab button";

    const int selectedFrame = ui.seekSlider->value();
    ui.savedWidget->addThumbnail(vfg::make_unique<vfg::ui::VideoFrameThumbnail>(selectedFrame,
                                                                                 frameGrabber->getFrame(selectedFrame)));
    statusBar()->showMessage(tr("Grabbed frame #%1").arg(selectedFrame), 3000);
}

void MainWindow::on_clearThumbsButton_clicked()
{
    ui.unsavedWidget->clearThumbnails();

    const bool resumeAfterClear = config.value("resumegeneratorafterclear", false).toBool();
    if(resumeAfterClear && frameGenerator->remaining() > 0 && frameGenerator->isPaused()) {
        resumeFrameGenerator();
    }
}

void MainWindow::on_thumbnailSizeSlider_sliderMoved(const int position)
{
    ui.unsavedWidget->resizeThumbnails(position);
    ui.savedWidget->resizeThumbnails(position);
}

void MainWindow::on_thumbnailSizeSlider_valueChanged(const int value)
{
    ui.unsavedWidget->resizeThumbnails(value);
    ui.savedWidget->resizeThumbnails(value);
}

void MainWindow::on_saveThumbnailsButton_clicked()
{
    // Save saved images to disk
    if(ui.savedWidget->isEmpty()) {
        QMessageBox::information(this, tr("Nothing to save"),
                                 tr("Add one or more screenshots to queue."));
        return;
    }

    // Pause frame generator
    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();
    }

    const auto lastSaveDirectory =
            QFileDialog::getExistingDirectory(this, tr("Select save directory"),
                                              config.value("last_save_dir", "/").toString());
    if(lastSaveDirectory.isEmpty()) {
        return;
    }

    config.setValue("last_save_dir", lastSaveDirectory);

    const auto numSaved = ui.savedWidget->numThumbnails();
    const QDir saveDir {lastSaveDirectory};

    QProgressDialog prog {"", "Cancel", 0, numSaved, this};
    prog.setWindowModality(Qt::WindowModal);
    prog.setCancelButton(0);
    prog.setMinimumDuration(0);
    for(const auto &widget : ui.savedWidget) {
        const auto current = prog.value();
        prog.setLabelText(tr("Saving image %1 of %2").arg(current).arg(numSaved));
        prog.setValue(current + 1);
        if(prog.wasCanceled()) {
            QMessageBox::warning(this, tr("Saving thumbnails aborted"),
                                 tr("Saved %1 of %2 thumbnails").arg(current).arg(numSaved));
            break;
        }

        const auto frameNumber = widget.frameNum();
        const auto filename = QString("%1.png").arg(QString::number(frameNumber));
        const auto savePath = saveDir.absoluteFilePath(filename);
        const auto frame = frameGrabber->getFrame(frameNumber);
        frame.save(savePath, "PNG");
    }

    prog.setValue(numSaved);
}

void MainWindow::on_actionAvisynth_Script_Editor_triggered()
{
    auto scriptEditor = getScriptEditor();
    scriptEditor->show();
    scriptEditor->setWindowState(Qt::WindowActive);
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *ev)
{
    qCDebug(MAINWINDOW) << "Drop event";

    ev->acceptProposedAction();

    resetUi();

    const QList<QUrl> urls = ev->mimeData()->urls();
    if(urls.length() == 1) {
        loadFile(urls.at(0).toLocalFile());
    }
    else {
        const auto isDvdOrBr = std::all_of(urls.cbegin(), urls.cend(), [](const QUrl &url) {
            return url.toLocalFile().endsWith("vob", Qt::CaseInsensitive) ||
                    url.toLocalFile().endsWith("m2ts", Qt::CaseInsensitive);
        });
        if(isDvdOrBr) {
            QStringList fileList;
            for(const QUrl &url : urls) {
                fileList.append(url.toLocalFile());
            }

            processDiscFiles(fileList);
        }
        else {
            QMessageBox::information(this, tr("Unsupported format"),
                                     tr("Currently multi-file drops only support "
                                        "VOB and M2TS files"));
        }
    }
}

void MainWindow::on_actionOptions_triggered()
{
    vfg::ConfigDialog configDialog;
    const auto saved = configDialog.exec();
    if(saved) {
        ui.unsavedWidget->setMaxThumbnails(config.value("maxthumbnails").toInt());

        auto dvdProcessor = getDvdProcessor();
        dvdProcessor->setProcessor(config.value("dgindexexecpath").toString());
    }
}

void MainWindow::on_screenshotsSpinBox_valueChanged(const int arg1)
{
    config.setValue("numscreenshots", arg1);
}

void MainWindow::on_frameStepSpinBox_valueChanged(const int arg1)
{
    config.setValue("framestep", arg1);
}

void MainWindow::on_actionAbout_triggered()
{
    vfg::ui::AboutWidget a;
    a.exec();
}

void MainWindow::on_cbUnlimitedScreens_clicked(const bool checked)
{
    ui.screenshotsSpinBox->setEnabled(!checked);
}

void MainWindow::on_btnPauseGenerator_clicked()
{
    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();

        // Jump to last generated frame if the option is selected
        const bool jumpAfterPaused = config.value("jumptolastonpause").toBool();
        if(jumpAfterPaused) {
            updateSeekSlider(config.value("last_received_frame").toInt(), SeekSlider::UpdateAll);
        }
    }
    else if(frameGenerator->isPaused()) {
        if(ui.unsavedWidget->isFull()) {
            QMessageBox::information(this, tr(""), tr("Can't resume generator while the container has reached max limit.\n"
                                                      "Click 'Clear' or raise the max thumbnail limit to continue."));
            return;
        }

        resumeFrameGenerator();
    }
}

void MainWindow::on_btnStopGenerator_clicked()
{
    qCDebug(MAINWINDOW) << "Stopping frame generator";

    frameGenerator->stop();

    ui.generateButton->setEnabled(true);
    ui.generatorProgressBar->setValue(0);
    ui.generatorProgressBar->setTextVisible(false);
    ui.btnPauseGenerator->setEnabled(false);
    ui.btnPauseGenerator->setText(tr("Pause"));
    ui.btnPauseGenerator->setIcon(QIcon(":/icon/pause.png"));
    ui.btnStopGenerator->setEnabled(false);

    // Jump to last generated frame if the option is selected
    const bool jumpAfterStopped = config.value("jumptolastonstop").toBool();
    if(jumpAfterStopped) {
        updateSeekSlider(config.value("last_received_frame").toInt(), SeekSlider::UpdateAll);
    }
}

void MainWindow::on_actionVideo_Settings_triggered()
{
    auto videoSettingsWindow = getVideoSettingsWindow();
    videoSettingsWindow->hide();
    videoSettingsWindow->show();
    videoSettingsWindow->setWindowState(Qt::WindowActive);
}

void MainWindow::activateGifMaker()
{
    if(previewContext == ui.menuCreateGIFImage) {
        return;
    }

    previewContext = ui.menuCreateGIFImage;

    ui.actionSetEndFrame->setEnabled(true);
    ui.actionSetStartFrame->setEnabled(true);

    if(!gifMaker) {
        gifMaker = vfg::make_unique<vfg::ui::GifMakerWidget>();
        connect(gifMaker.get(), &vfg::ui::GifMakerWidget::requestPreview,
                this,           &MainWindow::displayGifPreview);
    }

    const auto accepted = gifMaker->exec();
    if(!accepted) {
        previewContext = ui.menuVideo;

        ui.actionSetEndFrame->setEnabled(false);
        ui.actionSetStartFrame->setEnabled(false);
    }
}

void MainWindow::pauseFrameGenerator()
{
    qCDebug(MAINWINDOW) << "Pausing frame generator";
    if(!frameGenerator->isRunning()) {
        qCDebug(MAINWINDOW) << "Not running";

        return;
    }

    frameGenerator->pause();

    ui.btnPauseGenerator->setText(tr("Resume"));
    ui.btnPauseGenerator->setIcon(QIcon(":/icon/resume.png"));
    ui.generateButton->setEnabled(true);
}

void MainWindow::resumeFrameGenerator()
{
    qCDebug(MAINWINDOW) << "Resuming frame generator";
    if(!frameGenerator->isPaused()) {
        qCDebug(MAINWINDOW) << "Not paused";

        return;
    }

    QMetaObject::invokeMethod(frameGenerator.get(), "resume",
                              Qt::QueuedConnection);

    ui.btnPauseGenerator->setText(tr("Pause"));
    ui.btnPauseGenerator->setIcon(QIcon(":/icon/pause.png"));
    ui.generateButton->setEnabled(false);
}

void MainWindow::on_actionSave_as_PNG_triggered()
{
    if(!frameGrabber->hasVideo()) {
        QMessageBox::critical(this, tr("No video"), tr("This operation requires video"));
        return;
    }

    const auto selected = ui.seekSlider->value();
    const auto saveDir = QDir{config.value("last_save_dir", "/").toString()};
    const auto saveName = QString{"%1.png"}.arg(QString::number(selected));
    const auto defaultSavePath = saveDir.absoluteFilePath(saveName);
    const auto outFilename = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                                          defaultSavePath, tr("PNG (*.png)"));
    config.setValue("last_save_dir", QFileInfo{outFilename}.absoluteDir().absolutePath());
    const auto frame = frameGrabber->getFrame(selected);
    frame.save(outFilename);
}

void MainWindow::on_actionX264_Encoder_triggered()
{
    vfg::ui::x264EncoderDialog w;
    w.exec();
}

void MainWindow::on_actionDebugOn_triggered(bool checked)
{
    ui.actionDebugOff->setChecked(!checked);
    config.setValue("enable_logging", true);

    QMessageBox::information(this, tr("Restart"), tr("Please restart the application"));
}

void MainWindow::on_actionDebugOff_triggered(bool checked)
{
    ui.actionDebugOn->setChecked(!checked);
    config.setValue("enable_logging", false);

    QMessageBox::information(this, tr("Restart"), tr("Please restart the application"));
}

void MainWindow::on_buttonPlay_clicked()
{
    auto mediaPlayer = getMediaPlayer();
    if(mediaPlayer->state() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
    }
    else {
        mediaPlayer->play();
    }
}

void MainWindow::on_playbackSpeed_currentIndexChanged(const QString &arg1)
{
    static const QMap<QString, double> rates {{"25%", 0.25}, {"50%", 0.5},
                                              {"75%", 0.75}, {"100%", 1.0},
                                              {"125%", 1.25}, {"150%", 1.5},
                                              {"175%", 1.75}, {"200%", 2.0}};

    auto mediaPlayer = getMediaPlayer();
    mediaPlayer->setPlaybackRate(rates.value(arg1));
}

void MainWindow::on_actionOpen_URL_triggered()
{
    auto openDialog = getOpenDialog();
    openDialog->setActiveTab(vfg::ui::OpenDialog::Tab::OpenStream);
    openDialog->exec();
}

void MainWindow::on_actionDownloads_triggered()
{
    auto downloadsWindow = getDownloadsWindow();
    downloadsWindow->show();
}

void MainWindow::on_actionJump_to_triggered()
{
    vfg::ui::JumpToFrameDialog dlg;
    connect(&dlg, &vfg::ui::JumpToFrameDialog::jumpTo,
            [this](const int position, const vfg::ui::TimeFormat tf) {
        if(tf == vfg::ui::TimeFormat::Time) {
            updateSeekSlider(convertMsToFrame(1000 * position), SeekSlider::UpdateAll);
        }
        else {
            updateSeekSlider(position, SeekSlider::UpdateAll);
        }
    });
    dlg.exec();
}

void MainWindow::processDiscFiles(const QStringList& files)
{
    if(files.empty()) {
        return;
    }

    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();
    }

    const QFileInfo openedVobFile(files.first());
    config.setValue("last_opened_dvd", openedVobFile.absoluteDir().absolutePath());

    const QString dgIndexPath = config.value("dgindexexecpath").toString();
    if(!QFile::exists(dgIndexPath)) {
        QMessageBox::critical(this, tr("DGIndex invalid path"),
                              tr("Please set a valid path to DGIndex"));
        ui.actionOptions->trigger();
        return;
    }

    if(config.value("savedgindexfiles", false).toBool()) {
        qCDebug(MAINWINDOW) << "Saving DGIndex file";

        const QString out = QFileDialog::getSaveFileName(
                                0, tr("Select DGIndex project output path"),
                                openedVobFile.absoluteDir().absoluteFilePath("dgindex_project.d2v"),
                                tr("DGIndex project (*.d2v)"));
        if(out.isEmpty()) {
            return;
        }

        qCDebug(MAINWINDOW) << out;

        // Get path without suffix
        const QFileInfo outInfo(out);
        QString outputPath = outInfo.absoluteDir().absoluteFilePath(
                                    outInfo.completeBaseName());
        auto dvdProcessor = getDvdProcessor();
        dvdProcessor->setOutputPath(std::move(outputPath));
    }
    else {
        // Remove existing output file to prevent DGIndex from creating
        // lots of different .d2v files
        if(QFile::exists("dgindex_tmp.d2v")) {
            QFile::remove("dgindex_tmp.d2v");
        }

        auto dvdProcessor = getDvdProcessor();
        dvdProcessor->setOutputPath("dgindex_tmp");
    }

    // Reset all states back to zero
    resetUi();

    auto dvdProgress = getDvdProgress();
    dvdProgress->setValue(0);
    dvdProgress->setVisible(true);

    if(openedVobFile.suffix() == "m2ts") {
        dvdProgress->setLabelText(tr("Processing Blu-ray..."));
    }

    auto dvdProcessor = getDvdProcessor();
    dvdProcessor->process(files);
}

void MainWindow::on_saveGridButton_clicked()
{
    vfg::ui::SaveGridDialog dialog;

    for(const auto &widget : ui.savedWidget) {
        dialog.addPixmap(QPixmap::fromImage(frameGrabber->getFrame(widget.frameNum())));
    }

    dialog.exec();
}
