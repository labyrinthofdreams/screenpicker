#include <map>
#include <stdexcept>
#include <utility>
#include <QtCore>
#include <QtMultimedia>
#include <QtMultimediaWidgets>
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
#include "x264encoderdialog.hpp"

Q_LOGGING_CATEGORY(MAINWINDOW, "mainwindow")

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
    mediaPlayer(new QMediaPlayer),
    seekedTime(0),
    config("config.ini", QSettings::IniFormat)
{
    ui->setupUi(this);

    setupInternal();

    dvdProgress = util::make_unique<QProgressDialog>(tr("Processing DVD..."),
                                                     tr("Abort"), 0, 100);

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

    const auto logging = config.value("enable_logging", false).toBool();
    ui->actionDebugOn->setChecked(logging);
    ui->actionDebugOff->setChecked(!logging);

    // Open recent
    buildRecentMenu();

    mediaPlayer->setVideoOutput(ui->videoPreviewWidget->videoWidget);

    connect(mediaPlayer.get(), SIGNAL(positionChanged(qint64)),
            this, SLOT(videoPositionChanged(qint64)));

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

    connect(frameGenerator.get(),   SIGNAL(finished()),
            this,                   SLOT(frameGeneratorFinished()));

    connect(frameGenerator.get(),   SIGNAL(frameReady(int, QImage)),
            this,                   SLOT(frameReceived(int, QImage)),
            Qt::QueuedConnection);

    connect(ui->unsavedWidget,      SIGNAL(maximumChanged(int)),
            ui->unsavedProgressBar, SLOT(setMaximum(int)));

    connect(ui->unsavedWidget,  SIGNAL(thumbnailDoubleClicked(int)),
            this,               SLOT(thumbnailDoubleClicked(int)));

    connect(ui->unsavedWidget,  SIGNAL(full()),
            this,               SLOT(screenshotsFull()));

    connect(ui->savedWidget,    SIGNAL(thumbnailDoubleClicked(int)),
            this,               SLOT(thumbnailDoubleClicked(int)));

    connect(ui->menuCreateGIFImage, SIGNAL(triggered(QAction*)),
            this,                   SLOT(gifContextMenuTriggered(QAction*)));
}

MainWindow::~MainWindow()
{
    qCDebug(MAINWINDOW) << "Destructor";

    config.remove("video");

    delete ui;
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

    if(frameGeneratorThread->isRunning()) {
        qCDebug(MAINWINDOW) << "Quitting frame generator + thread";
        frameGenerator->stop();
        frameGeneratorThread->quit();
        frameGeneratorThread->wait();
    }

    if(frameGrabberThread->isRunning()) {
        qCDebug(MAINWINDOW) << "Quitting frame grabber thread";
        frameGrabberThread->quit();
        frameGrabberThread->wait();
    }

    scriptEditor->close();
    videoSettingsWindow->close();

    ev->accept();
}

void MainWindow::frameReceived(const int frameNum, const QImage& frame)
{
    qCDebug(MAINWINDOW) << "Frame" << frameNum << "received in thread" << qApp->thread()->currentThreadId();
    if(frame.isNull()) {
        qCWarning(MAINWINDOW) << "Frame is null";

        return;
    }

    auto thumb = util::make_unique<vfg::ui::VideoFrameThumbnail>(frameNum, frame);
    connect(thumb.get(),    SIGNAL(customContextMenuRequested(QPoint)),
            this,           SLOT(handleUnsavedMenu(QPoint)));

    config.setValue("last_received_frame", frameNum);

    // Update widgets
    ui->unsavedWidget->addThumbnail(std::move(thumb));
    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());

    const int numGenerated = ui->generatorProgressBar->value() + 1;
    ui->generatorProgressBar->setValue(numGenerated);
}

void MainWindow::screenshotsFull()
{
    qCDebug(MAINWINDOW) << "Screenshots tab is full";

    if(config.value("removeoldestafterlimit").toBool()) {
        ui->unsavedWidget->removeFirst();
    }
    else {
        if(frameGenerator->remaining() > 0) {
            pauseFrameGenerator();
        }

        if(config.value("jumptolastonreachingmax").toBool()) {
            ui->seekSlider->setValue(config.value("last_received_frame").toInt());
        }
    }
}

