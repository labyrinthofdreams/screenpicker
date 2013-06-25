#include <QMap>
#include <QString>
#include "videosettingswidget.h"
#include "ui_videosettingswidget.h"

namespace vfg {

VideoSettingsWidget::VideoSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoSettingsWidget)
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
    emit settingsChanged();
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
    return settings;
}

} // namespace vfg
