#include <QtGui>
#include <QtCore>
#include <QtConcurrentMap>
#include <stdexcept>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videoframegrabber.h"
#include "videoframethumbnail.h"
#include "avisynthvideosource.h"
#include "scripteditor.h"
#include "configdialog.h"

// TODO: When loading new video, the widgets inside scroll areas
// do not get resized to fit the scroll area (they remain large)
// TODO: Project files to save progress?
// TODO: Pausing and resuming screenshot generation + progress display
// TODO: Add parsers (loading automation) for common video formats to reduce scripting
// TODO: Configuration options for screen generation (unlimited, generate until hit limit, generate max)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    haltFrameGrabberThread(false),
    framesToSave(),
    lastRequestedFrame(vfg::FirstFrame)
{
    ui->setupUi(this);

    try
    {
        frameGrabber = new vfg::VideoFrameGrabber;

        frameGrabberThread = new QThread(this);
        frameGrabber->moveToThread(frameGrabberThread);
        frameGrabberThread->start();

        scriptEditor = new vfg::ScriptEditor;

        connect(ui->unsavedWidget, SIGNAL(maximumChanged(int)),
                ui->unsavedProgressBar, SLOT(setMaximum(int)));

        QSettings cfg("config.ini", QSettings::IniFormat);
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

    connect(scriptEditor, SIGNAL(scriptUpdated(QString)),
            this, SLOT(scriptEditorUpdated(QString)));

    connect(frameGrabber, SIGNAL(videoReady()),
            this, SLOT(videoLoaded()),
            Qt::QueuedConnection);

    connect(frameGrabber, SIGNAL(errorOccurred(QString)),
            this, SLOT(videoError(QString)),
            Qt::QueuedConnection);

    connect(frameGrabber, SIGNAL(frameGrabbed(QPair<unsigned, QImage>)),
            this, SLOT(onFrameGrabbed(QPair<unsigned, QImage>)),
            Qt::QueuedConnection);

    // Connect grabber to the widget
//    connect(frameGrabber.data(), SIGNAL(frameGrabbed(QImage)),
//            ui->videoFrameWidget, SLOT(setFrame(QImage)));

    connect(ui->unsavedWidget, SIGNAL(thumbnailDoubleClicked(unsigned)),
            this, SLOT(thumbnailDoubleClicked(unsigned)));

    connect(ui->savedWidget, SIGNAL(thumbnailDoubleClicked(unsigned)),
            this, SLOT(thumbnailDoubleClicked(unsigned)));
}

MainWindow::~MainWindow()
{
    delete scriptEditor;
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
            haltFrameGrabberThread = true;
            frameGrabberThread->quit();
            frameGrabberThread->wait();
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

void MainWindow::onFrameGrabbed(QPair<unsigned, QImage> frame)
{
    QMutexLocker mtx(&frameReceivedMtx);
    qDebug() << "FRAME_RECEIVED in thread" << qApp->thread()->currentThreadId() << frame.first;
    const unsigned thumbnailSize = ui->thumbnailSizeSlider->value();
    QPixmap thumbnail = QPixmap::fromImage(frame.second).scaledToWidth(200, Qt::SmoothTransformation);
    vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(frame.first, thumbnail);
    thumb->setFixedWidth(thumbnailSize);

    connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(handleUnsavedMenu(QPoint)));
    ui->unsavedWidget->addThumbnail(thumb);
    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());

    if(haltFrameGrabberThread)
    {
        return;
    }

    if(!framesToSave.isEmpty())
    {
        const unsigned nextFrame = framesToSave.takeFirst();
        qDebug() << qApp->thread()->currentThreadId() << framesToSave.size();

        mtx.unlock();
        //qDebug() << "From main, framegrabber thread is" << frameGrabber->thread()->currentThreadId();
        QMetaObject::invokeMethod(frameGrabber,
                                  "requestFrame",
                                  Qt::QueuedConnection,
                                  Q_ARG(unsigned, nextFrame));
    }
    else
    {
        // No more frames to process
        // TODO: Add an option for user to jump to last grabbed frame
    }
    //qDebug() << "END FRAME_RECEIVED";
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
}