void MainWindow::clearRecentMenu()
{
    config.setValue("recent", {});

    buildRecentMenu();
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
    auto menu = ui->actionRecent->menu();
    if(!menu) {
        menu = new QMenu;
        connect(menu,   SIGNAL(triggered(QAction*)),
                this,   SLOT(recentMenuTriggered(QAction*)));

        ui->actionRecent->setMenu(menu);
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
    connect(clearRecent,    SIGNAL(triggered()),
            this,           SLOT(clearRecentMenu()));

    menu->addSeparator();
    menu->addAction(clearRecent);
}

unsigned MainWindow::convertMsToFrame(const unsigned milliSecond) const
{
    const auto duration = mediaPlayer->duration();
    const auto completed = static_cast<double>(milliSecond)/duration;
    return ui->totalFramesLabel->text().toInt() * completed;
}

unsigned MainWindow::convertFrameToMs(const unsigned frameNumber) const
{
    const auto totalFrames = ui->totalFramesLabel->text().toInt();
    const auto progress = static_cast<double>(frameNumber) / totalFrames;
    const auto videoTime = mediaPlayer->duration();
    return videoTime * progress;
}

void MainWindow::recentMenuTriggered(QAction* action)
{
    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();
    }

    const auto path = action->text();
    qCDebug(MAINWINDOW) << "Triggered recent menu item" << path;
    if(!QFile::exists(path)) {
        QMessageBox::critical(this, tr("File missing"), tr("Selected file is missing"));

        removeRecentMenu(path);
    }
    else {
        resetUi();

        loadFile(path);
    }
}

void MainWindow::videoPositionChanged(const qint64 position)
{
    const auto sliderPosition = convertMsToFrame(position);
    seekedTime = sliderPosition;
    ui->seekSlider->setValue(sliderPosition);
}

void MainWindow::frameGeneratorFinished()
{
    qCDebug(MAINWINDOW) << "Frame generator finished";

    ui->btnPauseGenerator->setEnabled(false);
    ui->btnStopGenerator->setEnabled(false);
    ui->generateButton->setEnabled(true);

    // Jump to last generated frame
    const bool jumpAfterFinished = config.value("jumptolastonfinish").toBool();
    if(jumpAfterFinished) {
        qCDebug(MAINWINDOW) << "Set: Jump to last generated frame after finishing generation";

        ui->seekSlider->setValue(config.value("last_received_frame").toInt());
    }
}

void MainWindow::resetUi()
{
    qCDebug(MAINWINDOW) << "Setting up UI";

    videoSettingsWindow->resetSettings();

    scriptEditor->reset();

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
    ui->buttonPlay->setEnabled(false);

    ui->actionSave_as_PNG->setEnabled(false);
    ui->actionX264_Encoder->setEnabled(false);
}

