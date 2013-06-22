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

// TODO: Project files to save progress?
// TODO: Add parsers (loading automation) for common video formats to reduce scripting
// TODO: The way scripts are tied to scripteditor is bad design probably...
// TODO: Make script templates modifiable

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    framesToSave(),
    lastRequestedFrame(vfg::FirstFrame)
{
    ui->setupUi(this);

    try
    {
        frameGrabber = new vfg::VideoFrameGrabber;

        frameGenerator = new vfg::VideoFrameGenerator(frameGrabber);
        frameGeneratorThread = new QThread(this);
        frameGenerator->moveToThread(frameGeneratorThread);
        frameGeneratorThread->start();

        frameGrabberThread = new QThread(this);
        frameGrabber->moveToThread(frameGrabberThread);
        frameGrabberThread->start();

        scriptEditor = new vfg::ScriptEditor;

        QSettings cfg("config.ini", QSettings::IniFormat);

        QString dgIndexPath = cfg.value("dgindexexecpath").toString();
        dvdProcessor = new vfg::DvdProcessor(dgIndexPath, this);

        const unsigned maxThumbnails = cfg.value("maxthumbnails").toInt();
        ui->unsavedWidget->setMaxThumbnails(maxThumbnails);

        const unsigned numScreenshots = cfg.value("numscreenshots").toInt();
        ui->screenshotsSpinBox->setValue(numScreenshots);

        const unsigned frameStep = cfg.value("framestep").toInt();
        ui->frameStepSpinBox->setValue(frameStep);
    }
    catch(std::exception& ex)
    {
        throw;
    }

    connect(dvdProcessor, SIGNAL(finished(QString)),
            this, SLOT(loadFile(QString)));

    connect(dvdProcessor, SIGNAL(error(QString)),
            this, SLOT(videoError(QString)));

    connect(ui->unsavedWidget, SIGNAL(maximumChanged(int)),
            ui->unsavedProgressBar, SLOT(setMaximum(int)));

    connect(scriptEditor, SIGNAL(scriptUpdated()),
            this, SLOT(scriptEditorUpdated()));

    connect(frameGrabber, SIGNAL(videoReady()),
            this, SLOT(videoLoaded()),
            Qt::QueuedConnection);

    connect(frameGrabber, SIGNAL(errorOccurred(QString)),
            this, SLOT(videoError(QString)),
            Qt::QueuedConnection);

    connect(frameGrabber, SIGNAL(frameGrabbed(QPair<uint,QImage>)),
            ui->videoFrameWidget, SLOT(setFrame(QPair<uint,QImage>)));

    connect(frameGenerator, SIGNAL(frameReady(QPair<unsigned, QImage>)),
            this, SLOT(frameReceived(QPair<unsigned, QImage>)),
            Qt::QueuedConnection);

    connect(ui->unsavedWidget, SIGNAL(thumbnailDoubleClicked(unsigned)),
            this, SLOT(thumbnailDoubleClicked(unsigned)));

    connect(ui->savedWidget, SIGNAL(thumbnailDoubleClicked(unsigned)),
            this, SLOT(thumbnailDoubleClicked(unsigned)));
}

MainWindow::~MainWindow()
{
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
        if(frameGrabberThread->isRunning())
        {
            frameGrabberThread->quit();
            frameGrabberThread->wait();
        }

        if(frameGeneratorThread->isRunning())
        {
            frameGenerator->stop();
            frameGeneratorThread->quit();
            frameGeneratorThread->wait();
        }

        // Close script editor if it's open
        scriptEditor->close();

        ev->accept();
    }
    else
    {
        ev->ignore();
    }
}

