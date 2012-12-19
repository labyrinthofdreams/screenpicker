#include <QtGui>
#include <exception>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videoframegrabber.h"
#include "videoframethumbnail.h"
#include "avisynthvideosource.h"
#include "scripteditor.h"
#include "configdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    framesToSave()
{
    ui->setupUi(this);

    try
    {
         vfg::AvisynthVideoSource* avs = new vfg::AvisynthVideoSource;
         frameGrabber = new vfg::VideoFrameGrabber(avs, this);

         scriptEditor = new vfg::ScriptEditor;
         connect(scriptEditor, SIGNAL(scriptUpdated(QString)),
                 this, SLOT(loadFromAvisynthScript(QString)));

         createAvisynthScriptFile();
         createConfig();
    }
    catch(std::exception& ex)
    {
        throw;
    }

    connect(frameGrabber, SIGNAL(videoReady()),
            this, SLOT(videoLoaded()));
    connect(frameGrabber, SIGNAL(errorOccurred(QString)),
            this, SLOT(videoError(QString)));

    // Connect grabber to the widget
    connect(frameGrabber, SIGNAL(frameGrabbed(QImage)),
            ui->videoFrameWidget, SLOT(setFrame(QImage)));

    connect(ui->unsavedWidget, SIGNAL(thumbnailDoubleClicked(vfg::VideoFrameThumbnail*)),
            this, SLOT(thumbnailDoubleClicked(vfg::VideoFrameThumbnail*)));
}

MainWindow::~MainWindow()
{
    delete ui;

    if(scriptEditor)
        delete scriptEditor;
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
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open video"),
                                                    "", "All (*.*);;Avisynth (*.avs, *.avsi);;DGIndex (*.d2v)");
    if(filename.isEmpty())
        return;

    try
    {
        ui->unsavedWidget->clearThumbnails();
        ui->savedWidget->clearThumbnails();

        scriptEditor->load(filename);

        QSettings cfg("config.ini", QSettings::IniFormat);
        bool showEditor = cfg.value("showscripteditor").toBool();
        if(showEditor)
            scriptEditor->show();
    }
    catch(std::exception& ex)
    {
        QMessageBox::warning(this, tr("Error while loading file"),
                             QString(ex.what()));
    }
}

void MainWindow::loadFromAvisynthScript(QString path)
{
    try
    {
        frameGrabber->load(path);
    }
    catch(std::exception& ex)
    {
        QMessageBox::warning(this, tr("Error while loading file"),
                             QString(ex.what()));
    }
}

void MainWindow::videoLoaded()
{
    const unsigned numFrames = frameGrabber->totalFrames();
    // Update frame numbers on the labels
    ui->currentFrameLabel->setText(QString::number(vfg::FirstFrame));
    ui->totalFramesLabel->setText(QString::number(numFrames));
    // Update maximum for seek slider
    ui->seekSlider->setEnabled(true);
    ui->seekSlider->setMaximum(numFrames);
    // Move slider back to first frame
    ui->seekSlider->setValue(vfg::FirstFrame);
    // Show first frame
    frameGrabber->requestFrame(vfg::FirstFrame);

    // Enable buttons
    ui->previousButton->setEnabled(true);
    ui->nextButton->setEnabled(true);
    ui->grabButton->setEnabled(true);
    ui->generateButton->setEnabled(true);
}

void MainWindow::videoError(QString msg)
{
    ui->logger->appendPlainText(msg);
}

void MainWindow::thumbnailDoubleClicked(vfg::VideoFrameThumbnail *thumbnail)
{
    ui->seekSlider->setValue(thumbnail->frameNum());
    frameGrabber->requestFrame(thumbnail->frameNum());
}

void MainWindow::on_nextButton_clicked()
{
    frameGrabber->requestNextFrame();
    const unsigned current = frameGrabber->lastFrame();
    ui->currentFrameLabel->setText(QString::number(current));
    ui->seekSlider->setValue(current);
}

void MainWindow::on_previousButton_clicked()
{
    frameGrabber->requestPreviousFrame();
    const unsigned current = frameGrabber->lastFrame();
    ui->currentFrameLabel->setText(QString::number(current));
    ui->seekSlider->setValue(current);
}

void MainWindow::on_originalResolutionCheckBox_toggled(bool checked)
{
    ui->videoFrameWidget->setFullsize(checked);
    if(checked)
    {
        if(frameGrabber->hasVideo())
        {
            QImage frame = frameGrabber->getFrame(vfg::FirstFrame);

            ui->logger->appendPlainText(tr("Resolution locked to %1x%2")
                                    .arg(frame.height())
                                    .arg(frame.width()));
        }
        else
        {
            ui->logger->appendPlainText(tr("Resolution locked"));
        }
    }
    else
    {
        ui->logger->appendPlainText(tr("Resolution unlocked"));
    }
}


