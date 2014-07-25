#include <map>
#include <stdexcept>
#include <utility>
#include <QtCore>
#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutwidget.hpp"
#include "avisynthvideosource.h"
#include "configdialog.h"
#include "dvdprocessor.h"
#include "gifmakerwidget.hpp"
#include "ptrutil.hpp"
#include "scripteditor.h"
#include "scriptparser.h"
#include "videoframegenerator.h"
#include "videoframegrabber.h"
#include "videoframethumbnail.h"
#include "videosettingswidget.h"

/**
 * @brief Get video info via avinfo
 * @param path Path to video file
 * @return Video info as a map
 */
QMap<QString, QString> avinfoParseVideoHeader(const QString& path) {
    QMap<QString, QString> videoHeader;

    QProcess proc;
    proc.start(QDir::current().absoluteFilePath("avinfo.exe"), QStringList() << path << "--raw");
    // Wait for 3 seconds max
    if(!proc.waitForFinished(3000)) {
        throw std::runtime_error("avinfo.exe failed to run. Try again.");
    }

    const QList<QByteArray> output = proc.readAllStandardOutput().split('\n');
    for(const QString& row : output) {
        const QStringList kv = row.trimmed().split("=");
        if(kv.size() < 2) {
            break;
        }
        videoHeader.insert(kv.at(0), kv.at(1));
    }

    return videoHeader;
}

/**
 * @brief Get video resolution
 *
 * This function is necessary for some video containers that do not
 * return the correct resolution for a video via avisynth
 *
 * @param path Path to the video file
 * @return Resolution as a pair (width, height)
 */
