#include <map>
#include <QtWidgets>
#include <QtCore>
#include <stdexcept>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videoframegrabber.h"
#include "videoframethumbnail.h"
#include "avisynthvideosource.h"
#include "scripteditor.h"
#include "configdialog.h"
#include "videoframegenerator.h"
#include "dvdprocessor.h"
#include "scriptparserfactory.h"
#include "scriptparser.h"
#include "videosettingswidget.h"

QMap<QString, QString> avinfoParseVideoHeader(QString path)
{
    QMap<QString, QString> videoHeader;
    QProcess proc;
    proc.start(QDir::current().absoluteFilePath("avinfo.exe"), QStringList() << path << "--raw");
    // Wait for 3 seconds max
    bool ok = proc.waitForFinished(3000);
    if(ok)
    {
        QList<QByteArray> output = proc.readAllStandardOutput().split('\n');
        for(int i = 0; i < output.size(); ++i)
        {
            QString line(output.at(i).trimmed());
            QStringList kv = line.split("=");
            if(kv.size() < 2)
            {
                break;
            }
            videoHeader.insert(kv.at(0), kv.at(1));
        }
    }

    return videoHeader;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    videoSource(),
    framesToSave(),
    lastOpenedFile(),
    lastSaveDirectory("/"),
    config("config.ini", QSettings::IniFormat),
    lastRequestedFrame(vfg::FirstFrame)
{
    ui->setupUi(this);

    try
    {
        // Set Avisynth as the default video source
        videoSource = std::make_shared<vfg::core::AvisynthVideoSource>();

        frameGrabber = new vfg::core::VideoFrameGrabber(videoSource);

        frameGenerator = new vfg::core::VideoFrameGenerator(frameGrabber);
        frameGeneratorThread = new QThread(this);
        frameGenerator->moveToThread(frameGeneratorThread);
        frameGeneratorThread->start();

        frameGrabberThread = new QThread(this);
        frameGrabber->moveToThread(frameGrabberThread);
        frameGrabberThread->start();

        scriptEditor = new vfg::ui::ScriptEditor;

        videoSettingsWindow = new vfg::ui::VideoSettingsWidget;

        QString dgIndexPath = config.value("dgindexexecpath").toString();
        dvdProcessor = new vfg::DvdProcessor(dgIndexPath, this);

        const int maxThumbnails = config.value("maxthumbnails").toInt();
        ui->unsavedWidget->setMaxThumbnails(maxThumbnails);
        ui->unsavedProgressBar->setMaximum(maxThumbnails);

        const int numScreenshots = config.value("numscreenshots").toInt();
        ui->screenshotsSpinBox->setValue(numScreenshots);

        const int frameStep = config.value("framestep").toInt();
        ui->frameStepSpinBox->setValue(frameStep);

        videoZoomGroup = new QActionGroup {this};
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
        connect(videoZoomGroup, SIGNAL(triggered(QAction*)),
                this, SLOT(videoZoomChanged(QAction*)));

        connect(ui->videoPreviewWidget, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(contextMenuOnPreview(QPoint)));
    }
    catch(std::exception& ex)
    {
        throw;
    }

    connect(videoSettingsWindow, SIGNAL(settingsChanged()),
            this, SLOT(videoSettingsUpdated()));

    connect(videoSettingsWindow, SIGNAL(cropChanged(QRect)),
            ui->videoPreviewWidget, SLOT(setCrop(QRect)));

    connect(videoSettingsWindow, SIGNAL(closed()),
            ui->videoPreviewWidget, SLOT(resetCrop()));

    connect(scriptEditor, SIGNAL(scriptUpdated()),
            this, SLOT(scriptEditorUpdated()));

    connect(dvdProcessor, SIGNAL(finished(QString)),
            this, SLOT(dvdProcessorFinished(QString)));

    connect(dvdProcessor, SIGNAL(error(QString)),
            this, SLOT(videoError(QString)));

    connect(videoSource.get(), SIGNAL(videoLoaded()),
            this, SLOT(videoLoaded()));

    connect(frameGrabber, SIGNAL(errorOccurred(QString)),
            this, SLOT(videoError(QString)),
            Qt::QueuedConnection);

    connect(frameGrabber, SIGNAL(frameGrabbed(QPair<int,QImage>)),
            ui->videoPreviewWidget, SLOT(setFrame(QPair<int,QImage>)),
            Qt::QueuedConnection);

    connect(frameGenerator, SIGNAL(frameReady(QPair<int, QImage>)),
            this, SLOT(frameReceived(QPair<int, QImage>)),
            Qt::QueuedConnection);

    connect(ui->unsavedWidget, SIGNAL(maximumChanged(int)),
            ui->unsavedProgressBar, SLOT(setMaximum(int)));

    connect(ui->unsavedWidget, SIGNAL(thumbnailDoubleClicked(int)),
            this, SLOT(thumbnailDoubleClicked(int)));

    connect(ui->savedWidget, SIGNAL(thumbnailDoubleClicked(int)),
            this, SLOT(thumbnailDoubleClicked(int)));
}

MainWindow::~MainWindow()
{
    delete videoSettingsWindow;
    delete scriptEditor;
    delete frameGenerator;
    delete frameGrabber;
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

void MainWindow::frameReceived(QPair<int, QImage> frame)
{
    qDebug() << "FRAME_RECEIVED in thread" << qApp->thread()->currentThreadId() << frame.first;
    const int thumbnailSize = ui->thumbnailSizeSlider->value();
    QPixmap thumbnail = QPixmap::fromImage(frame.second).scaledToWidth(200, Qt::SmoothTransformation);
    auto thumb = new vfg::ui::VideoFrameThumbnail(frame.first, thumbnail);
    thumb->setFixedWidth(thumbnailSize);

    connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(handleUnsavedMenu(QPoint)));

    // Update widgets
    ui->unsavedWidget->addThumbnail(thumb);
    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());

    if(frameGenerator->isPaused())
    {
        // Jump to last generated frame
        const bool jumpAfterPaused = config.value("jumptolastonpause").toBool();
        if(jumpAfterPaused) {
            ui->seekSlider->setValue(frame.first);
        }
    }

    // If true, implies that generator has been explicitly stopped,
    // otherwise is still running or has finished
    const bool generatorStopped = ui->generatorProgressBar->value() == 0 && !frameGenerator->isRunning();
    if(generatorStopped) {
        // Jump to last generated frame
        const bool jumpAfterStopped = config.value("jumptolastonstop").toBool();
        if(jumpAfterStopped) {
            ui->seekSlider->setValue(frame.first);
        }
    }
    else {
        const int generated = ui->generatorProgressBar->value() + 1;
        ui->generatorProgressBar->setValue(generated);

        const bool generatorFinished = frameGenerator->remaining() == 0;
        if(generatorFinished)
        {
            // Generator has finished without explicit stopping
            ui->btnPauseGenerator->setEnabled(false);
            ui->btnStopGenerator->setEnabled(false);

            // Jump to last generated frame
            const bool jumpAfterFinished = config.value("jumptolastonfinish").toBool();
            if(jumpAfterFinished) {
                ui->seekSlider->setValue(frame.first);
            }
        }
    }

    if(ui->unsavedWidget->isFull())
    {
        const bool removeOldestAfterMax = config.value("removeoldestafterlimit").toBool();
        if(removeOldestAfterMax)
        {
            // When unsaved screenshots container becomes full and the setting
            // "remove oldest after reaching max" is checked, we simply get another frame...
            frameGenerator->fetchNext();
        }
        else
        {
            // ...If the user has NOT checked that option and chooses to pause instead...

            // ...In case the user has checked they want to jump to last generated frame
            // after filling the container, then jump...
            const bool jumpToLastAfterReachingMax = config.value("jumptolastonreachingmax").toBool();
            if(jumpToLastAfterReachingMax)
            {
                ui->seekSlider->setValue(frame.first);
            }

            // ...and wait for the user to click Clear, Generate, or open another file
        }
    }
    else {
        frameGenerator->fetchNext();
    }
}