void MainWindow::on_seekSlider_valueChanged(int value)
{
    frameGrabber->requestFrame(value);
    ui->currentFrameLabel->setText(QString::number(value));
}

void MainWindow::on_seekSlider_sliderMoved(int position)
{
    ui->currentFrameLabel->setText(QString::number(position));
}

void MainWindow::on_generateButton_clicked()
{
    const unsigned selected = ui->seekSlider->value();
    const unsigned step = ui->frameStepSpinBox->value();
    const unsigned num = ui->screenshotsSpinBox->value();
    const unsigned total = step * num;
    const unsigned lastPos = selected + total;
    const unsigned totalFrames = frameGrabber->totalFrames();
    const unsigned thumbnailSize = ui->thumbnailSizeSlider->value();
    QProgressDialog progress("", tr("Cancel"), 0, num, this);
    progress.setMinimumDuration(0);
    progress.setWindowModality(Qt::WindowModal);
    unsigned lastProcessed = selected;
    QList<vfg::VideoFrameThumbnail*> frames;
    for(unsigned i = selected, j = 1; i < lastPos; i += step, ++j, lastProcessed += step)
    {
        if(i > totalFrames)
        {
            break;
        }

        progress.setLabelText(tr("Generating image %1 of %2").arg(j).arg(num));
        progress.setValue(j);
        if(progress.wasCanceled())
        {
            break;
        }

        QImage frame = frameGrabber->getFrame(i);
        QPixmap thumbnail = QPixmap::fromImage(frame).scaledToWidth(200, Qt::SmoothTransformation);

        vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(i, thumbnail);
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
    ui->seekSlider->setValue(lastProcessed);
    ui->numUnsavedScreensLabel->setText(QString::number(ui->unsavedWidget->numThumbnails()));
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
            this, SLOT(handleUnsavedMenu(QPoint)));
    ui->unsavedWidget->addThumbnail(thumb);
    ui->numUnsavedScreensLabel->setText(QString::number(ui->unsavedWidget->numThumbnails()));
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

        // Move unsaved cached QImage to saved cache
//        QImage tmpUnsaved = unsaved.take(thumb->frameNum());
//        saved.insert(thumb->frameNum(), tmpUnsaved);
        framesToSave.insert(thumb->frameNum());

        ui->savedWidget->addThumbnail(thumb);
        ui->numUnsavedScreensLabel->setText(QString::number(ui->unsavedWidget->numThumbnails()));
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

        // Move saved cached QImage to unsaved cache
//        QImage tmpSaved = saved.take(thumb->frameNum());
//        unsaved.insert(thumb->frameNum(), tmpSaved);
        framesToSave.remove(thumb->frameNum());

        ui->unsavedWidget->addThumbnail(thumb);
        ui->numUnsavedScreensLabel->setText(QString::number(ui->unsavedWidget->numThumbnails()));
    }
}

void MainWindow::on_clearThumbsButton_clicked()
{
    ui->unsavedWidget->clearThumbnails();
    ui->numUnsavedScreensLabel->setText(QString::number(ui->unsavedWidget->numThumbnails()));
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
    int resizeWidth;
    QMessageBox::StandardButton clicked = QMessageBox::question(this, tr("Resize thumbnails"),
                                                                tr("Do you want to resize thumbnails?"),
                                                                QMessageBox::Yes, QMessageBox::No);
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
//    if(!scriptEditor->loadFile(avisynthScriptFile))
//    {
//        QMessageBox::critical(this, tr("Avisynth Script Editor"),
//                              tr("Failed to open default.avs, make sure default.avs exists in the app directory"));
//        return;
//    }

    scriptEditor->show();
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

    try
    {
        ui->unsavedWidget->clearThumbnails();
        ui->savedWidget->clearThumbnails();

        QString filename = urls.at(0).toLocalFile();
        scriptEditor->load(filename);

        QSettings cfg("config.ini", QSettings::IniFormat);
        bool showEditor = cfg.value("showscripteditor").toBool();
        if(showEditor)
            scriptEditor->show();
    }
    catch(std::exception& ex)
    {
        QMessageBox::warning(this, tr("Error while loading file"),
                             QString(ex.what()));
    }
}

void MainWindow::on_actionOptions_triggered()
{
    vfg::ConfigDialog configDialog;
    configDialog.exec();
}