void MainWindow::setupInternal()
{
    qCDebug(MAINWINDOW) << "Setting up internal state";

    // Set Avisynth as the default video source
    videoSource = std::make_shared<vfg::core::AvisynthVideoSource>();
    frameGrabber = std::make_shared<vfg::core::VideoFrameGrabber>(videoSource);
    frameGenerator = util::make_unique<vfg::core::VideoFrameGenerator>(frameGrabber);

    if(!frameGrabberThread) {
        qCDebug(MAINWINDOW) << "Creating frame grabber thread";
        frameGrabberThread = util::make_unique<QThread>();
    }

    if(frameGrabberThread->isRunning()) {
        qCDebug(MAINWINDOW) << "Quitting frame grabber thread";
        frameGrabberThread->quit();
    }

    qCDebug(MAINWINDOW) << "Starting frame grabber thread";
    frameGrabber->moveToThread(frameGrabberThread.get());
    frameGrabberThread->start();

    if(!frameGeneratorThread) {
        qCDebug(MAINWINDOW) << "Creating frame generator thread";
        frameGeneratorThread = util::make_unique<QThread>();
    }

    if(frameGeneratorThread->isRunning()) {
        qCDebug(MAINWINDOW) << "Quitting frame generator thread";
        frameGeneratorThread->quit();
    }

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

        const QFileInfo info(path);

        qCDebug(MAINWINDOW) << "Opening file" << info.absoluteFilePath();
        config.setValue("last_opened", info.absoluteFilePath());

        QMap<QString, QVariant> videoSettings = videoSettingsWindow->getSettings();
        videoSettings.insert("avisynthpluginspath", config.value("avisynthpluginspath"));

        // Only overwrite values in video settings if
        // they've not been set explicitly by the user
        const bool overrideWidth = (videoSettings.value("resizewidth") == 0);
        if(overrideWidth) {
            qCDebug(MAINWINDOW) << "Overriding video width";
            const int width = getMediaInfoParameter(info.absoluteFilePath(), "Video;%Width%").toInt();
            if(width != 0) {
                const double par = getMediaInfoParameter(info.absoluteFilePath(), "Video;%PixelAspectRatio%").toDouble();
                const int parWidth = par * width;
                qCDebug(MAINWINDOW) << "Original:" << width << "Par:" << par << "Width:" << parWidth;
                const int newWidth = ((parWidth % 2 == 0) ? parWidth : parWidth + 1);
                videoSettings.insert("resizewidth", newWidth);
            }
        }

        const bool overrideHeight = (videoSettings.value("resizeheight") == 0);
        if(overrideHeight) {
            qCDebug(MAINWINDOW) << "Overriding video height";
            const int height = getMediaInfoParameter(info.absoluteFilePath(), "Video;%Height%").toInt();
            if(height != 0) {
                qCDebug(MAINWINDOW) << "Height:" << height;
                const int newHeight = (height % 2 == 0) ? height : height + 1;
                videoSettings.insert("resizeheight", newHeight);
            }
        }

        qCDebug(MAINWINDOW) << "Parsing script template";

        const vfg::ScriptParser parser = videoSource->getParser(path);
        const QString parsedScript = parser.parse(videoSettings);

        scriptEditor->setContent(parsedScript);
        scriptEditor->save();
        const QString saveTo = scriptEditor->path();

        // Attempt to load the (parsed) Avisynth script
        qCDebug(MAINWINDOW) << "Loading file" << saveTo;
        videoSource->load(saveTo);

        mediaPlayer->setMedia(QUrl::fromLocalFile(saveTo));
    }
    catch(const vfg::ScriptParserError& ex)
    {
        qCCritical(MAINWINDOW) << "Script template error:" << ex.what();
        QMessageBox::warning(this, tr("Script template error"), QString(ex.what()));
    }
    catch(const vfg::exception::VideoSourceError& ex)
    {
        qCCritical(MAINWINDOW) << "Script processing error:" << ex.what();
        QMessageBox::warning(this, tr("Error while processing script"), QString(ex.what()));
    }
    catch(const std::exception& ex)
    {
        qCCritical(MAINWINDOW) << "Generic error:" << ex.what();
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

    qCDebug(MAINWINDOW) << "Changing video zoom mode:" << mode;
}

void MainWindow::contextMenuOnPreview(const QPoint &pos)
{
    qCDebug(MAINWINDOW) << "Context menu requested on preview";

    previewContext->exec(ui->videoPreviewWidget->mapToGlobal(pos));
}

void MainWindow::displayGifPreview(QString args, QString optArgs)
{
    qCDebug(MAINWINDOW) << "Displaying GIF preview";

    const auto imageMagickPath = config.value("imagemagickpath").toString();
    if(imageMagickPath.isEmpty()) {
        qCWarning(MAINWINDOW) << "ImageMagick path is not set";

        QMessageBox::critical(this, tr("Missing ImageMagick path"),
                              tr("Set path to ImageMagick and try again."));
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
        if(frame.isNull()) {
            continue;
        }

        // Save as uncompressed PNG images
        frame.save(QString("%1.png").arg(current), "PNG", 100);
        frames.append(current);

        QCoreApplication::processEvents();
    }

    qCDebug(MAINWINDOW) << "Extracted" << frames.size() << "frames";
    qCDebug(MAINWINDOW) << "Generating GIF";

    progress.setLabelText(tr("Generating GIF"));
    QCoreApplication::processEvents();

    QStringList newArgs;
    newArgs << args.split(" ") << "*.png" << "preview.gif";

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
        QFile::remove(QString("%1.png").arg(frame));
    }

    progress.setValue(end_frame + 1);

    if(!optArgs.isEmpty()) {
        qCDebug(MAINWINDOW) << "Optimizing GIF";

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
            const auto error = QProcessErrorToString(gifsicle.error(), gifsicle.errorString());
            qCCritical(MAINWINDOW) << "Gifsicle error:" << error;

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
    qCDebug(MAINWINDOW) << "Triggered Open file";

    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();
    }

    const QString lastOpened(config.value("last_opened", "").toString());

    const QString filename =
            QFileDialog::getOpenFileName(this, tr("Open video"), lastOpened,
                                         "All (*.*);;Avisynth (*.avs, *.avsi);;DGIndex (*.d2v)");
    if(filename.isEmpty()) {
        qCDebug(MAINWINDOW) << "Cancelled opening a new file";

        return;
    }

    // Reset all states back to zero
    resetUi();

    loadFile(filename);           
}

