#include <QtGui>
#include "flowlayout.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videoframegrabber.h"
#include "videoframewidget.h"
#include "ffms.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    frameGrabber = new vfg::VideoFrameGrabber(this);
    connect(frameGrabber, SIGNAL(videoReady(const FFMS_VideoProperties*)),
            this, SLOT(videoLoaded(const FFMS_VideoProperties*)));
    connect(frameGrabber, SIGNAL(frameGrabbed(const FFMS_Frame*)),
            this, SLOT(frameReceived(const FFMS_Frame*)));
    connect(frameGrabber, SIGNAL(errorOccurred(QString)),
            this, SLOT(videoError(QString)));

    frameWidget = new vfg::VideoFrameWidget(this);
    ui->videoFrameArea->setWidget(frameWidget);

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

    // Update total frame number
    ui->totalFramesLabel->setText(QString::number(videoProps->NumFrames));
    // Update maximum for seek slider
    ui->seekSlider->setEnabled(true);
    ui->seekSlider->setMaximum(videoProps->NumFrames);
    // Show first frame
    frameGrabber->requestFrame(0);
}

void MainWindow::frameReceived(const FFMS_Frame *frame)
{
    if(frame == NULL)
    {
        qDebug() << "Null image";
        return;
    }

    QImage img = vfg::convertToQImage(frame);

    frameWidget->setFrame(img);
}

void MainWindow::videoError(QString msg)
{
    ui->logger->appendPlainText(msg);
}

void MainWindow::on_nextButton_clicked()
{
    frameGrabber->requestNextFrame();
    ui->currentFrameLabel->setText(QString::number(frameGrabber->lastFrame()));
}

void MainWindow::on_previousButton_clicked()
{
    frameGrabber->requestPreviousFrame();
    ui->currentFrameLabel->setText(QString::number(frameGrabber->lastFrame()));
}

void MainWindow::on_generateButton_clicked()
{

}

void MainWindow::on_originalResolutionCheckBox_toggled(bool checked)
{
    if(checked && frameGrabber->hasVideo())
    {
        const FFMS_Frame* frame = frameGrabber->getFrame(0);
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

void MainWindow::on_grabButton_clicked()
{

}