void MainWindow::frameReceived(QPair<unsigned, QImage> frame)
{
    qDebug() << "FRAME_RECEIVED in thread" << qApp->thread()->currentThreadId() << frame.first;
    const unsigned thumbnailSize = ui->thumbnailSizeSlider->value();
    QPixmap thumbnail = QPixmap::fromImage(frame.second).scaledToWidth(200, Qt::SmoothTransformation);
    vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(frame.first, thumbnail);
    thumb->setFixedWidth(thumbnailSize);

    connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(handleUnsavedMenu(QPoint)));

    // Update widgets
    ui->unsavedWidget->addThumbnail(thumb);
    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());

    // TODO: See if it's possible to implement signals to the generator
    // and get rid of all this code here
    QSettings cfg("config.ini", QSettings::IniFormat);
    if(frameGenerator->isPaused())
    {
        // Jump to last generated frame
        const bool jumpAfterPaused = cfg.value("jumptolastonpause").toBool();
        if(jumpAfterPaused) {
            ui->seekSlider->setValue(frame.first);
        }
    }

    // If true, implies that generator has been explicitly stopped,
    // otherwise is still running or has finished
    const bool generatorStopped = ui->generatorProgressBar->value() == 0 && !frameGenerator->isRunning();
    if(generatorStopped) {
        // Jump to last generated frame
        const bool jumpAfterStopped = cfg.value("jumptolastonstop").toBool();
        if(jumpAfterStopped) {
            ui->seekSlider->setValue(frame.first);
        }
    }
    else {
        const unsigned generated = ui->generatorProgressBar->value() + 1;
        ui->generatorProgressBar->setValue(generated);

        const bool generatorFinished = frameGenerator->remaining() == 0;
        if(generatorFinished)
        {
            // Generator has finished without explicit stopping
            ui->btnPauseGenerator->setEnabled(false);
            ui->btnStopGenerator->setEnabled(false);

            // Jump to last generated frame
            const bool jumpAfterFinished = cfg.value("jumptolastonfinish").toBool();
            if(jumpAfterFinished) {
                ui->seekSlider->setValue(frame.first);
            }
        }
    }

    if(ui->unsavedWidget->isFull())
    {
        const bool pauseAfterLimit = cfg.value("pauseafterlimit").toBool();
        if(pauseAfterLimit)
        {
            //frameGenerator->pause();
            //ui->btnPauseGenerator->setText(tr("Resume"));
        }
        else
        {
            frameGenerator->fetchNext();
        }
    }
    else {
        frameGenerator->fetchNext();
    }
}

void MainWindow::resetState()
{
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

        QString parsedScript = parser->parse();

        // TODO: Save to filepath if user wants
        QString saveTo = "temp.avs";
        vfg::script::save(saveTo, parsedScript);

        QFileInfo info(path);
        setWindowTitle(tr("ScreenPicker - %1").arg(info.fileName()));

        // Load the (parsed) Avisynth script to script editor
        scriptEditor->setContent(saveTo);

        // Create Avisynth video source and attempt to load the (parsed) Avisynth script
        QSharedPointer<vfg::AvisynthVideoSource> videoSource(new vfg::AvisynthVideoSource);
        videoSource->load(saveTo);

        // TODO: Stop screenshot generation if that's happening...
        frameGrabber->setVideoSource(videoSource);

        // Reset all states back to zero
        lastRequestedFrame = vfg::FirstFrame;
        resetState();

        // Load config
        QSettings cfg("config.ini", QSettings::IniFormat);
        const bool showEditor = cfg.value("showscripteditor").toBool();
        if(showEditor)
        {
            scriptEditor->show();
            scriptEditor->setWindowState(Qt::WindowActive);
        }
    }
    catch(std::exception& ex)
    {
        QMessageBox::warning(this, tr("Error while loading file"),
                             QString(ex.what()));

        scriptEditor->show();
        scriptEditor->setWindowState(Qt::WindowActive);
    }
}

void MainWindow::on_actionOpen_triggered()
{
    if(frameGenerator->isRunning()) {
        frameGenerator->pause();
        ui->btnPauseGenerator->setText(tr("Resume"));
    }

    QString filename = QFileDialog::getOpenFileName(this, tr("Open video"),
                                                    "", "All (*.*);;Avisynth (*.avs, *.avsi);;DGIndex (*.d2v)");
    if(filename.isEmpty())
        return;

    loadFile(filename);
}

void MainWindow::on_actionOpen_DVD_triggered()
{
    if(frameGenerator->isRunning()) {
        frameGenerator->pause();
        ui->btnPauseGenerator->setText(tr("Resume"));
    }

    QStringList vobFiles = QFileDialog::getOpenFileNames(this, tr("Select DVD VOB/Blu-ray M2TS files"),
                                                         "", "DVD VOB (*.vob);;Blu-ray M2TS (*.m2ts)");
    if(vobFiles.empty())
    {
        return;
    }

    QSettings cfg("config.ini", QSettings::IniFormat);
    QString dgIndexPath = cfg.value("dgindexexecpath").toString();
    if(!QFile::exists(dgIndexPath))
    {
        QMessageBox::critical(this, tr("DGIndex invalid path"), tr("Please set a valid path to DGIndex"));
        ui->actionOptions->trigger();
        return;
    }

    QString dgIndexOutPath = QDir::currentPath().append("/dgindex_tmp");

    dvdProcessor->process(vobFiles, dgIndexOutPath);
}