void MainWindow::on_actionOpen_DVD_triggered()
{
    qCDebug(MAINWINDOW) << "Triggered Open DVD";

    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();
    }

    const QString lastOpened(config.value("last_opened_dvd", "").toString());

    const QStringList vobFiles = QFileDialog::getOpenFileNames(this, tr("Select DVD VOB/Blu-ray M2TS files"),
                                                         lastOpened,
                                                         "DVD VOB (*.vob);;Blu-ray M2TS (*.m2ts)");
    if(vobFiles.empty()) {
        qCDebug(MAINWINDOW) << "Cancelled opening DVD";

        return;
    }

    const QFileInfo openedVobFile(vobFiles.first());
    config.setValue("last_opened_dvd", openedVobFile.absoluteDir().absolutePath());

    const QString dgIndexPath = config.value("dgindexexecpath").toString();
    if(!QFile::exists(dgIndexPath)) {
        qCWarning(MAINWINDOW) << "DGIndex invalid path";

        QMessageBox::critical(this, tr("DGIndex invalid path"),
                              tr("Please set a valid path to DGIndex"));
        ui->actionOptions->trigger();
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
    resetUi();

    dvdProgress->setValue(0);
    dvdProgress->setVisible(true);

    if(openedVobFile.suffix() == "m2ts") {
        dvdProgress->setLabelText(tr("Processing Blu-ray..."));
    }

    dvdProcessor->process(vobFiles);
}

void MainWindow::dvdProcessorFinished(const QString& path)
{
    qCDebug(MAINWINDOW) << "DVD Processor finished";

    dvdProgress->setValue(100);

    loadFile(path);
}

void MainWindow::videoSettingsUpdated()
{    
    qCDebug(MAINWINDOW) << "Video settings updated";

    if(!frameGrabber->hasVideo()) {
        qCCritical(MAINWINDOW) << "No video in frame grabber";
        QMessageBox::warning(this, tr("No video"), tr("This operation requires a video"));

        return;
    }

    loadFile(config.value("last_opened").toString());
}

void MainWindow::scriptEditorUpdated()
{
    qCDebug(MAINWINDOW) << "Script editor updated";

    loadFile(scriptEditor->path());
}

void MainWindow::videoLoaded()
{
    qCDebug(MAINWINDOW) << "Video loaded";

    setWindowTitle(config.value("last_opened").toString());

    appendRecentMenu(config.value("last_opened").toString());

    config.setValue("last_opened_script", videoSource->fileName());

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

    ui->buttonPlay->setEnabled(true);

    ui->actionSave_as_PNG->setEnabled(true);
    ui->actionX264_Encoder->setEnabled(true);

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
    qCWarning(MAINWINDOW) << "Video error:" << msg;
    dvdProgress->cancel();
    QMessageBox::warning(this, tr("Video error"), msg);
}

void MainWindow::thumbnailDoubleClicked(const int frameNumber)
{    
    qCDebug(MAINWINDOW) << "Double clicked thumbnail";
    QMetaObject::invokeMethod(frameGrabber.get(), "requestFrame",
                              Qt::QueuedConnection, Q_ARG(int, frameNumber));

    ui->currentFrameLabel->setText(QString::number(frameNumber));
    ui->seekSlider->setValue(frameNumber);
}

