#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMovie>
#include <QRect>
#include <QSettings>
#include <QString>
#include "gifmakerwidget.hpp"
#include "ptrutil.hpp"

namespace {

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

} // namespace

namespace vfg {
namespace ui {

GifMakerWidget::GifMakerWidget(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    ui.comboImageMagick->addItems(imageMagick.childGroups());
    const auto first = ui.comboImageMagick->currentText();
    ui.plainTextPreset->setPlainText(parseImageMagickArgs(first));

    ui.comboGifsicle->addItem(tr("None"));
    ui.comboGifsicle->addItems(gifsicle.childGroups());
}

GifMakerWidget::~GifMakerWidget()
{
    if(preview) {
        const auto fileName = preview->fileName();
        preview.reset();
        QFile::remove(fileName);
    }
}

void GifMakerWidget::showPreview(const QString& path)
{
    if(preview && ui.checkBoxCompare->isChecked()) {
        preview->stop();
        previousPreview.reset(preview.release());
        ui.labelPreviousGif->setMovie(previousPreview.get());
        previousPreview->start();

        ui.checkBoxCompare->setChecked(false);
    }

    preview = vfg::make_unique<QMovie>(path);
    preview->setCacheMode(QMovie::CacheAll);
    ui.labelPreview->setMovie(preview.get());
    preview->start();

    const auto bytes = prettySize(preview->device()->size());
    const auto res = prettyResolution(preview->frameRect());
    ui.labelGifSize->setText(QString("%1 [%2]").arg(bytes).arg(res));

    ui.buttonSave->setEnabled(true);
    ui.checkBoxCompare->setEnabled(true);
}

void GifMakerWidget::updateStartFrame(const int value)
{
    if(value > ui.spinLastFrame->value()) {
        ui.spinLastFrame->setValue(value);
        config.setValue("gif/endframe", value);
    }

    ui.spinStartFrame->setValue(value);
    config.setValue("gif/startframe", value);
    ui.labelTotalFrames->setText(QString::number(totalFrames()));
}

void GifMakerWidget::updateLastFrame(const int value)
{
    if(value < ui.spinStartFrame->value()) {
        ui.spinStartFrame->setValue(value);
        config.setValue("gif/startframe", value);
    }

    ui.spinLastFrame->setValue(value);
    config.setValue("gif/endframe", value);
    ui.labelTotalFrames->setText(QString::number(totalFrames()));
}

int GifMakerWidget::totalFrames() const
{
    return (ui.spinLastFrame->value() - ui.spinStartFrame->value()) /
            (ui.spinSkipFrames->value() + 1);
}

QString GifMakerWidget::parseImageMagickArgs(const QString& argName)
{
    auto args = imageMagick.value(QString("%1/args").arg(argName)).toString();
    args.replace(QString("%delay%"), QString::number(ui.spinFrameDelay->value()));
    return args;
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
    const auto delay = (ui.spinSkipFrames->value() * 4) + 4;
    const auto adjusted = delay <= ui.spinFrameDelay->maximum() ?
                              delay : ui.spinFrameDelay->maximum();
    ui.spinFrameDelay->setValue(adjusted);
}

void GifMakerWidget::on_spinSkipFrames_valueChanged(const int value)
{
    config.setValue("gif/skipframes", value);
    ui.labelTotalFrames->setText(QString::number(totalFrames()));
}

void GifMakerWidget::on_buttonPreviewGif_clicked()
{
    if(totalFrames() == 0) {
        QMessageBox::information(this, tr("Nothing to process"),
                                 tr("Set start/end frame by right-clicking the video"));
        return;
    }

    config.setValue("gif/delay", ui.spinFrameDelay->value());
    config.setValue("gif/skipframes", ui.spinSkipFrames->value());

    auto args = ui.plainTextPreset->toPlainText();
    if(args.isEmpty()) {
        const auto key = ui.comboImageMagick->currentText();
        args = imageMagick.value(QString("%1/args").arg(key)).toString();
    }

    const auto optKey = ui.comboGifsicle->currentText();
    const auto optArgs = optKey == "None" ? "" : gifsicle.value(QString("%1/args").arg(optKey)).toString();
    emit requestPreview(args, optArgs);
}

void GifMakerWidget::on_buttonSave_clicked()
{
    const QDir saveDir(config.value("last_saved_gif_dir").toString());
    const QString savePath = QFileDialog::getSaveFileName(this, tr("Select save path"),
                                                          saveDir.absoluteFilePath(preview->fileName()),
                                                          tr("GIF (*.gif)"));
    if(savePath.isEmpty()) {
        return;
    }

    if(QFile::exists(savePath)) {
        // QFileDialog will ask to overwrite the output file if it exists
        // but the copy operation below will not succeed if it's not removed
        // since QFile::copy does not overwrite existing files
        QFile::remove(savePath);
    }

    if(!QFile::copy(preview->fileName(), savePath)) {
        QMessageBox::critical(this, tr("Saving failed"),
                              tr("Try again. If the problem persists, try a new filename."));

        return;
    }

    const QFileInfo saved(savePath);
    config.setValue("last_saved_gif_dir", saved.absoluteDir().absolutePath());
}

void GifMakerWidget::on_buttonReset_clicked()
{
    ui.spinStartFrame->setValue(0);
    ui.spinLastFrame->setValue(0);
    ui.spinSkipFrames->setValue(0);
    ui.spinFrameDelay->setValue(4);
    ui.comboGifsicle->setCurrentIndex(0);
    ui.labelPreview->clear();
    ui.labelGifSize->setText(tr("n/a"));
    ui.labelTotalFrames->clear();
    ui.buttonSave->setEnabled(false);
}

void GifMakerWidget::on_comboImageMagick_activated(const QString &arg1)
{
    ui.plainTextPreset->setPlainText(parseImageMagickArgs(arg1));
}

void GifMakerWidget::on_spinFrameDelay_valueChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    const auto current = ui.comboImageMagick->currentText();
    ui.plainTextPreset->setPlainText(parseImageMagickArgs(current));
}

} // namespace ui
} // namespace vfg