void MainWindow::scriptEditorUpdated()
{
    try
    {
        // Create Avisynth video source and attempt to load the (parsed) Avisynth script
        QSharedPointer<vfg::AvisynthVideoSource> videoSource(new vfg::AvisynthVideoSource);
        videoSource->load(path);

        frameGrabber->setVideoSource(videoSource);
    }
    catch(std::exception& ex)
    {
        QMessageBox::warning(this, tr("Error while loading script"),
                             QString(ex.what()));
        scriptEditor->show();
        scriptEditor->setWindowState(Qt::WindowActive);
    }
}

void MainWindow::videoLoaded()
{
    const unsigned numFrames = frameGrabber->totalFrames();

    const bool invalidRange = !frameGrabber->validRange(lastRequestedFrame);
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
                              Qt::QueuedConnection, Q_ARG(unsigned, lastRequestedFrame));

    // Enable buttons
    ui->previousButton->setEnabled(true);
    ui->nextButton->setEnabled(true);
    ui->grabButton->setEnabled(true);
    ui->generateButton->setEnabled(true);
    ui->saveSingleButton->setEnabled(true);
}

void MainWindow::videoError(QString msg)
{
    QMessageBox::warning(this, tr("Video error"), msg);
}

void MainWindow::thumbnailDoubleClicked(unsigned frameNumber)
{    
    QMetaObject::invokeMethod(frameGrabber, "requestFrame",
                              Qt::QueuedConnection, Q_ARG(unsigned, frameNumber));
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

void MainWindow::on_originalResolutionCheckBox_toggled(bool checked)
{
    ui->videoFrameWidget->setFullsize(checked);
}


void MainWindow::on_seekSlider_valueChanged(int value)
{
    const unsigned frameNumber = static_cast<unsigned>(value);
    if(lastRequestedFrame == frameNumber)
    {
        return;
    }

    QMetaObject::invokeMethod(frameGrabber, "requestFrame",
                              Qt::QueuedConnection, Q_ARG(unsigned, frameNumber));
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
    // TODO: container isFull() check
    QSettings cfg("config.ini", QSettings::IniFormat);
    const bool pauseAfterLimit = cfg.value("pauseafterlimit").toBool();
    if(pauseAfterLimit && ui->unsavedWidget->isFull()) {
        // If user has chosen to pause the generator after reaching
        // maximum limit and has clicked resume (this path) while
        // the container is still full we can't resume
        QMessageBox::information(this, tr(""), tr("Can't start generator while the container has reached max limit.\n"
                                                  "Click 'Clear' or raise the max thumbnail limit to continue."));
        return;
    }

    // Compute list of frame numbers to grab
    const unsigned selected_frame = ui->seekSlider->value();
    const unsigned frame_step = ui->frameStepSpinBox->value();
    const unsigned num_generate = ui->screenshotsSpinBox->value();
    const unsigned unlimited_screens = ui->cbUnlimitedScreens->isChecked();
    const unsigned total_video_frames = frameGrabber->totalFrames();
    const unsigned total_frame_range = frame_step * num_generate;
    const unsigned last_frame = selected_frame + total_frame_range;

    if(frameGenerator->isRunning() || frameGenerator->isPaused()) {
        frameGenerator->stop();
    }

    for(unsigned current_frame = selected_frame; ; current_frame += frame_step)
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

    const unsigned remaining = frameGenerator->remaining();

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
    const unsigned selected = ui->seekSlider->value();      
    const unsigned thumbnailSize = ui->thumbnailSizeSlider->value();
    QImage frame = frameGrabber->getFrame(selected);
    QPixmap thumbnail = QPixmap::fromImage(frame).scaledToWidth(200, Qt::SmoothTransformation);

    vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(selected, thumbnail);
    thumb->setFixedWidth(thumbnailSize);
    connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(handleSavedMenu(QPoint)));
    ui->savedWidget->addThumbnail(thumb);
    framesToSave.append(thumb->frameNum());


    statusBar()->showMessage(tr("Grabbed frame #%1").arg(selected), 3000);
}

