#include <QFile>
#include <QFileDialog>
#include <QMovie>
#include <QSettings>
#include "gifmakerwidget.hpp"
#include "ui_gifmakerwidget.h"

using vfg::ui::GifMakerWidget;

GifMakerWidget::GifMakerWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GifMakerWidget),
    preview(),
    config("config.ini", QSettings::IniFormat)
{
    ui->setupUi(this);
}

GifMakerWidget::~GifMakerWidget()
{
    if(preview) {
        QFile::remove(preview->fileName());
    }

    delete ui;
}

void GifMakerWidget::showPreview(const QString& path)
{
    preview.reset(new QMovie(path));

    ui->labelPreview->setMovie(preview.get());
    preview->start();
}

void GifMakerWidget::updateStartFrame(const int value)
{
    if(value > ui->spinLastFrame->value()) {
        ui->spinLastFrame->setValue(value);
        config.setValue("gif/endframe", value);
    }

    ui->spinStartFrame->setValue(value);
    config.setValue("gif/startframe", value);
    ui->labelTotalFrames->setText(QString::number(totalFrames()));
}

void GifMakerWidget::updateLastFrame(const int value)
{
    if(value < ui->spinStartFrame->value()) {
        ui->spinStartFrame->setValue(value);
        config.setValue("gif/startframe", value);
    }

    ui->spinLastFrame->setValue(value);
    config.setValue("gif/endframe", value);
    ui->labelTotalFrames->setText(QString::number(totalFrames()));
}

int GifMakerWidget::totalFrames() const
{
    return (ui->spinLastFrame->value() - ui->spinStartFrame->value()) /
            (ui->spinSkipFrames->value() + 1);
}

void GifMakerWidget::on_spinStartFrame_valueChanged(const int value)
{
    updateStartFrame(value);
}

void GifMakerWidget::on_spinLastFrame_valueChanged(const int value)
{
    updateLastFrame(value);
}

void GifMakerWidget::on_buttonAutoDelay_clicked()
{
    const auto delay = (ui->spinSkipFrames->value() * 4) + 4;
    const auto adjusted = delay <= ui->spinFrameDelay->maximum() ?
                              delay : ui->spinFrameDelay->maximum();
    ui->spinFrameDelay->setValue(adjusted);
}

void GifMakerWidget::on_buttonBrowse_clicked()
{
    const QString outDir = QFileDialog::getExistingDirectory(this, tr("Select output directory"), "/");
    ui->editOutputDir->setText(outDir);
}

void GifMakerWidget::on_spinSkipFrames_valueChanged(const int value)
{
    Q_UNUSED(value);
    config.setValue("gif/skipframes", value);
    ui->labelTotalFrames->setText(QString::number(totalFrames()));
}

void vfg::ui::GifMakerWidget::on_buttonPreviewGif_clicked()
{
    config.setValue("gif/delay", ui->spinFrameDelay->value());
    ui->labelPreview->setText(tr("Generating preview..."));
    emit requestPreview();
}
