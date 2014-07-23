#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMovie>
#include <QRect>
#include <QSettings>
#include <QString>
#include "gifmakerwidget.hpp"
#include "ui_gifmakerwidget.h"

QString prettySize(double size) {
    if(size < 1000) {
        return QString("%1 B").arg(size);
    }
    size /= 1000;
    if(size < 1000) {
        return QString("%1 KB").arg(QString::number(size, 'f', 2));
    }
    size /= 1000;
    return QString("%1 MB").arg(QString::number(size, 'f', 2));
}

QString prettyResolution(const QRect& area) {
    return QString("%1x%2").arg(area.width()).arg(area.height());
}

using vfg::ui::GifMakerWidget;

GifMakerWidget::GifMakerWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GifMakerWidget),
    preview(),
    config("config.ini", QSettings::IniFormat),
    imageMagick("scripts/imagemagick.ini", QSettings::IniFormat),
    gifsicle("scripts/gifsicle.ini", QSettings::IniFormat)
{
    ui->setupUi(this);

    ui->comboImageMagick->addItems(imageMagick.childGroups());
    const auto first = ui->comboImageMagick->currentText();
    const auto args = imageMagick.value(QString("%1/args").arg(first)).toString();
    ui->plainTextPreset->setPlainText(args);

    ui->comboGifsicle->addItem(tr("None"));
    ui->comboGifsicle->addItems(gifsicle.childGroups());
}

GifMakerWidget::~GifMakerWidget()
{
    if(preview) {
        const auto fileName = preview->fileName();
        preview.reset();
        QFile::remove(fileName);
    }

    delete ui;
}

void GifMakerWidget::showPreview(const QString& path)
{
    preview.reset(new QMovie(path));

    ui->labelPreview->setMovie(preview.get());
    preview->start();

    const auto bytes = prettySize(preview->device()->size());
    const auto res = prettyResolution(preview->frameRect());
    ui->labelGifSize->setText(QString("%1 [%2]").arg(bytes).arg(res));

    ui->buttonSave->setEnabled(true);
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

    ui->labelGifSize->setText("n/a");
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

    ui->labelGifSize->setText("n/a");
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

void GifMakerWidget::on_spinSkipFrames_valueChanged(const int value)
{
    Q_UNUSED(value);
    config.setValue("gif/skipframes", value);
    ui->labelTotalFrames->setText(QString::number(totalFrames()));
    ui->labelGifSize->setText("n/a");
}

void vfg::ui::GifMakerWidget::on_buttonPreviewGif_clicked()
{
    if(totalFrames() == 0) {
        QMessageBox::information(this, tr("Nothing to process"),
                                 tr("Set start/end frame by right-clicking the video"));
    }

    config.setValue("gif/delay", ui->spinFrameDelay->value());
    config.setValue("gif/skipframes", ui->spinSkipFrames->value());

    auto args = ui->plainTextPreset->toPlainText();
    if(args.isEmpty()) {
        const auto key = ui->comboImageMagick->currentText();
        args = imageMagick.value(QString("%1/args").arg(key)).toString();
    }

    const auto optKey = ui->comboGifsicle->currentText();
    const auto optArgs = optKey == "None" ? "" : gifsicle.value(QString("%1/args").arg(optKey)).toString();
    emit requestPreview(args, optArgs);
}

void vfg::ui::GifMakerWidget::on_buttonSave_clicked()
{
    const QString savePath = QFileDialog::getSaveFileName(this, tr("Select save path"),
                                                          preview->fileName(), tr("GIF (*.gif)"));
    if(!savePath.isEmpty() &&
            !QFile::copy(preview->fileName(), savePath)) {
        QMessageBox::critical(this, tr("Saving failed"),
                              tr("Try again. If the problem persists, try a new filename."));
    }
}

void vfg::ui::GifMakerWidget::on_buttonReset_clicked()
{
    ui->spinStartFrame->setValue(0);
    ui->spinLastFrame->setValue(0);
    ui->spinSkipFrames->setValue(0);
    ui->spinFrameDelay->setValue(4);
    ui->comboGifsicle->setCurrentIndex(0);
    ui->labelPreview->clear();
    ui->buttonSave->setEnabled(false);
}

void vfg::ui::GifMakerWidget::on_comboImageMagick_activated(const QString &arg1)
{
    ui->plainTextPreset->setPlainText(imageMagick.value(QString("%1/args").arg(arg1)).toString());
}
