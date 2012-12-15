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
    if(checked && frameGrabber->hasVideo())
    {
        QImage frame = frameGrabber->getFrame(vfg::FirstFrame);

        frameWidget->setFullsize(true);
        ui->logger->appendPlainText(tr("Resolution locked to %1x%2")
                                .arg(frame.height())
                                .arg(frame.width()));
    }
    else
    {
        frameWidget->setFullsize(false);
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

//            vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(this);
//            thumb->setThumbnail(QPixmap::fromImage(frame));
//            unsaved.insert(i, thumb);
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
        unsaved.insert(selected, thumb);
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
        // Move unsaved
        vfg::VideoFrameThumbnail* thumb = ui->unsavedWidget->takeSelected();
        disconnect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(handleUnsavedMenu(QPoint)));
        connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(handleSavedMenu(QPoint)));

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
        // If user chooses to unsave a thumbnail,
        // we just remove it because we don't care
        vfg::VideoFrameThumbnail* thumb = ui->savedWidget->takeSelected();
        disconnect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(handleSavedMenu(QPoint)));
        connect(thumb, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(handleUnsavedMenu(QPoint)));

        ui->unsavedWidget->addThumbnail(thumb);
    }
}