void MainWindow::loadFile(QString path)
{
    try
    {
        QString savedPath = path;
        QFileInfo info(path);
        if(info.suffix() != "avs" && info.suffix() != "avsi")
        {
            savedPath = "temp.avs";

            // Parse and save Avisynth script
            QString parsedScript = parseScript(path);
            saveScript(savedPath, parsedScript);
        }

        // Reset all states back to zero
        lastRequestedFrame = vfg::FirstFrame;
        resetState();

        setWindowTitle(tr("ScreenPicker - %1").arg(info.fileName()));

        // Load the (parsed) Avisynth script to script editor
        scriptEditor->load(savedPath);

        // Create Avisynth video source and attempt to load the (parsed) Avisynth script
        QSharedPointer<vfg::AvisynthVideoSource> videoSource(new vfg::AvisynthVideoSource);
        videoSource->load(savedPath);

        // TODO: Stop screenshot generation if that's happening...
        frameGrabber->setVideoSource(videoSource);

        //QMetaObject::invokeMethod(frameGrabber.data(), "load", Q_ARG(QString, savedPath));
        //frameGrabber->load(savedPath);

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

QString MainWindow::parseScript(QString filepath)
{
    QFile scriptfile("default.avs");
    if(!scriptfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw std::runtime_error("Failed to open script");
    }

    QTextStream in(&scriptfile);

    // Load config
    QSettings cfg("config.ini", QSettings::IniFormat);
    QString pluginsPath = cfg.value("avisynthpluginspath").toString();

    // Parse avisynth script
    QString parsedScript = in.readAll();
    parsedScript = parsedScript.arg(filepath).arg(pluginsPath);

    return parsedScript;
}

void MainWindow::saveScript(QString path, QString script)
{
    QFile outFile(path);
    if(!outFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        throw std::runtime_error("Failed to open file for writing.");
    }

    QTextStream out(&outFile);
    out << script;
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open video"),
                                                    "", "All (*.*);;Avisynth (*.avs, *.avsi);;DGIndex (*.d2v)");
    if(filename.isEmpty())
        return;

    loadFile(filename);
}

void MainWindow::scriptEditorUpdated(QString path)
{
    try
    {
        // Create Avisynth video source and attempt to load the (parsed) Avisynth script
        QSharedPointer<vfg::AvisynthVideoSource> videoSource(new vfg::AvisynthVideoSource);
        videoSource->load(path);

        frameGrabber->setVideoSource(videoSource);

        //QMetaObject::invokeMethod(frameGrabber.data(), "load", Q_ARG(QString, path));
        //frameGrabber->load(path);
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
    if(lastRequestedFrame > vfg::FirstFrame && numFrames < lastRequestedFrame)
    {
        // If last requested frame is not FirstFrame, it means
        // that we have reloaded our Avisynth script via the editor.
        // If our reloaded Avisynth script modifies the video
        // in a way that it outputs less frames than where we
        // last were, then go back to first frame
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
    QImage frame = frameGrabber->getFrame(lastRequestedFrame);
    if(!frame.isNull())
    {
        ui->videoFrameWidget->setFrame(frame);
    }

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
    QImage frame = frameGrabber->getFrame(frameNumber);
    if(frame.isNull())
    {
        return;
    }
    lastRequestedFrame = frameNumber;
    ui->videoFrameWidget->setFrame(frame);
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_nextButton_clicked()
{
    qDebug() << "Start Main Thread " << qApp->thread()->currentThreadId();
    QImage frame = frameGrabber->getFrame(lastRequestedFrame + 1);
    if(frame.isNull())
    {
        return;
    }
    lastRequestedFrame++;
    ui->videoFrameWidget->setFrame(frame);
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_previousButton_clicked()
{
    qDebug() << "Main Thread " << qApp->thread()->currentThreadId();
    QImage frame = frameGrabber->getFrame(lastRequestedFrame - 1);
    if(frame.isNull())
    {
        return;
    }
    lastRequestedFrame--;
    ui->videoFrameWidget->setFrame(frame);
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_originalResolutionCheckBox_toggled(bool checked)
{
    ui->videoFrameWidget->setFullsize(checked);
}


void MainWindow::on_seekSlider_valueChanged(int value)
{
    //qDebug() << frameGrabber->lastFrame() << value << lastRequestedFrame;
    if(lastRequestedFrame == value)
    {
        return;
    }

    QImage frame = frameGrabber->getFrame(value);
    ui->videoFrameWidget->setFrame(frame);

    lastRequestedFrame = value;
    ui->currentFrameLabel->setText(QString::number(value));
}

void MainWindow::on_seekSlider_sliderMoved(int position)
{
    //lastRequestedFrame = position;
    ui->currentFrameLabel->setText(QString::number(position));
}

void MainWindow::on_generateButton_clicked()
{
    const unsigned selected_frame = ui->seekSlider->value();
    const unsigned frame_step = ui->frameStepSpinBox->value();
    const unsigned num_generate = ui->screenshotsSpinBox->value();
    const unsigned total_video_frames = frameGrabber->totalFrames();

    const unsigned total_frame_range = frame_step * num_generate;
    const unsigned last_frame = selected_frame + total_frame_range;

    // Compute list of frame numbers to grab
    //QList<const unsigned> frame_numbers;
    for(unsigned current_frame = selected_frame; ; current_frame += frame_step)
    {
        const bool reached_last_frame = current_frame >= last_frame;
        const bool reached_video_end = current_frame > total_video_frames;
        if(reached_last_frame || reached_video_end)
            break;

        //frame_numbers.append(current_frame);
        framesToSave.append(current_frame);
    }

    const unsigned next_frame = framesToSave.takeFirst();
    QMetaObject::invokeMethod(frameGrabber,
                              "requestFrame",
                              Qt::QueuedConnection,
                              Q_ARG(unsigned, next_frame));

    // Last processed frame number
//    unsigned lastProcessed = selected;
//    QList<vfg::VideoFrameThumbnail*> frames;
//    QProgressDialog progress("", tr("Cancel"), 0, num, this);
//    progress.setMinimumDuration(0);
//    progress.setWindowModality(Qt::WindowModal);
//    progress.show();
//    for(unsigned currentFrame = selected, frameCtr = 1;
//        currentFrame < lastPos && currentFrame <= totalFrames;
//        currentFrame += step, ++frameCtr, lastProcessed += step)
//    {
//        progress.setLabelText(tr("Generating image %1 of %2").arg(frameCtr).arg(num));
//        progress.setValue(frameCtr);
//        if(progress.wasCanceled())
//        {
//            break;
//        }

//        QImage frame = frameGrabber->getFrame(currentFrame);
//        QPixmap thumbnail = QPixmap::fromImage(frame).scaledToWidth(200, Qt::SmoothTransformation);

//        vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(currentFrame, thumbnail);
//        thumb->setFixedWidth(thumbnailSize);
//        connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
//                this, SLOT(handleUnsavedMenu(QPoint)));
//        frames.append(thumb);
//    }
//    while(!frames.isEmpty())
//    {
//        ui->unsavedWidget->addThumbnail(frames.takeFirst());
//    }
//    progress.setValue(num);
//    lastRequestedFrame = lastProcessed;
//    ui->seekSlider->setValue(lastRequestedFrame);
//    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());
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
    //framesToSave.append(thumb->frameNum());


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

        //framesToSave.append(thumb->frameNum());

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

        //framesToSave.removeOne(thumb->frameNum());

        ui->unsavedWidget->addThumbnail(thumb);
        ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());
    }
}

void MainWindow::on_clearThumbsButton_clicked()
{
    ui->unsavedWidget->clearThumbnails();
    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());
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
        const int maxThumbnails = cfg.value("maxthumbnails").toInt();
        ui->unsavedWidget->setMaxThumbnails(maxThumbnails);
        ui->screenshotsSpinBox->setValue(maxThumbnails);
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