void MainWindow::on_nextButton_clicked()
{
    qCDebug(MAINWINDOW) << "Clicked next button";

    frameGrabber->requestNextFrame();
    const int lastRequestedFrame = frameGrabber->lastFrame();

    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_previousButton_clicked()
{
    qCDebug(MAINWINDOW) << "Clicked previous button";

    frameGrabber->requestPreviousFrame();
    const int lastRequestedFrame = frameGrabber->lastFrame();

    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_seekSlider_valueChanged(const int frameNumber)
{
    qCDebug(MAINWINDOW) << "Moved seek slider";

    // This check makes sure that the slider was moved by the user
    // and not auto-moved by videoPositionChanged
    if(frameNumber != seekedTime && seekedTime > 0) {
        mediaPlayer->setPosition(convertFrameToMs(frameNumber));
    }

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
    qCDebug(MAINWINDOW) << "Clicked generate button";

    if(frameGenerator->isRunning()) {
        qCWarning(MAINWINDOW) << "Frame generator is running";

        return;
    }

    const bool pauseAfterLimit = config.value("pauseafterlimit").toBool();
    if(pauseAfterLimit && ui->unsavedWidget->isFull()) {
        // Can't start the generator if the unsaved widget container is full
        // and user has selected to pause after the container is full
        qCWarning(MAINWINDOW) << "Thumbnail container is full";

        QMessageBox::information(this, tr("Thumbnail container is full"),
                tr("Click 'Clear' or raise the max thumbnail limit."));
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
        qCDebug(MAINWINDOW) << "Restarting frame generator";

        frameGenerator->stop();
    }

    QList<int> queue;
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
        if(reached_video_end) {
            break;
        }

        queue.append(current_frame);
    }

    frameGenerator->enqueue(queue);

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
    qCDebug(MAINWINDOW) << "Clicked grab button";

    const int selectedFrame = ui->seekSlider->value();
    const QImage frame = frameGrabber->getFrame(selectedFrame);
    if(frame.isNull()) {
        qCCritical(MAINWINDOW) << "Frame is null";
        QMessageBox::critical(this, tr("Invalid image"), tr("Invalid image format. Try again."));

        return;
    }

    auto thumb = util::make_unique<vfg::ui::VideoFrameThumbnail>(selectedFrame, frame);
    connect(thumb.get(),    SIGNAL(customContextMenuRequested(QPoint)),
            this,           SLOT(handleSavedMenu(QPoint)));

    ui->savedWidget->addThumbnail(std::move(thumb));

    statusBar()->showMessage(tr("Grabbed frame #%1").arg(selectedFrame), 3000);
}

void MainWindow::handleUnsavedMenu(const QPoint &pos)
{
    Q_UNUSED(pos);

    qCDebug(MAINWINDOW) << "Moving unsaved screenshot";

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
            qCWarning(MAINWINDOW) << "Thumbnail is null";

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

    qCDebug(MAINWINDOW) << "Moving saved screenshot";

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
            qCWarning(MAINWINDOW) << "Thumbnail is null";

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
        resumeFrameGenerator();
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
    qCDebug(MAINWINDOW) << "Clicked save button";

    // Save saved images to disk
    if(ui->savedWidget->isEmpty())
    {
        qCWarning(MAINWINDOW) << "Nothing to save";
        QMessageBox::information(this, tr("Nothing to save"),
                                 tr("Add one or more screenshots to queue."));
        return;
    }

    // Pause frame generator
    if(frameGenerator->isRunning()) {
        pauseFrameGenerator();
    }

    const QString lastSaveDirectory =
            QFileDialog::getExistingDirectory(this, tr("Select save directory"),
                                              config.value("last_save_dir", "/").toString());
    if(lastSaveDirectory.isEmpty()) {
        return;
    }

    config.setValue("last_save_dir", lastSaveDirectory);

    const std::size_t numSaved = ui->savedWidget->numThumbnails();    
    const QDir saveDir(lastSaveDirectory);

    QProgressDialog prog("", "Cancel", 0, numSaved, this);
    prog.setWindowModality(Qt::WindowModal);
    prog.setCancelButton(0);
    prog.setMinimumDuration(0);
    for(std::size_t idx = 0; idx < numSaved; ++idx) {
        const auto widget = ui->savedWidget->at(idx);
        if(!widget) {
            qCCritical(MAINWINDOW) << "Invalid widget";

            break;
        }

        const int current = prog.value();
        prog.setLabelText(tr("Saving image %1 of %2").arg(current).arg(numSaved));
        prog.setValue(current + 1);
        if(prog.wasCanceled()) {
            qCCritical(MAINWINDOW) << "Aborted";

            QMessageBox::warning(this, tr("Saving thumbnails aborted"),
                                 tr("Saved %1 of %2 thumbnails").arg(current).arg(numSaved));
            break;
        }

        const int frameNumber = widget->frameNum();
        const QString filename = QString("%1.png").arg(QString::number(frameNumber));
        const QString savePath = saveDir.absoluteFilePath(filename);

        const QImage frame = frameGrabber->getFrame(frameNumber);
        if(frame.isNull()) {
            qCCritical(MAINWINDOW) << "Frame is null";

            continue;
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
    qCDebug(MAINWINDOW) << "Drop event";

    const QList<QUrl> urls = ev->mimeData()->urls();
    if(urls.length() > 1) {
        qCWarning(MAINWINDOW) << "Too many dropped objects";

        ev->ignore();
        QMessageBox::information(this, tr("Drop event"),
                                 tr("You can drop only one file"));
        return;
    }

    ev->acceptProposedAction();

    const QString filename = urls.at(0).toLocalFile();

    // Reset all states back to zero
    resetUi();

    loadFile(filename);
}

void MainWindow::on_actionOptions_triggered()
{
    vfg::ConfigDialog configDialog;
    const auto saved = configDialog.exec();
    if(saved) {
        ui->unsavedWidget->setMaxThumbnails(config.value("maxthumbnails").toInt());

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
        pauseFrameGenerator();

        // Jump to last generated frame if the option is selected
        const bool jumpAfterPaused = config.value("jumptolastonpause").toBool();
        if(jumpAfterPaused) {
            ui->seekSlider->setValue(config.value("last_received_frame").toInt());
        }
    }
    else if(frameGenerator->isPaused())
    {
        if(ui->unsavedWidget->isFull()) {
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
        ui->seekSlider->setValue(config.value("last_received_frame").toInt());
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

void MainWindow::pauseFrameGenerator()
{
    qCDebug(MAINWINDOW) << "Pausing frame generator";
    if(!frameGenerator->isRunning()) {
        qCDebug(MAINWINDOW) << "Not running";

        return;
    }

    frameGenerator->pause();

    ui->btnPauseGenerator->setText(tr("Resume"));
    ui->btnPauseGenerator->setIcon(QIcon(":/icon/resume.png"));
    ui->generateButton->setEnabled(true);
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

    ui->btnPauseGenerator->setText(tr("Pause"));
    ui->btnPauseGenerator->setIcon(QIcon(":/icon/pause.png"));
    ui->generateButton->setEnabled(false);
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
    qCDebug(MAINWINDOW) << "Saving current as PNG";

    if(!frameGrabber->hasVideo()) {
        qCCritical(MAINWINDOW) << "No video";

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
    if(!frame.isNull()) {
        frame.save(outFilename);
    }
    else {
        qCWarning(MAINWINDOW) << "Frame is null";
        QMessageBox::critical(this, tr("Invalid image"), tr("Invalid image format. Try again."));
    }
}

void MainWindow::on_actionX264_Encoder_triggered()
{
    vfg::ui::x264EncoderDialog w;
    w.exec();
}

void MainWindow::on_actionDebugOn_triggered(bool checked)
{
    ui->actionDebugOff->setChecked(!checked);
    config.setValue("enable_logging", true);

    QMessageBox::information(this, tr("Restart"), tr("Please restart the application"));
}

void MainWindow::on_actionDebugOff_triggered(bool checked)
{
    ui->actionDebugOn->setChecked(!checked);
    config.setValue("enable_logging", false);

    QMessageBox::information(this, tr("Restart"), tr("Please restart the application"));
}

void MainWindow::on_buttonPlay_clicked()
{
    const auto state = mediaPlayer->state();
    if(state == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
        ui->videoPreviewWidget->hideVideo();
        videoPositionChanged(mediaPlayer->position());
        ui->buttonPlay->setIcon(QIcon(":/icon/play.png"));
        ui->buttonPlay->setText(tr("Play"));
        seekedTime = 0;
    }
    else {
        ui->videoPreviewWidget->showVideo();
        mediaPlayer->play();
        ui->buttonPlay->setIcon(QIcon(":/icon/pause2.png"));
        ui->buttonPlay->setText(tr("Pause"));
        // Start playing from where the seek slider is
        mediaPlayer->setPosition(convertFrameToMs(ui->seekSlider->value()));
    }
}
