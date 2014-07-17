#include <QFileDialog>
#include "gifmakerwidget.hpp"
#include "ui_gifmakerwidget.h"

using vfg::ui::GifMakerWidget;

GifMakerWidget::GifMakerWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GifMakerWidget)
{
    ui->setupUi(this);
}

GifMakerWidget::~GifMakerWidget()
{
    delete ui;
}

void GifMakerWidget::updateStartFrame(const int value)
{
    if(value > ui->spinLastFrame->value()) {
        ui->spinLastFrame->setValue(value);
    }

    ui->spinStartFrame->setValue(value);
    ui->labelTotalFrames->setText(QString::number(totalFrames()));
}

void GifMakerWidget::updateLastFrame(const int value)
{
    if(value < ui->spinStartFrame->value()) {
        ui->spinStartFrame->setValue(value);
    }

    ui->spinLastFrame->setValue(value);
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

void vfg::ui::GifMakerWidget::on_buttonAutoDelay_clicked()
{
    const auto delay = (ui->spinSkipFrames->value() * 4) + 4;
    const auto adjusted = delay <= ui->spinFrameDelay->maximum() ?
                              delay : ui->spinFrameDelay->maximum();
    ui->spinFrameDelay->setValue(adjusted);
}

void vfg::ui::GifMakerWidget::on_buttonBrowse_clicked()
{
    const QString outDir = QFileDialog::getExistingDirectory(this, tr("Select output directory"), "/");
    ui->editOutputDir->setText(outDir);
}

void vfg::ui::GifMakerWidget::on_spinSkipFrames_valueChanged(const int value)
{
    Q_UNUSED(value);
    ui->labelTotalFrames->setText(QString::number(totalFrames()));
}