void MainWindow::resetState()
{
    videoSettingsWindow->resetSettings();

    scriptEditor->reset();

    lastRequestedFrame = vfg::FirstFrame;
    lastOpenedFile.clear();

    ui->unsavedWidget->clearThumbnails();
    ui->savedWidget->clearThumbnails();
    framesToSave.clear();

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
    ui->saveSingleButton->setEnabled(false);
    ui->btnPauseGenerator->setEnabled(false);
    ui->btnStopGenerator->setEnabled(false);
    ui->generatorProgressBar->setValue(0);
    ui->generatorProgressBar->setTextVisible(false);
}

void MainWindow::loadFile(QString path)
{
    try
    {
        vfg::ScriptParserFactory parserFactory;
        QSharedPointer<vfg::ScriptParser> parser = parserFactory.parser(path);

        QMap<QString, int> videoSettings = videoSettingsWindow->getSettings();

        // Only overwrite values in video settings if
        // they've not been set explicitly by the user
        const bool overrideWidth = (videoSettings.value("resizewidth") == 0);
        const bool overrideHeight = (videoSettings.value("resizeheight") == 0);
        const bool overrideSize = (overrideWidth || overrideHeight);
        if(overrideSize)
        {
            QMap<QString, QString> videoHeader = avinfoParseVideoHeader(path);

            if(overrideWidth)
            {
                int resizeWidth = videoHeader.value("v1.x", "0").toInt();
                if(resizeWidth % 2 != 0) {
                    resizeWidth++;
                }
                videoSettings.insert("resizewidth", resizeWidth);
            }
            if(overrideHeight)
            {
                int resizeHeight = videoHeader.value("v1.y", "0").toInt();
                if(resizeHeight % 2 != 0) {
                    resizeHeight++;
                }
                videoSettings.insert("resizeheight", resizeHeight);
            }
        }

        QString parsedScript = parser->parse(videoSettings);

        scriptEditor->setContent(parsedScript);
        scriptEditor->save();
        QString saveTo = scriptEditor->path();

        // Attempt to load the (parsed) Avisynth script
        // TODO: Stop screenshot generation if that's happening...
        videoSource->load(saveTo);
    }
    catch(vfg::ScriptParserTemplateException& ex)
    {
        QMessageBox::warning(this, tr("Script template error"), QString(ex.what()));
    }
    catch(vfg::exception::VideoSourceError& ex)
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

void MainWindow::videoZoomChanged(QAction *action)
{
    QString mode = action->data().toString();
    qDebug() << "mode:" << mode;
    std::map<QString, vfg::ZoomMode> m {
        {"25", vfg::ZoomMode::Zoom_25},
        {"50", vfg::ZoomMode::Zoom_50},
        {"100", vfg::ZoomMode::Zoom_100},
        {"200", vfg::ZoomMode::Zoom_200},
        {"scale", vfg::ZoomMode::Zoom_Scale}
    };
    ui->videoPreviewWidget->setZoom(m[mode]);
}

void MainWindow::contextMenuOnPreview(const QPoint &pos)
{
    ui->menuZoom->exec(ui->videoPreviewWidget->mapToGlobal(pos));
}

void MainWindow::on_actionOpen_triggered()
{
    if(frameGenerator->isRunning()) {
        frameGenerator->pause();
        ui->btnPauseGenerator->setText(tr("Resume"));
    }

    QString lastOpened {config.value("last_opened", "").toString()};

    QString filename = QFileDialog::getOpenFileName(this, tr("Open video"),
                                                    lastOpened,
                                                    "All (*.*);;Avisynth (*.avs, *.avsi);;DGIndex (*.d2v)");
    if(filename.isEmpty())
        return;

    // Reset all states back to zero
    resetState();

    loadFile(filename);       

    QFileInfo info {filename};
    setWindowTitle(info.absoluteFilePath());
    config.setValue("last_opened", info.absoluteFilePath());

    lastOpenedFile = filename;
}

void MainWindow::on_actionOpen_DVD_triggered()
{
    if(frameGenerator->isRunning()) {
        frameGenerator->pause();
        ui->btnPauseGenerator->setText(tr("Resume"));
    }

    QString lastOpened {config.value("last_opened_dvd", "").toString()};

    QStringList vobFiles = QFileDialog::getOpenFileNames(this, tr("Select DVD VOB/Blu-ray M2TS files"),
                                                         lastOpened,
                                                         "DVD VOB (*.vob);;Blu-ray M2TS (*.m2ts)");
    if(vobFiles.empty())
    {
        return;
    }

    QFileInfo openedVobFile {vobFiles.first()};
    config.setValue("last_opened_dvd", openedVobFile.absoluteDir().absolutePath());

    QString dgIndexPath = config.value("dgindexexecpath").toString();
    if(!QFile::exists(dgIndexPath))
    {
        QMessageBox::critical(this, tr("DGIndex invalid path"), tr("Please set a valid path to DGIndex"));
        ui->actionOptions->trigger();
        return;
    }

    // Reset all states back to zero
    resetState();

    dvdProcessor->process(vobFiles);
}

void MainWindow::dvdProcessorFinished(QString path)
{
    loadFile(path);

    QFileInfo info(path);
    setWindowTitle(info.absoluteFilePath());

    lastOpenedFile = path;
}

void MainWindow::videoSettingsUpdated()
{    
    if(!frameGrabber->hasVideo())
    {
        QMessageBox::warning(this, tr("No video"), tr("This operation requires a video"));
        return;
    }

    loadFile(lastOpenedFile);
}

void MainWindow::scriptEditorUpdated()
{
    const QString path = scriptEditor->path();
    loadFile(path);

    QFileInfo info(path);
    setWindowTitle(info.absoluteFilePath());

    lastOpenedFile = path;
}

void MainWindow::videoLoaded()
{
    const int numFrames = frameGrabber->totalFrames();

    const bool invalidRange = !frameGrabber->isValidFrame(lastRequestedFrame);
    if(invalidRange)
    {
        // lastRequestFrame may be out of range when the script
        // is reloaded via the editor and when the script produces
        // video with fewer frames than the last request frame
        lastRequestedFrame = vfg::FirstFrame;
    }
    // Update frame numbers on the labels
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->totalFramesLabel->setText(QString::number(numFrames));
    // Update maximum for seek slider
    ui->seekSlider->setEnabled(true);
    ui->seekSlider->setMaximum(numFrames);
    // Move slider back to first frame
    ui->seekSlider->setValue(lastRequestedFrame);
    // Show first frame
    QMetaObject::invokeMethod(frameGrabber, "requestFrame",
                              Qt::QueuedConnection, Q_ARG(int, lastRequestedFrame));

    // Enable buttons
    ui->previousButton->setEnabled(true);
    ui->nextButton->setEnabled(true);
    ui->grabButton->setEnabled(true);
    ui->generateButton->setEnabled(true);
    ui->saveSingleButton->setEnabled(true);

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

void MainWindow::videoError(QString msg)
{
    QMessageBox::warning(this, tr("Video error"), msg);
}

void MainWindow::thumbnailDoubleClicked(int frameNumber)
{    
    QMetaObject::invokeMethod(frameGrabber, "requestFrame",
                              Qt::QueuedConnection, Q_ARG(int, frameNumber));
    lastRequestedFrame = frameNumber;
    ui->currentFrameLabel->setText(QString::number(frameNumber));
    ui->seekSlider->setValue(frameNumber);
}

void MainWindow::on_nextButton_clicked()
{
    qDebug() << "Start Main Thread " << qApp->thread()->currentThreadId();
    QMetaObject::invokeMethod(frameGrabber, "requestNextFrame", Qt::QueuedConnection);
    lastRequestedFrame++;
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_previousButton_clicked()
{
    qDebug() << "Main Thread " << qApp->thread()->currentThreadId();
    QMetaObject::invokeMethod(frameGrabber, "requestPreviousFrame", Qt::QueuedConnection);
    lastRequestedFrame--;
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_seekSlider_valueChanged(int value)
{
    const int frameNumber = value;
    if(lastRequestedFrame == frameNumber)
    {
        return;
    }

    QMetaObject::invokeMethod(frameGrabber, "requestFrame",
                              Qt::QueuedConnection, Q_ARG(int, frameNumber));
    lastRequestedFrame = frameNumber;
    ui->currentFrameLabel->setText(QString::number(frameNumber));
}

void MainWindow::on_seekSlider_sliderMoved(int position)
{
    //lastRequestedFrame = position;
    ui->currentFrameLabel->setText(QString::number(position));
}

void MainWindow::on_generateButton_clicked()
{
    const bool pauseAfterLimit = config.value("pauseafterlimit").toBool();
    if(pauseAfterLimit && ui->unsavedWidget->isFull()) {
        // If user has chosen to pause the generator after reaching
        // maximum limit and has clicked resume (this path) while
        // the container is still full we can't resume
        QMessageBox::information(this, tr(""), tr("Can't start generator while the container has reached max limit.\n"
                                                  "Click 'Clear' or raise the max thumbnail limit to continue."));
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

    if(frameGenerator->isRunning() || frameGenerator->isPaused()) {
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

    const int remaining = frameGenerator->remaining();

    QMetaObject::invokeMethod(frameGenerator, "start", Qt::QueuedConnection);

    // Update generator widgets
    ui->btnPauseGenerator->setEnabled(true);
    ui->btnPauseGenerator->setText(tr("Pause"));
    ui->btnStopGenerator->setEnabled(true);
    ui->generatorProgressBar->setValue(0);
    ui->generatorProgressBar->setMaximum(remaining);
    ui->generatorProgressBar->setTextVisible(true);
}

void MainWindow::on_grabButton_clicked()
{
    const int selected = ui->seekSlider->value();
    const int thumbnailSize = ui->thumbnailSizeSlider->value();
    QImage frame = frameGrabber->getFrame(selected);
    QPixmap thumbnail = QPixmap::fromImage(frame).scaledToWidth(200, Qt::SmoothTransformation);

    auto thumb = new vfg::ui::VideoFrameThumbnail(selected, thumbnail);
    thumb->setFixedWidth(thumbnailSize);
    connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(handleSavedMenu(QPoint)));
    ui->savedWidget->addThumbnail(thumb);
    framesToSave.append(thumb->frameNum());


    statusBar()->showMessage(tr("Grabbed frame #%1").arg(selected), 3000);
}

void MainWindow::handleUnsavedMenu(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu menu;
    QAction *saveAction = new QAction(tr("Save"), this);
    saveAction->setData(1);
    menu.addAction(saveAction);

    QAction* selected = menu.exec(QCursor::pos());
    if(selected && selected->data().toInt() == 1)
    {
        // Move thumbnail from unsaved tab to saved tab
        auto thumb = ui->unsavedWidget->takeSelected();
        disconnect(thumb.get(), SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(handleUnsavedMenu(QPoint)));
        connect(thumb.get(), SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(handleSavedMenu(QPoint)));

        framesToSave.append(thumb->frameNum());

        ui->savedWidget->addThumbnail(thumb.release());
        ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());
    }
}

void MainWindow::handleSavedMenu(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu menu;
    QAction *unsaveAction = new QAction(tr("Unsave"), this);
    unsaveAction->setData(1);
    menu.addAction(unsaveAction);

    QAction* selected = menu.exec(QCursor::pos());
    if(selected && selected->data().toInt() == 1)
    {
        // Move thumbnail from saved tab to unsaved tab
        auto thumb = ui->savedWidget->takeSelected();
        disconnect(thumb.get(), SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(handleSavedMenu(QPoint)));
        connect(thumb.get(), SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(handleUnsavedMenu(QPoint)));

        framesToSave.removeOne(thumb->frameNum());

        ui->unsavedWidget->addThumbnail(thumb.release());
        ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());
    }
}

void MainWindow::on_clearThumbsButton_clicked()
{
    ui->unsavedWidget->clearThumbnails();
    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());

    const bool generatorHasMore = frameGenerator->remaining() > 0;
    const bool generatorNotExplicitlyPaused = !frameGenerator->isPaused();
    if(generatorHasMore && generatorNotExplicitlyPaused) {
        frameGenerator->fetchNext();
    }
}

void MainWindow::on_thumbnailSizeSlider_sliderMoved(int position)
{
    ui->unsavedWidget->resizeThumbnails(position);
    ui->savedWidget->resizeThumbnails(position);
}

void MainWindow::on_thumbnailSizeSlider_valueChanged(int value)
{
    ui->unsavedWidget->resizeThumbnails(value);
    ui->savedWidget->resizeThumbnails(value);
}

void MainWindow::on_saveThumbnailsButton_clicked()
{
    // Save saved images to disk
    if(framesToSave.isEmpty())
    {
        QMessageBox::information(this, tr("Save screenshots"),
                                 tr("Nothing to save. Add one or more screenshots to save."));
        return;
    }

    // Pause frame generator
    if(frameGenerator->isRunning()) {
        frameGenerator->pause();
    }

    lastSaveDirectory = QFileDialog::getExistingDirectory(this, tr("Select save directory"),
                                                          lastSaveDirectory);
    if(lastSaveDirectory.isEmpty())
    {
        return;
    }

    int resizeWidth = 0;
    QMessageBox::StandardButton clicked = QMessageBox::question(this, tr("Resize thumbnails"),
                                                                tr("Do you want to resize thumbnails?"),
                                                                QMessageBox::Yes | QMessageBox::No,
                                                                QMessageBox::No);
    if(clicked == QMessageBox::Yes)
    {
        resizeWidth = QInputDialog::getInt(this, tr("Resize thumbnails"), tr("Resize to width:"));
    }

    const int numSaved = framesToSave.count();
    QProgressDialog prog("", "Cancel", 0, numSaved, this);
    prog.setWindowModality(Qt::WindowModal);
    prog.setCancelButton(0);
    prog.setMinimumDuration(0);
    QDir saveDir(lastSaveDirectory);
    QListIterator<int> iter(framesToSave);
    while(iter.hasNext())
    {
        const int frameNumber = iter.next();
        const int current = prog.value();
        prog.setLabelText(tr("Saving image %1 of %2").arg(current).arg(numSaved));
        prog.setValue(current + 1);
        if(prog.wasCanceled())
        {
            QMessageBox::warning(this, tr("Saving thumbnails aborted"),
                                 tr("Saved %1 of %2 thumbnails").arg(current).arg(numSaved));
            break;
        }

        QString filename = QString("%1.png").arg(QString::number(frameNumber));
        QString savePath = saveDir.absoluteFilePath(filename);

        // Get current frame
        QImage frame = frameGrabber->getFrame(frameNumber);
        if(resizeWidth > 0)
        {
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
    QList<QUrl> urls = ev->mimeData()->urls();
    if(urls.length() > 1)
    {
        ev->ignore();
        QMessageBox::information(this, tr("Drop event"),
                                 tr("You can drop only one file"));
        return;
    }

    ev->acceptProposedAction();

    QString filename = urls.at(0).toLocalFile();

    // Reset all states back to zero
    resetState();

    loadFile(filename);

    QFileInfo info(filename);
    setWindowTitle(info.absoluteFilePath());

    config.setValue("last_opened", info.absoluteFilePath());

    lastOpenedFile = filename;
}

void MainWindow::on_actionOptions_triggered()
{
    vfg::ConfigDialog configDialog;
    const int saved = configDialog.exec();

    if(saved)
    {
        const int newMaxThumbnails = config.value("maxthumbnails").toInt();
        const bool containerFull = ui->unsavedWidget->isFull();
        const bool containerHasRoom = (newMaxThumbnails > ui->unsavedWidget->numThumbnails());
        const bool generatorHasQueue = (frameGenerator->remaining() > 0);
        const bool generatorWaiting = (frameGenerator->isRunning() && !frameGenerator->isPaused());
        const bool continueGenerator = (containerFull && containerHasRoom
                                        && generatorHasQueue && generatorWaiting);

        if(continueGenerator)
        {
            frameGenerator->fetchNext();
        }

        ui->unsavedWidget->setMaxThumbnails(newMaxThumbnails);
        dvdProcessor->setProcessor(config.value("dgindexexecpath").toString());
    }
}

void MainWindow::on_screenshotsSpinBox_valueChanged(int arg1)
{
    config.setValue("numscreenshots", arg1);

    ui->screenshotsSpinBox->setValue(arg1);
}

void MainWindow::on_frameStepSpinBox_valueChanged(int arg1)
{
    config.setValue("framestep", arg1);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, tr("About"), qApp->applicationVersion());
}

void MainWindow::on_saveSingleButton_clicked()
{
    const int selected = ui->seekSlider->value();
    QImage frame = frameGrabber->getFrame(selected);

    QDir saveDir(lastSaveDirectory);
    QString defaultSavePath = saveDir.absoluteFilePath(QString("%1.png").arg(QString::number(selected)));
    QString outFilename = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                                       defaultSavePath,
                                                       tr("PNG (*.png)"));
    QFileInfo info(outFilename);
    lastSaveDirectory = info.absoluteDir().absolutePath();

    int resizeWidth = 0;
    QMessageBox::StandardButton clicked = QMessageBox::question(this, tr("Resize thumbnails"),
                                                                tr("Do you want to resize thumbnails?"),
                                                                QMessageBox::Yes | QMessageBox::No,
                                                                QMessageBox::No);
    if(clicked == QMessageBox::Yes)
    {
        resizeWidth = QInputDialog::getInt(this, tr("Resize thumbnails"), tr("Resize to width:"));
        if(resizeWidth > 0) {
            frame = frame.scaledToWidth(resizeWidth, Qt::SmoothTransformation);
        }
    }

    frame.save(outFilename);
}

void MainWindow::on_cbUnlimitedScreens_clicked(bool checked)
{
    ui->screenshotsSpinBox->setEnabled(!checked);
}

void MainWindow::on_btnPauseGenerator_clicked()
{
    if(!frameGenerator->isPaused())
    {
        frameGenerator->pause();
        ui->btnPauseGenerator->setText(tr("Resume"));
    }
    else
    {
        const bool pauseAfterLimit = config.value("pauseafterlimit").toBool();
        if(pauseAfterLimit && ui->unsavedWidget->isFull()) {
            // If user has chosen to pause the generator after reaching
            // maximum limit and has clicked resume (this path) while
            // the container is still full we can't resume
            QMessageBox::information(this, tr(""), tr("Can't resume generator while the container has reached max limit.\n"
                                                      "Click 'Clear' or raise the max thumbnail limit to continue."));
            return;
        }
        ui->btnPauseGenerator->setText(tr("Pause"));
        // TODO: If generator is waiting, this will fail (should be fixed)
        if(frameGenerator->isPaused()) {
            QMetaObject::invokeMethod(frameGenerator, "resume", Qt::QueuedConnection);
        }
        else {
            frameGenerator->fetchNext();
        }
    }
}

void MainWindow::on_btnStopGenerator_clicked()
{
    frameGenerator->stop();
    ui->generatorProgressBar->setValue(0);
    ui->generatorProgressBar->setTextVisible(false);
    ui->btnPauseGenerator->setEnabled(false);
    ui->btnPauseGenerator->setText(tr("Pause"));
    ui->btnStopGenerator->setEnabled(false);
}

void MainWindow::on_actionVideo_Settings_triggered()
{
    videoSettingsWindow->hide();
    videoSettingsWindow->show();
    videoSettingsWindow->setWindowState(Qt::WindowActive);
}