QPair<int, int> getVideoResolution(const QString& path) {
    const QMap<QString, QString> videoHeader = avinfoParseVideoHeader(path);

    const int resizeWidth = videoHeader.value("v1.x", "0").toInt();
    const int resizeHeight = videoHeader.value("v1.y", "0").toInt();

    return qMakePair(resizeWidth, resizeHeight);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    frameGrabberThread(nullptr),
    frameGeneratorThread(nullptr),
    videoZoomGroup(nullptr),
    dvdProgress(nullptr),
    videoSource(nullptr),
    frameGrabber(nullptr),
    frameGenerator(nullptr),
    scriptEditor(nullptr),
    videoSettingsWindow(nullptr),
    dvdProcessor(nullptr),
    previewContext(nullptr),
    gifMaker(nullptr),
    config("config.ini", QSettings::IniFormat)
{
    ui->setupUi(this);

    try
    {
        dvdProgress = util::make_unique<QProgressDialog>(tr("Processing DVD..."),
                                                         tr("Abort"), 0, 100);

        // Set Avisynth as the default video source
        videoSource = std::make_shared<vfg::core::AvisynthVideoSource>();

        frameGrabber = std::make_shared<vfg::core::VideoFrameGrabber>(videoSource);

        frameGenerator = util::make_unique<vfg::core::VideoFrameGenerator>(frameGrabber);
        frameGeneratorThread = util::make_unique<QThread>();
        frameGenerator->moveToThread(frameGeneratorThread.get());
        frameGeneratorThread->start();

        frameGrabberThread = util::make_unique<QThread>();
        frameGrabber->moveToThread(frameGrabberThread.get());
        frameGrabberThread->start();

        scriptEditor = util::make_unique<vfg::ui::ScriptEditor>();

        videoSettingsWindow = util::make_unique<vfg::ui::VideoSettingsWidget>();

        QString dgIndexPath = config.value("dgindexexecpath").toString();
        dvdProcessor = util::make_unique<vfg::DvdProcessor>(dgIndexPath);

        const int maxThumbnails = config.value("maxthumbnails").toInt();
        ui->unsavedWidget->setMaxThumbnails(maxThumbnails);
        ui->unsavedProgressBar->setMaximum(maxThumbnails);

        const int numScreenshots = config.value("numscreenshots").toInt();
        ui->screenshotsSpinBox->setValue(numScreenshots);

        const int frameStep = config.value("framestep").toInt();
        ui->frameStepSpinBox->setValue(frameStep);

        videoZoomGroup = util::make_unique<QActionGroup>(nullptr);
        videoZoomGroup->addAction(ui->action25);
        videoZoomGroup->addAction(ui->action50);
        videoZoomGroup->addAction(ui->action100);
        videoZoomGroup->addAction(ui->action200);
        videoZoomGroup->addAction(ui->actionScaleToWindow);
        ui->action25->setData("25");
        ui->action50->setData("50");
        ui->action100->setData("100");
        ui->action200->setData("200");
        ui->actionScaleToWindow->setData("scale");
        // Scale by default
        ui->actionScaleToWindow->setChecked(true);
        ui->videoPreviewWidget->setZoom(vfg::ZoomMode::Zoom_Scale);

        // Set default thumbnail sizes for the containers
        const auto thumbnailSize = ui->thumbnailSizeSlider->value();
        ui->unsavedWidget->resizeThumbnails(thumbnailSize);
        ui->savedWidget->resizeThumbnails(thumbnailSize);

        previewContext = ui->menuVideo;
    }
    catch(std::exception& ex)
    {
        throw;
    }

    connect(videoZoomGroup.get(),   SIGNAL(triggered(QAction*)),
            this,                   SLOT(videoZoomChanged(QAction*)));

    connect(ui->videoPreviewWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this,                   SLOT(contextMenuOnPreview(QPoint)));

    connect(videoSettingsWindow.get(),  SIGNAL(settingsChanged()),
            this,                       SLOT(videoSettingsUpdated()));

    connect(videoSettingsWindow.get(),  SIGNAL(cropChanged(QRect)),
            ui->videoPreviewWidget,     SLOT(setCrop(QRect)));

    connect(videoSettingsWindow.get(),  SIGNAL(closed()),
            ui->videoPreviewWidget,     SLOT(resetCrop()));

    connect(scriptEditor.get(), SIGNAL(scriptUpdated()),
            this,               SLOT(scriptEditorUpdated()));

    connect(dvdProcessor.get(), SIGNAL(finished(QString)),
            this, SLOT(dvdProcessorFinished(QString)));

    connect(dvdProcessor.get(), SIGNAL(error(QString)),
            this,               SLOT(videoError(QString)));

    connect(dvdProcessor.get(), SIGNAL(progressUpdate(int)),
            this,               SLOT(updateDvdProgressDialog(int)));

    connect(dvdProgress.get(),  SIGNAL(canceled()),
            dvdProcessor.get(), SLOT(handleAbortProcess()));

    connect(videoSource.get(),  SIGNAL(videoLoaded()),
            this,               SLOT(videoLoaded()));

    connect(frameGrabber.get(), SIGNAL(errorOccurred(QString)),
            this,               SLOT(videoError(QString)),
            Qt::QueuedConnection);

    connect(frameGrabber.get(),     SIGNAL(frameGrabbed(int, QImage)),
            ui->videoPreviewWidget, SLOT(setFrame(int, QImage)),
            Qt::QueuedConnection);

    connect(frameGenerator.get(),   SIGNAL(frameReady(int, QImage)),
            this,                   SLOT(frameReceived(int, QImage)),
            Qt::QueuedConnection);

    connect(ui->unsavedWidget,      SIGNAL(maximumChanged(int)),
            ui->unsavedProgressBar, SLOT(setMaximum(int)));

    connect(ui->unsavedWidget,  SIGNAL(thumbnailDoubleClicked(int)),
            this,               SLOT(thumbnailDoubleClicked(int)));

    connect(ui->savedWidget,    SIGNAL(thumbnailDoubleClicked(int)),
            this,               SLOT(thumbnailDoubleClicked(int)));

    connect(ui->menuCreateGIFImage, SIGNAL(triggered(QAction*)),
            this,                   SLOT(gifContextMenuTriggered(QAction*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    // TODO: Change it to only ask if there's unsaved screenshots,
    // or if the generation is still running (in which case pause it)
    const QMessageBox::StandardButton response =
            QMessageBox::question(this, tr("Quit?"), tr("Are you sure?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);
    if(response == QMessageBox::Yes)
    {
        if(frameGeneratorThread->isRunning())
        {
            frameGenerator->stop();
            frameGeneratorThread->quit();
            frameGeneratorThread->wait();
        }

        if(frameGrabberThread->isRunning())
        {
            frameGrabberThread->quit();
            frameGrabberThread->wait();
        }

        // Close script editor if it's open
        scriptEditor->close();
        videoSettingsWindow->close();

        ev->accept();
    }
    else
    {
        ev->ignore();
    }
}

void MainWindow::frameReceived(const int frameNum, const QImage& frame)
{
    // TODO: Is a lock needed here?
    qDebug() << "FRAME_RECEIVED in thread" << qApp->thread()->currentThreadId() << frameNum;

    auto thumbnail = QPixmap::fromImage(frame).scaledToWidth(200, Qt::SmoothTransformation);
    auto thumb = util::make_unique<vfg::ui::VideoFrameThumbnail>(frameNum, std::move(thumbnail));

    connect(thumb.get(),    SIGNAL(customContextMenuRequested(QPoint)),
            this,           SLOT(handleUnsavedMenu(QPoint)));

    // Update widgets
    ui->unsavedWidget->addThumbnail(std::move(thumb));
    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());

    const int numGenerated = ui->generatorProgressBar->value() + 1;
    ui->generatorProgressBar->setValue(numGenerated);

    const bool generatorFinished = frameGenerator->remaining() == 0;
    if(generatorFinished)
    {
        // Generator has finished without explicit stopping
        ui->btnPauseGenerator->setEnabled(false);
        ui->btnStopGenerator->setEnabled(false);
        ui->generateButton->setEnabled(true);

        // Jump to last generated frame
        const bool jumpAfterFinished = config.value("jumptolastonfinish").toBool();
        if(jumpAfterFinished) {
            ui->seekSlider->setValue(frameNum);
        }
    }

    if(ui->unsavedWidget->isFull())
    {
        const bool removeOldestAfterMax = config.value("removeoldestafterlimit").toBool();
        if(removeOldestAfterMax) {
            // When unsaved screenshots container becomes full and the setting
            // "remove oldest after reaching max" is checked, let the generator generate
            return;
        }

        // ...If the user has NOT checked that option and chooses to pause instead
        // as pausing is the other action, then...
        frameGenerator->pause();

        ui->generateButton->setEnabled(true);
        ui->btnPauseGenerator->setText(tr("Resume"));
        ui->btnPauseGenerator->setIcon(QIcon(":/icon/resume.png"));

        // ...In case the user has checked they want to jump to last generated frame
        // after filling the container, then jump...
        const bool jumpToLastAfterReachingMax = config.value("jumptolastonreachingmax").toBool();
        if(jumpToLastAfterReachingMax)
        {
            ui->seekSlider->setValue(frameNum);
        }

        // ...and wait for the user to click Clear, Generate, open another file,
        // or raise the max screenshots limit
    }
}

void MainWindow::resetState()
{
    videoSettingsWindow->resetSettings();

    scriptEditor->reset();

    frameGrabber->setVideoSource(videoSource);
    frameGenerator->stop();

    ui->unsavedWidget->clearThumbnails();
    ui->savedWidget->clearThumbnails();

    ui->unsavedProgressBar->setValue(0);

    ui->currentFrameLabel->setText("0");
    ui->totalFramesLabel->setText("0");

    ui->seekSlider->setValue(ui->seekSlider->minimum());

    // Select first tab
    ui->tabWidget->setCurrentIndex(0);

    // Disable buttons
    ui->seekSlider->setEnabled(false);
    ui->previousButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->grabButton->setEnabled(false);
    ui->generateButton->setEnabled(false);
    ui->btnPauseGenerator->setEnabled(false);
    ui->btnStopGenerator->setEnabled(false);
    ui->generatorProgressBar->setValue(0);
    ui->generatorProgressBar->setTextVisible(false);
    ui->actionSave_as_PNG->setEnabled(false);
}

void MainWindow::loadFile(const QString& path)
{
    try
    {      
        const QFileInfo info(path);
        config.setValue("last_opened", info.absoluteFilePath());

        QMap<QString, QVariant> videoSettings = videoSettingsWindow->getSettings();
        videoSettings.insert("avisynthpluginspath", config.value("avisynthpluginspath"));

        // Only overwrite values in video settings if
        // they've not been set explicitly by the user
        const bool overrideWidth = (videoSettings.value("resizewidth") == 0);
        const bool overrideHeight = (videoSettings.value("resizeheight") == 0);
        const bool overrideSize = (overrideWidth || overrideHeight);
        if(overrideSize) {
            const QPair<int, int> res = getVideoResolution(path);

            if(overrideWidth) {
                const int newWidth = (res.first % 2 == 0) ? res.first : res.first + 1;
                videoSettings.insert("resizewidth", newWidth);
            }
            if(overrideHeight) {
                const int newHeight = (res.second % 2 == 0) ? res.second : res.second + 1;
                videoSettings.insert("resizeheight", newHeight);
            }
        }

        const vfg::ScriptParser parser = videoSource->getParser(path);
        const QString parsedScript = parser.parse(videoSettings);

        scriptEditor->setContent(parsedScript);
        scriptEditor->save();
        const QString saveTo = scriptEditor->path();

        // Attempt to load the (parsed) Avisynth script
        // TODO: Stop screenshot generation if that's happening...
        videoSource->load(saveTo);
    }
    catch(vfg::ScriptParserError& ex)
    {
        QMessageBox::warning(this, tr("Script template error"), QString(ex.what()));
    }
    catch(vfg::exception::VideoSourceError& ex)
    {
        QMessageBox::warning(this, tr("Error while processing script"), QString(ex.what()));
    }
    catch(vfg::exception::AvisynthError& ex)
    {
        QMessageBox::warning(this, tr("Error while processing script"), QString(ex.what()));
    }
    catch(std::exception& ex)
    {
        QMessageBox::warning(this, tr("Error while loading file"),
                             QString(ex.what()));

        scriptEditor->show();
        scriptEditor->setWindowState(Qt::WindowActive);
    }
}

void MainWindow::videoZoomChanged(QAction* action)
{
    static const std::map<QString, vfg::ZoomMode> modes {
        {"25", vfg::ZoomMode::Zoom_25},
        {"50", vfg::ZoomMode::Zoom_50},
        {"100", vfg::ZoomMode::Zoom_100},
        {"200", vfg::ZoomMode::Zoom_200},
        {"scale", vfg::ZoomMode::Zoom_Scale}
    };

    const QString mode = action->data().toString();
    ui->videoPreviewWidget->setZoom(modes.at(mode));
}

void MainWindow::contextMenuOnPreview(const QPoint &pos)
{
    previewContext->exec(ui->videoPreviewWidget->mapToGlobal(pos));
}

void MainWindow::displayGifPreview(QString args, QString optArgs)
{
    const auto imageMagickPath = config.value("imagemagickpath").toString();
    if(imageMagickPath.isEmpty()) {
        QMessageBox::critical(this, tr("Missing ImageMagick path"),
                              tr("Set path to ImageMagick and try again."));
    }

    const auto gifsiclePath = config.value("gifsiclepath").toString();
    if(!optArgs.isEmpty() && gifsiclePath.isEmpty()) {
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
    for(auto current = start_frame; current <= end_frame; current += skip_frames) {
        if(progress.wasCanceled()) {
            progress.setLabelText(tr("Cancelling"));
            // Remove saved images
            for(const auto frame : frames) {
                QFile::remove(QString("%1.png").arg(frame));
            }

            return;
        }
        progress.setValue(current);

        QImage frame = frameGrabber->getFrame(current);

        // Save as uncompressed PNG images
        frame.save(QString("%1.png").arg(current), "PNG", 100);
        frames.append(current);

        QCoreApplication::processEvents();
    }
    progress.setLabelText(tr("Generating GIF"));
    QCoreApplication::processEvents();

    QStringList newArgs;
    newArgs << args.split(" ") << "*.png" << "preview.gif";

    QProcess imageMagick;
    imageMagick.start(imageMagickPath, newArgs);
    const auto imTimeout = 1000 * config.value("imagemagicktimeout").toInt();
    if(!imageMagick.waitForFinished(imTimeout)) {
        QString error;
        switch(imageMagick.error()) {
        case QProcess::FailedToStart:
            error = tr("ImageMagick failed to start. Either it is missing "
                    "or you have insufficient permissions.");
        case QProcess::Crashed:
            error = tr("ImageMagick crashed. Please try again.");
        case QProcess::Timedout:
            error = tr("ImageMagick took too long to execute. Try resizing "
                    "the video or selecting fewer frames.");
        default:
            error = tr("Unknown error invoking ImageMagick (code %1): %2.")
                    .arg(imageMagick.error()).arg(imageMagick.errorString());
        }

        QMessageBox::critical(this, tr("Gifsicle error"), error);
    }

    // Remove saved images
    for(const auto frame : frames) {
        QFile::remove(QString("%1.png").arg(frame));
    }

    progress.setValue(end_frame + 1);

    if(!optArgs.isEmpty()) {
        progress.setLabelText(tr("Optimizing GIF"));
        QCoreApplication::processEvents();

        const auto curDir = QDir::current();
        QStringList newOptArgs;
        newOptArgs << "--batch" << optArgs.split(" ")
                   << curDir.absoluteFilePath("preview.gif");

        QProcess gifsicle;
        gifsicle.start(gifsiclePath, newOptArgs);
        const auto gifsicleTimeout = 1000 * config.value("gifsicletimeout").toInt();
        if(!gifsicle.waitForFinished(gifsicleTimeout)) {
            QString error;
            switch(gifsicle.error()) {
            case QProcess::FailedToStart:
                error = tr("Gifsicle failed to start. Either it is missing "
                        "or you have insufficient permissions.");
            case QProcess::Crashed:
                error = tr("Gifsicle crashed. Please try again.");
            case QProcess::Timedout:
                error = tr("Gifsicle took too long to execute. Try resizing "
                        "the video or selecting fewer frames.");
            default:
                error = tr("Unknown error invoking gifsicle (code %1): %2.")
                        .arg(gifsicle.error()).arg(gifsicle.errorString());
            }

            QMessageBox::critical(this, tr("Gifsicle error"), error);
        }
    }

    progress.setValue(end_frame + 2);

    gifMaker->showPreview("preview.gif");
}

void MainWindow::updateDvdProgressDialog(const int progress)
{
    dvdProgress->setValue(progress);
}

void MainWindow::on_actionOpen_triggered()
{
    if(frameGenerator->isRunning()) {
        frameGenerator->pause();

        ui->generateButton->setEnabled(true);
        ui->btnPauseGenerator->setText(tr("Resume"));
        ui->btnPauseGenerator->setIcon(QIcon(":/icon/resume.png"));
    }

    const QString lastOpened(config.value("last_opened", "").toString());

    const QString filename =
            QFileDialog::getOpenFileName(this, tr("Open video"), lastOpened,
                                         "All (*.*);;Avisynth (*.avs, *.avsi);;DGIndex (*.d2v)");
    if(filename.isEmpty()) {
        return;
    }

    // Reset all states back to zero
    resetState();

    loadFile(filename);           
}

void MainWindow::on_actionOpen_DVD_triggered()
{
    if(frameGenerator->isRunning()) {
        frameGenerator->pause();

        ui->generateButton->setEnabled(true);
        ui->btnPauseGenerator->setText(tr("Resume"));
        ui->btnPauseGenerator->setIcon(QIcon(":/icon/resume.png"));
    }

    const QString lastOpened(config.value("last_opened_dvd", "").toString());

    const QStringList vobFiles = QFileDialog::getOpenFileNames(this, tr("Select DVD VOB/Blu-ray M2TS files"),
                                                         lastOpened,
                                                         "DVD VOB (*.vob);;Blu-ray M2TS (*.m2ts)");
    if(vobFiles.empty()) {
        return;
    }

    const QFileInfo openedVobFile(vobFiles.first());
    config.setValue("last_opened_dvd", openedVobFile.absoluteDir().absolutePath());

    const QString dgIndexPath = config.value("dgindexexecpath").toString();
    if(!QFile::exists(dgIndexPath)) {
        QMessageBox::critical(this, tr("DGIndex invalid path"),
                              tr("Please set a valid path to DGIndex"));
        ui->actionOptions->trigger();
        return;
    }

    if(config.value("savedgindexfiles", false).toBool()) {
        const QString out = QFileDialog::getSaveFileName(
                                0, tr("Select DGIndex project output path"),
                                openedVobFile.absoluteDir().absoluteFilePath("dgindex_project.d2v"),
                                tr("DGIndex project (*.d2v)"));
        if(out.isEmpty()) {
            return;
        }

        // Get path without suffix
        const QFileInfo outInfo(out);
        QString outputPath = outInfo.absoluteDir().absoluteFilePath(
                                    outInfo.completeBaseName());
        dvdProcessor->setOutputPath(std::move(outputPath));
    }
    else {
        // Remove existing output file to prevent DGIndex from creating
        // lots of different .d2v files
        if(QFile::exists("dgindex_tmp.d2v")) {
            QFile::remove("dgindex_tmp.d2v");
        }

        dvdProcessor->setOutputPath("dgindex_tmp");
    }

    // Reset all states back to zero
    resetState();

    dvdProgress->setValue(0);
    dvdProgress->setVisible(true);

    if(openedVobFile.suffix() == "m2ts") {
        dvdProgress->setLabelText(tr("Processing Blu-ray..."));
    }

    dvdProcessor->process(vobFiles);
}

void MainWindow::dvdProcessorFinished(const QString& path)
{
    dvdProgress->setValue(100);

    loadFile(path);
}

void MainWindow::videoSettingsUpdated()
{    
    if(!frameGrabber->hasVideo())
    {
        QMessageBox::warning(this, tr("No video"), tr("This operation requires a video"));
        return;
    }

    loadFile(config.value("last_opened").toString());
}

void MainWindow::scriptEditorUpdated()
{
    const QString path = scriptEditor->path();
    loadFile(path);
}

void MainWindow::videoLoaded()
{
    setWindowTitle(config.value("last_opened").toString());

    const int numFrames = frameGrabber->totalFrames() - 1;

    const QSize resolution = frameGrabber->resolution();
    ui->labelVideoResolution->setText(QString("[%1x%2]").arg(resolution.width())
                                      .arg(resolution.height()));
    config.setValue("video/resolution", resolution);

    ui->menuCreateGIFImage->setEnabled(true);
    ui->actionSetEndFrame->setEnabled(false);
    ui->actionSetStartFrame->setEnabled(false);

    // lastRequestedFrame may be out of range when the script
    // is reloaded via the editor and when the script produces
    // video with fewer frames than the last requested frame
    const int lastRequestedFrame = frameGrabber->lastFrame();
    const bool invalidRange = !frameGrabber->isValidFrame(lastRequestedFrame);
    const int jumpToFrame = invalidRange ? 0 : lastRequestedFrame;

    // Update widgets
    ui->currentFrameLabel->setText(QString::number(jumpToFrame));
    ui->totalFramesLabel->setText(QString::number(numFrames));

    ui->seekSlider->setEnabled(true);
    ui->seekSlider->setMaximum(numFrames);
    ui->seekSlider->setValue(jumpToFrame);

    ui->previousButton->setEnabled(true);
    ui->nextButton->setEnabled(true);
    ui->grabButton->setEnabled(true);
    ui->generateButton->setEnabled(true);

    ui->actionSave_as_PNG->setEnabled(true);

    QMetaObject::invokeMethod(frameGrabber.get(), "requestFrame",
                              Qt::QueuedConnection, Q_ARG(int, jumpToFrame));

    // Load config
    const bool showEditor = config.value("showscripteditor").toBool();
    if(showEditor)
    {
        scriptEditor->hide();
        scriptEditor->show();
    }

    const bool showVideoSettings = config.value("showvideosettings").toBool();
    if(showVideoSettings)
    {
        videoSettingsWindow->hide();
        videoSettingsWindow->show();
    }
}

void MainWindow::videoError(const QString& msg)
{
    dvdProgress->cancel();
    QMessageBox::warning(this, tr("Video error"), msg);
}

void MainWindow::thumbnailDoubleClicked(const int frameNumber)
{    
    QMetaObject::invokeMethod(frameGrabber.get(), "requestFrame",
                              Qt::QueuedConnection, Q_ARG(int, frameNumber));

    ui->currentFrameLabel->setText(QString::number(frameNumber));
    ui->seekSlider->setValue(frameNumber);
}

void MainWindow::on_nextButton_clicked()
{
    frameGrabber->requestNextFrame();
    const int lastRequestedFrame = frameGrabber->lastFrame();

    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_previousButton_clicked()
{
    frameGrabber->requestPreviousFrame();
    const int lastRequestedFrame = frameGrabber->lastFrame();

    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_seekSlider_valueChanged(const int frameNumber)
{
    const int lastRequestedFrame = frameGrabber->lastFrame();
    if(lastRequestedFrame == frameNumber) {
        return;
    }

    frameGrabber->requestFrame(frameNumber);
    ui->currentFrameLabel->setText(QString::number(frameNumber));
}

void MainWindow::on_seekSlider_sliderMoved(const int position)
{
    ui->currentFrameLabel->setText(QString::number(position));
}

void MainWindow::on_generateButton_clicked()
{
    if(frameGenerator->isRunning()) {
        return;
    }

    const bool pauseAfterLimit = config.value("pauseafterlimit").toBool();
    if(pauseAfterLimit && ui->unsavedWidget->isFull()) {
        // Can't start the generator if the unsaved widget container is full
        // and user has selected to pause after the container is full
        QMessageBox::information(this, tr(""),
                tr("Thumbnail container has reached max limit.\n"
                   "Click 'Clear' or raise the max thumbnail limit."));
        return;
    }

    // Compute list of frame numbers to grab
    const int selected_frame = ui->seekSlider->value();
    const int frame_step = ui->frameStepSpinBox->value();
    const int num_generate = ui->screenshotsSpinBox->value();
    const int unlimited_screens = ui->cbUnlimitedScreens->isChecked();
    const int total_video_frames = frameGrabber->totalFrames();
    const int total_frame_range = frame_step * num_generate;
    const int last_frame = selected_frame + total_frame_range;

    if(frameGenerator->isPaused()) {
        frameGenerator->stop();
    }

    for(int current_frame = selected_frame; ; current_frame += frame_step)
    {
        if(!unlimited_screens) {
            const bool reached_last_frame = current_frame >= last_frame;
            // Only check for reaching last generated frame
            // IF generating limited number of screenshots
            if(reached_last_frame)
                break;
        }

        const bool reached_video_end = current_frame > total_video_frames;
        // Always check for reaching last frame of the video
        if(reached_video_end)
            break;

        frameGenerator->enqueue(current_frame);
    }

    // Update generator widgets
    ui->generateButton->setEnabled(false);
    ui->btnPauseGenerator->setEnabled(true);
    ui->btnPauseGenerator->setText(tr("Pause"));
    ui->btnPauseGenerator->setIcon(QIcon(":/icon/pause.png"));
    ui->btnStopGenerator->setEnabled(true);
    ui->generatorProgressBar->setValue(0);
    ui->generatorProgressBar->setMaximum(frameGenerator->remaining());
    ui->generatorProgressBar->setTextVisible(true);

    QMetaObject::invokeMethod(frameGenerator.get(), "start",
                              Qt::QueuedConnection);
}

void MainWindow::on_grabButton_clicked()
{
    const int selectedFrame = ui->seekSlider->value();
    QImage frame = frameGrabber->getFrame(selectedFrame);
    auto thumbnail = QPixmap::fromImage(frame).scaledToWidth(200, Qt::SmoothTransformation);
    auto thumb = util::make_unique<vfg::ui::VideoFrameThumbnail>(selectedFrame, std::move(thumbnail));

    connect(thumb.get(),    SIGNAL(customContextMenuRequested(QPoint)),
            this,           SLOT(handleSavedMenu(QPoint)));

    ui->savedWidget->addThumbnail(std::move(thumb));

    statusBar()->showMessage(tr("Grabbed frame #%1").arg(selectedFrame), 3000);
}

void MainWindow::handleUnsavedMenu(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu menu;
    QAction *saveAction = new QAction(tr("Enqueue"), this);
    saveAction->setData(1);
    menu.addAction(saveAction);

    QAction* selected = menu.exec(QCursor::pos());
    if(selected && selected->data().toInt() == 1)
    {
        // Move thumbnail from unsaved tab to saved tab
        auto thumb = ui->unsavedWidget->takeSelected();
        if(!thumb) {
            return;
        }

        disconnect(thumb.get(), SIGNAL(customContextMenuRequested(QPoint)),
                   this,        SLOT(handleUnsavedMenu(QPoint)));

        connect(thumb.get(),    SIGNAL(customContextMenuRequested(QPoint)),
                this,           SLOT(handleSavedMenu(QPoint)));

        ui->savedWidget->addThumbnail(std::move(thumb));
        ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());
    }
}

void MainWindow::handleSavedMenu(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu menu;
    QAction *unsaveAction = new QAction(tr("Remove from queue"), this);
    unsaveAction->setData(1);
    menu.addAction(unsaveAction);

    QAction* selected = menu.exec(QCursor::pos());
    if(selected && selected->data().toInt() == 1)
    {
        // Move thumbnail from saved tab to unsaved tab
        auto thumb = ui->savedWidget->takeSelected();
        if(!thumb) {
            return;
        }

        disconnect(thumb.get(), SIGNAL(customContextMenuRequested(QPoint)),
                   this,        SLOT(handleSavedMenu(QPoint)));

        connect(thumb.get(),    SIGNAL(customContextMenuRequested(QPoint)),
                this,           SLOT(handleUnsavedMenu(QPoint)));

        ui->unsavedWidget->addThumbnail(std::move(thumb));
        ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());
    }
}

void MainWindow::on_clearThumbsButton_clicked()
{
    ui->unsavedWidget->clearThumbnails();
    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());

    const bool resumeAfterClear = config.value("resumegeneratorafterclear", false).toBool();
    if(resumeAfterClear && frameGenerator->remaining() > 0
            && frameGenerator->isPaused()) {
        QMetaObject::invokeMethod(frameGenerator.get(), "resume", Qt::QueuedConnection);
    }
}

void MainWindow::on_thumbnailSizeSlider_sliderMoved(const int position)
{
    ui->unsavedWidget->resizeThumbnails(position);
    ui->savedWidget->resizeThumbnails(position);
}

void MainWindow::on_thumbnailSizeSlider_valueChanged(const int value)
{
    ui->unsavedWidget->resizeThumbnails(value);
    ui->savedWidget->resizeThumbnails(value);
}

void MainWindow::on_saveThumbnailsButton_clicked()
{
    // Save saved images to disk
    if(ui->savedWidget->isEmpty())
    {
        QMessageBox::information(this, tr("Save screenshots"),
                                 tr("Nothing to save. Add one or more screenshots to save."));
        return;
    }

    // Pause frame generator
    if(frameGenerator->isRunning()) {
        frameGenerator->pause();
    }

    QString lastSaveDirectory = config.value("last_save_dir", "/").toString();
    lastSaveDirectory = QFileDialog::getExistingDirectory(
                            this, tr("Select save directory"),
                            lastSaveDirectory);
    if(lastSaveDirectory.isEmpty()) {
        return;
    }

    config.setValue("last_save_dir", lastSaveDirectory);

    int resizeWidth = 0;
    const QMessageBox::StandardButton clicked =
            QMessageBox::question(this, tr("Resize thumbnails"),
                                  tr("Do you want to resize thumbnails?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);

    if(clicked == QMessageBox::Yes) {
        resizeWidth = QInputDialog::getInt(this, tr("Resize thumbnails"), tr("Resize to width:"));
    }

    const std::size_t numSaved = ui->savedWidget->numThumbnails();
    QProgressDialog prog("", "Cancel", 0, numSaved, this);
    prog.setWindowModality(Qt::WindowModal);
    prog.setCancelButton(0);
    prog.setMinimumDuration(0);
    const QDir saveDir(lastSaveDirectory);
    for(std::size_t idx = 0; idx < numSaved; ++idx) {
        const auto widget = ui->savedWidget->at(idx);
        if(!widget) {
            break;
        }
        const int frameNumber = widget->frameNum();
        const int current = prog.value();
        prog.setLabelText(tr("Saving image %1 of %2").arg(current).arg(numSaved));
        prog.setValue(current + 1);
        if(prog.wasCanceled()) {
            QMessageBox::warning(this, tr("Saving thumbnails aborted"),
                                 tr("Saved %1 of %2 thumbnails").arg(current).arg(numSaved));
            break;
        }

        const QString filename = QString("%1.png").arg(QString::number(frameNumber));
        const QString savePath = saveDir.absoluteFilePath(filename);

        // Get current frame
        QImage frame = frameGrabber->getFrame(frameNumber);
        if(resizeWidth > 0) {
            frame = frame.scaledToWidth(resizeWidth, Qt::SmoothTransformation);
        }
        frame.save(savePath, "PNG");
    }
    prog.setValue(numSaved);
}

void MainWindow::on_actionAvisynth_Script_Editor_triggered()
{
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
    const QList<QUrl> urls = ev->mimeData()->urls();
    if(urls.length() > 1) {
        ev->ignore();
        QMessageBox::information(this, tr("Drop event"),
                                 tr("You can drop only one file"));
        return;
    }

    ev->acceptProposedAction();

    const QString filename = urls.at(0).toLocalFile();

    // Reset all states back to zero
    resetState();

    loadFile(filename);
}

void MainWindow::on_actionOptions_triggered()
{
    vfg::ConfigDialog configDialog;
    const int saved = configDialog.exec();

    if(saved == QDialog::Accepted)
    {
        const int newMaxThumbnails = config.value("maxthumbnails").toInt();
        ui->unsavedWidget->setMaxThumbnails(newMaxThumbnails);

        const bool containerHasRoom = ui->unsavedWidget->isFull();
        const bool generatorHasQueue = (frameGenerator->remaining() > 0);
        const bool generatorWaiting = frameGenerator->isPaused();
        const bool continueGenerator = (containerHasRoom && generatorHasQueue && generatorWaiting);

        if(continueGenerator)
        {
            frameGenerator->resume();
        }

        dvdProcessor->setProcessor(config.value("dgindexexecpath").toString());
    }
}

void MainWindow::on_screenshotsSpinBox_valueChanged(const int arg1)
{
    config.setValue("numscreenshots", arg1);

    ui->screenshotsSpinBox->setValue(arg1);
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
    ui->screenshotsSpinBox->setEnabled(!checked);
}

void MainWindow::on_btnPauseGenerator_clicked()
{
    if(frameGenerator->isRunning())
    {
        // Pause
        frameGenerator->pause();

        ui->btnPauseGenerator->setText(tr("Resume"));
        ui->btnPauseGenerator->setIcon(QIcon(":/icon/resume.png"));
        ui->generateButton->setEnabled(true);

        // Jump to last generated frame if the option is selected
        const bool jumpAfterPaused = config.value("jumptolastonpause").toBool();
        if(jumpAfterPaused) {
            const int lastIdx = ui->unsavedWidget->numThumbnails() - 1;
            const auto widget = ui->unsavedWidget->at(lastIdx);
            if(widget) {
                ui->seekSlider->setValue(widget->frameNum());
            }
        }
    }
    else if(frameGenerator->isPaused())
    {
        // Resume
        if(ui->unsavedWidget->isFull()) {
            QMessageBox::information(this, tr(""), tr("Can't resume generator while the container has reached max limit.\n"
                                                      "Click 'Clear' or raise the max thumbnail limit to continue."));
            return;
        }

        ui->btnPauseGenerator->setText(tr("Pause"));
        ui->btnPauseGenerator->setIcon(QIcon(":/icon/pause.png"));
        ui->generateButton->setEnabled(false);

        QMetaObject::invokeMethod(frameGenerator.get(), "resume",
                                  Qt::QueuedConnection);
    }
}

void MainWindow::on_btnStopGenerator_clicked()
{
    frameGenerator->stop();

    ui->generateButton->setEnabled(true);
    ui->generatorProgressBar->setValue(0);
    ui->generatorProgressBar->setTextVisible(false);
    ui->btnPauseGenerator->setEnabled(false);
    ui->btnPauseGenerator->setText(tr("Pause"));
    ui->btnPauseGenerator->setIcon(QIcon(":/icon/pause.png"));
    ui->btnStopGenerator->setEnabled(false);

    // Jump to last generated frame if the option is selected
    const bool jumpAfterStopped = config.value("jumptolastonstop").toBool();
    if(jumpAfterStopped) {
        const int lastIdx = ui->unsavedWidget->numThumbnails() - 1;
        const auto widget = ui->unsavedWidget->at(lastIdx);
        if(widget) {
            ui->seekSlider->setValue(widget->frameNum());
        }
    }
}

void MainWindow::on_actionVideo_Settings_triggered()
{
    videoSettingsWindow->hide();
    videoSettingsWindow->show();
    videoSettingsWindow->setWindowState(Qt::WindowActive);
}

void MainWindow::activateGifMaker()
{
    if(previewContext == ui->menuCreateGIFImage) {
        return;
    }

    previewContext = ui->menuCreateGIFImage;

    ui->actionSetEndFrame->setEnabled(true);
    ui->actionSetStartFrame->setEnabled(true);

    if(!gifMaker) {
        gifMaker = util::make_unique<vfg::ui::GifMakerWidget>();
        connect(gifMaker.get(), SIGNAL(requestPreview(QString, QString)),
                this, SLOT(displayGifPreview(QString, QString)));
    }

    const auto accepted = gifMaker->exec();
    if(!accepted) {
        previewContext = ui->menuVideo;

        ui->actionSetEndFrame->setEnabled(false);
        ui->actionSetStartFrame->setEnabled(false);
    }
}

void MainWindow::gifContextMenuTriggered(QAction* action)
{
    const auto objName = action->objectName();
    if(objName == "actionShowEditor") {
        activateGifMaker();
    }
    else if(objName == "actionSetStartFrame") {
        gifMaker->updateStartFrame(ui->seekSlider->value());
    }
    else if(objName == "actionSetEndFrame") {
        gifMaker->updateLastFrame(ui->seekSlider->value());
    }
}

void MainWindow::on_actionSave_as_PNG_triggered()
{
    if(!frameGrabber->hasVideo()) {
        return;
    }
    const int selected = ui->seekSlider->value();
    const QDir saveDir(config.value("last_save_dir", "/").toString());
    const auto saveName = QString("%1.png").arg(QString::number(selected));
    const QString defaultSavePath = saveDir.absoluteFilePath(saveName);
    const QString outFilename =
            QFileDialog::getSaveFileName(this, tr("Save as..."),
                                         defaultSavePath, tr("PNG (*.png)"));
    const auto absOutPath = QFileInfo(outFilename).absoluteDir().absolutePath();
    config.setValue("last_save_dir", absOutPath);

    QImage frame = frameGrabber->getFrame(selected);
    frame.save(absOutPath);
}
