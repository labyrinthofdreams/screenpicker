#include <QtGui>
#include <exception>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videoframegrabber.h"
#include "videoframewidget.h"
#include "videoframethumbnail.h"
#include "avisynthvideosource.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    try
    {
         vfg::AvisynthVideoSource* avs = new vfg::AvisynthVideoSource;
         frameGrabber = new vfg::VideoFrameGrabber(avs, this);

         frameWidget = new vfg::VideoFrameWidget(this);
         ui->videoFrameArea->setWidget(frameWidget);
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
            frameWidget, SLOT(setFrame(QImage)));    

    connect(ui->unsavedWidget, SIGNAL(thumbnailDoubleClicked(vfg::VideoFrameThumbnail*)),
            this, SLOT(thumbnailDoubleClicked(vfg::VideoFrameThumbnail*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open video"),
                                                    QString(),
                                                    frameGrabber->getVideoSource()->getSupportedFormats());
    if(filename.isEmpty())
        return;

    try
    {
        frameGrabber->load(filename);
        ui->logger->clear();
        ui->logger->appendPlainText(tr("Loading file... %1").arg(filename));
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
    frameWidget->setFullsize(checked);
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
    for(unsigned i = selected; i < lastPos; i += step)
    {
        if(i > totalFrames)
        {
            break;
        }

        if(!unsaved.contains(i))
        {
            QImage frame = frameGrabber->getFrame(i);
            QPixmap thumbnail = QPixmap::fromImage(frame).scaledToWidth(200);

            vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(i, thumbnail, this);
            connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                    this, SLOT(handleUnsavedMenu(QPoint)));
            //connect(thumb, SIGNAL(selected(uint)), this, SLOT(thumbnailDoubleClicked(uint)));
            unsaved.insert(i, frame);
            ui->unsavedWidget->addThumbnail(thumb);
        }
    }
    ui->seekSlider->setValue(selected + total);
}

void MainWindow::on_grabButton_clicked()
{
    const unsigned selected = ui->seekSlider->value();
    if(!unsaved.contains(selected))
    {        
        QImage frame = frameGrabber->getFrame(selected);
        QPixmap thumbnail = QPixmap::fromImage(frame).scaledToWidth(200);

        vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(selected, thumbnail, this);
        connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(handleUnsavedMenu(QPoint)));
        //connect(thumb, SIGNAL(selected(uint)), this, SLOT(thumbnailDoubleClicked(uint)));
        unsaved.insert(selected, frame);
        ui->unsavedWidget->addThumbnail(thumb);
    }
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
        QImage tmpUnsaved = unsaved.take(thumb->frameNum());
        saved.insert(thumb->frameNum(), tmpUnsaved);

        ui->savedWidget->addThumbnail(thumb);
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
        QImage tmpSaved = saved.take(thumb->frameNum());
        unsaved.insert(thumb->frameNum(), tmpSaved);

        ui->unsavedWidget->addThumbnail(thumb);
    }
}

void MainWindow::on_actionSave_thumbnails_triggered()
{
    // Save saved images to disk
    if(saved.isEmpty())
    {
        QMessageBox::information(this, tr("Save thumbnails..."),
                                 tr("Nothing to save. Add one or more thumbnails to save."));
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
        QImage dummyImage = saved.begin().value();
        int imageWidth = dummyImage.width();

        resizeWidth = QInputDialog::getInt(this, tr("Resize thumbnails"), tr("Resize to width (100-%2):")
                                           .arg(QString::number(imageWidth)),
                                           imageWidth, 100, imageWidth, 10, &resizeOk);
    }

    int numSaved = saved.size();
    QProgressDialog prog("", "", 0, numSaved, this);
    prog.setWindowModality(Qt::WindowModal);
    prog.setCancelButton(0);
    prog.setMinimumDuration(0);
    QDir saveDir(dir);
    QMapIterator<int, QImage> iter(saved);
    while(iter.hasNext())
    {
        iter.next();
        int current = prog.value();
        prog.setLabelText(tr("Saving image %1 of %2").arg(current).arg(numSaved));
        prog.setValue(current + 1);
        if(prog.wasCanceled())
        {
            QMessageBox::warning(this, tr("Saving thumbnails aborted"),
                                 tr("Saved %1 of %2 thumbnails").arg(current).arg(numSaved));
            break;
        }

        QString filename = QString("%1.png").arg(QString::number(iter.key()));
        QString savePath = saveDir.absoluteFilePath(filename);

        QImage saveImage = iter.value();
        if(resizeOk)
        {
            saveImage = saveImage.scaledToWidth(resizeWidth, Qt::SmoothTransformation);
        }
        saveImage.save(savePath, "PNG");
    }
    prog.setValue(numSaved);
}