void MainWindow::handleUnsavedMenu(const QPoint &pos)
{
    QMenu menu;
    QAction *saveAction = new QAction(tr("Save"), this);
    saveAction->setData(1);
    menu.addAction(saveAction);

    QAction* selected = menu.exec(QCursor::pos());
    if(selected && selected->data().toInt() == 1)
    {
        // Move thumbnail from unsaved tab to saved tab
        vfg::VideoFrameThumbnail* thumb = ui->unsavedWidget->takeSelected();
        disconnect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(handleUnsavedMenu(QPoint)));
        connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(handleSavedMenu(QPoint)));

        framesToSave.append(thumb->frameNum());

        ui->savedWidget->addThumbnail(thumb);
        ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());
    }
}

void MainWindow::handleSavedMenu(const QPoint &pos)
{
    QMenu menu;
    QAction *unsaveAction = new QAction(tr("Unsave"), this);
    unsaveAction->setData(1);
    menu.addAction(unsaveAction);

    QAction* selected = menu.exec(QCursor::pos());
    if(selected && selected->data().toInt() == 1)
    {
        // Move thumbnail from saved tab to unsaved tab
        vfg::VideoFrameThumbnail* thumb = ui->savedWidget->takeSelected();
        disconnect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(handleSavedMenu(QPoint)));
        connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(handleUnsavedMenu(QPoint)));

        framesToSave.removeOne(thumb->frameNum());

        ui->unsavedWidget->addThumbnail(thumb);
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

    QString dir = QFileDialog::getExistingDirectory(this, tr("Select save directory"));
    if(dir.isEmpty())
    {
        return;
    }

    bool resizeOk = false;
    int resizeWidth = 0;
    QMessageBox::StandardButton clicked = QMessageBox::question(this, tr("Resize thumbnails"),
                                                                tr("Do you want to resize thumbnails?"),
                                                                QMessageBox::Yes | QMessageBox::No,
                                                                QMessageBox::No);
    if(clicked == QMessageBox::Yes)
    {
        QSize frameSize = ui->videoFrameWidget->getFrameSize();
        const int resizeTo = frameSize.width() / 2;
        resizeWidth = QInputDialog::getInt(this, tr("Resize thumbnails"), tr("Resize to width:"),
                                           resizeTo, 100, frameSize.width(), 10, &resizeOk);
    }

    const int numSaved = framesToSave.count();
    QProgressDialog prog("", "Cancel", 0, numSaved, this);
    prog.setWindowModality(Qt::WindowModal);
    prog.setCancelButton(0);
    prog.setMinimumDuration(0);
    QDir saveDir(dir);
    QListIterator<unsigned> iter(framesToSave);
    while(iter.hasNext())
    {
        const unsigned frameNumber = iter.next();
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
        if(resizeOk)
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
    loadFile(filename);
}

void MainWindow::on_actionOptions_triggered()
{
    vfg::ConfigDialog configDialog;
    const int saved = configDialog.exec();

    if(saved)
    {
        QSettings cfg("config.ini", QSettings::IniFormat);
        ui->unsavedWidget->setMaxThumbnails(cfg.value("maxthumbnails").toInt());
        dvdProcessor->setProcessor(cfg.value("dgindexexecpath").toString());
    }
}

void MainWindow::on_screenshotsSpinBox_valueChanged(int arg1)
{
    QSettings cfg("config.ini", QSettings::IniFormat);
    cfg.setValue("numscreenshots", arg1);

    ui->screenshotsSpinBox->setValue(arg1);
}

void MainWindow::on_frameStepSpinBox_valueChanged(int arg1)
{
    QSettings cfg("config.ini", QSettings::IniFormat);
    cfg.setValue("framestep", arg1);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, tr("About"), tr("ScreenPicker 1.0b2 20130122"));
}

void MainWindow::on_saveSingleButton_clicked()
{
    const unsigned selected = ui->seekSlider->value();
    QImage frame = frameGrabber->getFrame(selected);

    QString outFilename = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                                       QString("%1.png").arg(QString::number(selected)),
                                                       tr("PNG (*.png)"));
    if(QFile::exists(outFilename))
    {
        QMessageBox::StandardButton clicked = QMessageBox::question(this, tr("Overwrite?"),
                                                                    tr("File %1 exists. Overwrite?").arg(outFilename),
                                                                    QMessageBox::Yes | QMessageBox::No,
                                                                    QMessageBox::No);
        if(clicked == QMessageBox::Yes)
        {
            frame.save(outFilename);
        }
    }
    else
    {
        frame.save(outFilename);
    }
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
        QSettings cfg("config.ini", QSettings::IniFormat);
        const bool pauseAfterLimit = cfg.value("pauseafterlimit").toBool();
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
