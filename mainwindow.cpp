#include <QtGui>
#include "flowlayout.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videoframegrabber.h"
#include "videoframewidget.h"
#include "videoframethumbnail.h"
#include "ffms.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    frameGrabber = new vfg::VideoFrameGrabber(this);
    connect(frameGrabber, SIGNAL(videoReady(const FFMS_VideoProperties*)),
            this, SLOT(videoLoaded(const FFMS_VideoProperties*)));
    connect(frameGrabber, SIGNAL(errorOccurred(QString)),
            this, SLOT(videoError(QString)));

    frameWidget = new vfg::VideoFrameWidget(this);
    ui->videoFrameArea->setWidget(frameWidget);

    // Connect grabber to the widget
    connect(frameGrabber, SIGNAL(frameGrabbed(QImage)),
            frameWidget, SLOT(setFrame(QImage)));

    // Widgets/layouts for the tabs
    unsavedLayout = new FlowLayout;
    savedLayout = new FlowLayout;

    ui->unsavedWidget->setLayout(unsavedLayout);
    ui->savedWidget->setLayout(savedLayout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open video"));
    if(filename.isEmpty())
        return;

    ui->logger->clear();
    ui->logger->appendPlainText(tr("Loading file... %1").arg(filename));
    frameGrabber->load(filename);
}

void MainWindow::videoLoaded(const FFMS_VideoProperties *videoProps)
{
    ui->logger->appendPlainText(tr("Number of frames: %1")
                            .arg(videoProps->NumFrames));
    ui->logger->appendPlainText(tr("Start / End time: %1 %2")
                            .arg(videoProps->FirstTime)
                            .arg(videoProps->LastTime));
    ui->logger->appendPlainText(tr("FPS: %1 / %2")
                            .arg(videoProps->FPSNumerator)
                            .arg(videoProps->FPSDenominator));
    ui->logger->appendPlainText(tr("SAR (num / den): %1 / %2")
                            .arg(videoProps->SARNum)
                            .arg(videoProps->SARDen));
    ui->logger->appendPlainText(tr("Crop: Bottom: %1 Left: %2 Right: %3 Top: %4")
                            .arg(videoProps->CropBottom)
                            .arg(videoProps->CropLeft)
                            .arg(videoProps->CropRight)
                            .arg(videoProps->CropTop));

    const unsigned numFrames = videoProps->NumFrames;
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
        const FFMS_Frame* frame = frameGrabber->getFrame(vfg::FirstFrame);
        if(frame == NULL)
            return;

        frameWidget->setFullsize(true);
        ui->logger->appendPlainText(tr("Resolution locked to %1x%2")
                                .arg(frame->ScaledHeight)
                                .arg(frame->ScaledWidth));
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
            QImage frame = vfg::convertToQImage(frameGrabber->getFrame(i));

            vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(this);
            thumb->setThumbnail(QPixmap::fromImage(frame));
            unsaved.insert(i, thumb);
            unsavedLayout->addWidget(thumb);
        }
    }
    ui->seekSlider->setValue(selected + total);
}

void MainWindow::on_grabButton_clicked()
{
    const unsigned selected = ui->seekSlider->value();
    if(!unsaved.contains(selected))
    {        
        QImage frame = vfg::convertToQImage(frameGrabber->getFrame(selected));

        vfg::VideoFrameThumbnail* thumb = new vfg::VideoFrameThumbnail(this);
        thumb->setThumbnail(QPixmap::fromImage(frame));
        unsaved.insert(selected, thumb);
        unsavedLayout->addWidget(thumb);
    }
}
