#include <QtGui>
#include <QtCore>
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
// TODO: When generating screenshots, let user skip around the video, grab, ...
// can be done by queueing the frames in a stack where you can push_front new frames on the fly

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    frameGrabber(),
    scriptEditor(),
    framesToSave(),
    lastRequestedFrame(vfg::FirstFrame)
{
    ui->setupUi(this);

    try
    {
         QSharedPointer<vfg::AvisynthVideoSource> avs (new vfg::AvisynthVideoSource);
         frameGrabber.reset(new vfg::VideoFrameGrabber(avs));

         scriptEditor.reset(new vfg::ScriptEditor);

         createAvisynthScriptFile();
         createConfig();

         // Set progress bar max
         QSettings cfg("config.ini", QSettings::IniFormat);
         const unsigned maxThumbnails = cfg.value("maxthumbnails").toInt();
         ui->unsavedProgressBar->setMaximum(maxThumbnails);

         // Set max thumbnails for the Unsaved screenshots
         ui->unsavedWidget->setMaxThumbnails(maxThumbnails);

         const unsigned numScreenshots = cfg.value("numscreenshots").toInt();
         ui->screenshotsSpinBox->setValue(numScreenshots);

         const unsigned frameStep = cfg.value("framestep").toInt();
         ui->frameStepSpinBox->setValue(frameStep);        

         if(numScreenshots > maxThumbnails)
         {
             QMessageBox::warning(this, tr("Invalid screenshot value"),
                                  tr("Number of screenshots exceeds max limit. Setting value to max."));
             ui->screenshotsSpinBox->setValue(maxThumbnails);
         }
    }
    catch(std::exception& ex)
    {
        throw;
    }

    connect(scriptEditor.data(), SIGNAL(scriptUpdated(QString)),
            this, SLOT(scriptEditorUpdated(QString)));

    connect(frameGrabber.data(), SIGNAL(videoReady()),
            this, SLOT(videoLoaded()));
    connect(frameGrabber.data(), SIGNAL(errorOccurred(QString)),
            this, SLOT(videoError(QString)));

    // Connect grabber to the widget
    connect(frameGrabber.data(), SIGNAL(frameGrabbed(QImage)),
            ui->videoFrameWidget, SLOT(setFrame(QImage)));

    connect(ui->unsavedWidget, SIGNAL(thumbnailDoubleClicked(vfg::VideoFrameThumbnail*)),
            this, SLOT(thumbnailDoubleClicked(vfg::VideoFrameThumbnail*)));

    connect(ui->savedWidget, SIGNAL(thumbnailDoubleClicked(vfg::VideoFrameThumbnail*)),
            this, SLOT(thumbnailDoubleClicked(vfg::VideoFrameThumbnail*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createAvisynthScriptFile()
{
    QDir appDir(QDir::currentPath());
    QString avisynthScriptFile = appDir.absoluteFilePath("default.avs");

    // Only create the default Avisynth script template file
    // if it doesn't exist to allow the user to change it
    if(QFile::exists(avisynthScriptFile))
        return;

    QFile inFile(":/scripts/default.avs");
    QFile outFile(avisynthScriptFile);

    if(!inFile.open(QFile::ReadOnly | QFile::Text))
        return;

    if(!outFile.open(QFile::WriteOnly | QFile::Truncate))
        return;

    QTextStream in(&inFile);
    QTextStream out(&outFile);
    out << in.readAll();
}

void MainWindow::createConfig()
{
    if(!QFile::exists("config.ini"))
    {
        QSettings cfg("config.ini", QSettings::IniFormat);
        cfg.setValue("avisynthpluginspath", QDir::currentPath().append("/avisynth"));
        cfg.setValue("savescripts", false);
        cfg.setValue("showscripteditor", true);
        cfg.setValue("maxthumbnails", 100);
        cfg.setValue("numscreenshots", 100);
        cfg.setValue("framestep", 100);
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
        frameGrabber->setVideoSource(videoSource);
        frameGrabber->load(savedPath);

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
        frameGrabber->setVideoSource(videoSource);
        frameGrabber->load(path);
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
    frameGrabber->requestFrame(lastRequestedFrame);

    // Enable buttons
    ui->previousButton->setEnabled(true);
    ui->nextButton->setEnabled(true);
    ui->grabButton->setEnabled(true);
    ui->generateButton->setEnabled(true);
    ui->saveSingleButton->setEnabled(true);
}

void MainWindow::videoError(QString msg)
{
    statusBar()->showMessage(msg, 3000);
}

void MainWindow::thumbnailDoubleClicked(vfg::VideoFrameThumbnail *thumbnail)
{
    ui->seekSlider->setValue(thumbnail->frameNum());
    frameGrabber->requestFrame(thumbnail->frameNum());
    lastRequestedFrame = frameGrabber->lastFrame();
}

void MainWindow::on_nextButton_clicked()
{
    frameGrabber->requestNextFrame();
    lastRequestedFrame = frameGrabber->lastFrame();
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_previousButton_clicked()
{
    frameGrabber->requestPreviousFrame();
    lastRequestedFrame = frameGrabber->lastFrame();
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
    ui->seekSlider->setValue(lastRequestedFrame);
}

void MainWindow::on_originalResolutionCheckBox_toggled(bool checked)
{
    ui->videoFrameWidget->setFullsize(checked);
}


void MainWindow::on_seekSlider_valueChanged(int value)
{
    frameGrabber->requestFrame(value);
    lastRequestedFrame = value;
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
}

void MainWindow::on_seekSlider_sliderMoved(int position)
{
    lastRequestedFrame = position;
    ui->currentFrameLabel->setText(QString::number(lastRequestedFrame));
}

void MainWindow::on_generateButton_clicked()
{
    // Currently selected frame
    const unsigned selected = ui->seekSlider->value();
    // Frame step
    const unsigned step = ui->frameStepSpinBox->value();
    // Number of screenshots to generate
    const unsigned num = ui->screenshotsSpinBox->value();
    // Total number of frames to jump forward
    const unsigned total = step * num;
    // Last frame to grab
    const unsigned lastPos = selected + total;
    // Number of frames in the video
    const unsigned totalFrames = frameGrabber->totalFrames();
    const unsigned thumbnailSize = ui->thumbnailSizeSlider->value();
    // Last processed frame number
    unsigned lastProcessed = selected;
    QList<vfg::VideoFrameThumbnail*> frames;
    QProgressDialog progress("", tr("Cancel"), 0, num, this);
    progress.setMinimumDuration(0);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    for(unsigned currentFrame = selected, frameCtr = 1;
        currentFrame < lastPos && currentFrame <= totalFrames;
        currentFrame += step, ++frameCtr, lastProcessed += step)
    {
        progress.setLabelText(tr("Generating image %1 of %2").arg(frameCtr).arg(num));
        progress.setValue(frameCtr);
        if(progress.wasCanceled())
        {
            break;
        }

        QImage frame = frameGrabber->getFrame(currentFrame);
        QPixmap thumbnail = QPixmap::fromImage(frame).scaledToWidth(200, Qt::SmoothTransformation);

        vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(currentFrame, thumbnail);
        thumb->setFixedWidth(thumbnailSize);
        connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(handleUnsavedMenu(QPoint)));
        frames.append(thumb);
    }
    while(!frames.isEmpty())
    {
        ui->unsavedWidget->addThumbnail(frames.takeFirst());
    }
    progress.setValue(num);
    lastRequestedFrame = lastProcessed;
    ui->seekSlider->setValue(lastRequestedFrame);
    ui->unsavedProgressBar->setValue(ui->unsavedWidget->numThumbnails());
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
    framesToSave.insert(thumb->frameNum());

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

        framesToSave.insert(thumb->frameNum());

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

        framesToSave.remove(thumb->frameNum());

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
    QSetIterator<unsigned> iter(framesToSave);
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


void MainWindow::closeEvent(QCloseEvent *ev)
{
    ev->ignore();

    const QMessageBox::StandardButton response =
            QMessageBox::question(this, tr("Quit?"), tr("Are you sure?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);
    if(response == QMessageBox::Yes)
    {
        ev->accept();

        // Close script editor if it's open
        scriptEditor->close();
    }
}

void MainWindow::on_actionOptions_triggered()
{
    vfg::ConfigDialog configDialog;
    const int saved = configDialog.exec();

    if(saved)
    {
        QSettings cfg("config.ini", QSettings::IniFormat);
        const int maxThumbnails = cfg.value("maxthumbnails").toInt();
        ui->unsavedProgressBar->setMaximum(maxThumbnails);
        ui->unsavedWidget->setMaxThumbnails(maxThumbnails);

        if(ui->screenshotsSpinBox->value() > maxThumbnails)
        {
            QMessageBox::warning(this, tr(""), tr("Number of generated screenshots exceeds max limit. Setting number to max."));
            ui->screenshotsSpinBox->setValue(maxThumbnails);
        }
    }
}

void MainWindow::on_screenshotsSpinBox_valueChanged(int arg1)
{
    QSettings cfg("config.ini", QSettings::IniFormat);
    const int maxThumbnails = cfg.value("maxthumbnails").toInt();

    if(arg1 > maxThumbnails)
    {
        QMessageBox::warning(this, tr(""), tr("Number exceeds maximum thumbnails"));
        ui->screenshotsSpinBox->setValue(maxThumbnails);
        cfg.setValue("numscreenshots", maxThumbnails);
    }
    else
    {
        cfg.setValue("numscreenshots", arg1);
    }
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

    QString outFilename = QFileDialog::getSaveFileName(this, tr("Save as..."), "", tr("PNG (*.png)"));
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
