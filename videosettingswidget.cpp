#include <QMap>
#include <QString>
#include "videosettingswidget.h"
#include "ui_videosettingswidget.h"

namespace vfg {

VideoSettingsWidget::VideoSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoSettingsWidget),
    prevSettings()
{
    ui->setupUi(this);
}

VideoSettingsWidget::~VideoSettingsWidget()
{
    delete ui;
}

void VideoSettingsWidget::on_cboxDvdResolution_activated(int index)
{
    switch(index)
    {
    case static_cast<int>(Resolution::NTSC_169):
        ui->sboxResizeWidth->setValue(854);
        ui->sboxResizeHeight->setValue(480);
        break;
    case static_cast<int>(Resolution::NTSC_43):
        ui->sboxResizeWidth->setValue(720);
        ui->sboxResizeHeight->setValue(540);
        break;
    case static_cast<int>(Resolution::PAL_169):
        ui->sboxResizeWidth->setValue(1024);
        ui->sboxResizeHeight->setValue(576);
        break;
    case static_cast<int>(Resolution::PAL_43):
        ui->sboxResizeWidth->setValue(768);
        ui->sboxResizeHeight->setValue(576);
        break;
    default:
        ui->sboxResizeWidth->setValue(0);
        ui->sboxResizeHeight->setValue(0);
    }
}

void VideoSettingsWidget::on_pushButton_clicked()
{
    // Override previous settings when applying new settings
    prevSettings = getSettings();
    emit settingsChanged();
}


void VideoSettingsWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    // Temporarily store previous settings that can be restored
    // when the window is closed
    prevSettings = getSettings();
    event->accept();
}

void VideoSettingsWidget::closeEvent(QCloseEvent *event)
{
    emit closed();

    // Restore previous settings that were applied
    auto& t = prevSettings;
    ui->sboxCropBottom->setValue(t.value("cropbottom"));
    ui->sboxCropLeft->setValue(t.value("cropleft"));
    ui->sboxCropRight->setValue(t.value("cropright"));
    ui->sboxCropTop->setValue(t.value("croptop"));
    ui->sboxResizeHeight->setValue(t.value("resizeheight"));
    ui->sboxResizeWidth->setValue(t.value("resizewidth"));
    ui->radioButton->setChecked(true);
    ui->radioDeinterlace->setChecked(static_cast<bool>(t.value("deinterlace")));
    ui->radioInverseTelecine->setChecked(static_cast<bool>(t.value("ivtc")));
    ui->cboxDvdResolution->setCurrentIndex(t.value("dvdresolutionidx"));

    event->accept();
}

QMap<QString, int> VideoSettingsWidget::getSettings() const
{
    QMap<QString, int> settings;
    settings.clear();
    settings.insert("croptop", ui->sboxCropTop->value());
    settings.insert("cropright", ui->sboxCropRight->value());
    settings.insert("cropbottom", ui->sboxCropBottom->value());
    settings.insert("cropleft", ui->sboxCropLeft->value());
    settings.insert("resizewidth", ui->sboxResizeWidth->value());
    settings.insert("resizeheight", ui->sboxResizeHeight->value());
    settings.insert("ivtc", static_cast<int>(ui->radioInverseTelecine->isChecked()));
    settings.insert("deinterlace", static_cast<int>(ui->radioDeinterlace->isChecked()));
    settings.insert("dvdresolutionidx", ui->cboxDvdResolution->currentIndex());
    return settings;
}

void VideoSettingsWidget::resetSettings()
{
    ui->sboxCropBottom->setValue(0);
    ui->sboxCropLeft->setValue(0);
    ui->sboxCropRight->setValue(0);
    ui->sboxCropTop->setValue(0);
    ui->sboxResizeHeight->setValue(0);
    ui->sboxResizeWidth->setValue(0);
    ui->radioButton->setChecked(true);
    ui->cboxDvdResolution->setCurrentIndex(0);
}

} // namespace vfg
