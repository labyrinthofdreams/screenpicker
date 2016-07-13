#include <cmath>
#include <QCloseEvent>
#include <QMap>
#include <QRect>
#include <QSettings>
#include <QShowEvent>
#include <QSize>
#include <QString>
#include <QVariant>
#include "ptrutil.hpp"
#include "videosettingswidget.h"

namespace {

/**
 * @brief noneg prevents negative values being used
 * @param x variable to check for negative value
 * @return 0 if x is less than 0, otherwise x
 */
constexpr int noneg(const int x)
{
    return x < 0 ? 0 : x;
}

} // namespace

namespace vfg {
namespace ui {

VideoSettingsWidget::VideoSettingsWidget(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);

    ui.cboxDvdResolution->insertItem(Original, tr("Original"));
    ui.cboxDvdResolution->insertItem(NTSC_16_9, tr("NTSC 16:9"));
    ui.cboxDvdResolution->insertItem(NTSC_4_3, tr("NTSC 4:3"));
    ui.cboxDvdResolution->insertItem(PAL_16_9, tr("PAL 16:9"));
    ui.cboxDvdResolution->insertItem(PAL_4_3, tr("PAL 4:3"));

    connect(ui.sboxCropBottom,  static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,               &VideoSettingsWidget::handleCropChange);

    connect(ui.sboxCropLeft,    static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,               &VideoSettingsWidget::handleCropChange);

    connect(ui.sboxCropTop,     static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,               &VideoSettingsWidget::handleCropChange);

    connect(ui.sboxCropRight,   static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,               &VideoSettingsWidget::handleCropChange);

    resetSettings();
}

void VideoSettingsWidget::on_cboxDvdResolution_activated(const int index)
{
    const QMap<int, QSize> resolutions {
        {Original, {sourceWidth, sourceHeight}},
        {NTSC_16_9, {854, 480}},
        {NTSC_4_3, {720, 540}},
        {PAL_16_9, {1024, 576}},
        {PAL_4_3, {768, 576}}
    };

    const auto& res = resolutions.value(index);
    ui.sboxResizeWidth->setValue(res.width());
    ui.sboxResizeHeight->setValue(res.height());
}

void VideoSettingsWidget::on_pushButton_clicked()
{
    // Override previous settings when applying new settings
    prevSettings = getSettings();

    // Since the crop values are always relative to the original
    // frame we cannot have negative crop values which may result
    // if the user is attempting to apply too large negative crop value
    crop.left = noneg(crop.left + ui.sboxCropLeft->value());
    crop.top = noneg(crop.top + ui.sboxCropTop->value());
    crop.right = noneg(crop.right + ui.sboxCropRight->value());
    crop.bottom = noneg(crop.bottom + ui.sboxCropBottom->value());

    ui.sboxCropBottom->setValue(0);
    ui.sboxCropLeft->setValue(0);
    ui.sboxCropRight->setValue(0);
    ui.sboxCropTop->setValue(0);

    if(crop.left || crop.top || crop.right || crop.bottom) {
        ui.btnRevertCrop->setEnabled(true);
    }

    emit settingsChanged();
}

void VideoSettingsWidget::handleCropChange(int)
{
    const QRect area(noneg(ui.sboxCropLeft->value()),
                     noneg(ui.sboxCropTop->value()),
                     noneg(ui.sboxCropRight->value()),
                     noneg(ui.sboxCropBottom->value()));

    emit cropChanged(area);
}

void VideoSettingsWidget::showEvent(QShowEvent *event)
{
    // Temporarily store previous settings that can be restored
    // when the window is closed
    prevSettings = getSettings();

    event->accept();
}

void VideoSettingsWidget::closeEvent(QCloseEvent *event)
{
    // Restore previous settings that were applied
    ui.sboxCropBottom->setValue(0);
    ui.sboxCropLeft->setValue(0);
    ui.sboxCropRight->setValue(0);
    ui.sboxCropTop->setValue(0);
    ui.sboxResizeHeight->setValue(prevSettings.value("resizeheight").toInt());
    ui.sboxResizeWidth->setValue(prevSettings.value("resizewidth").toInt());
    ui.radioButton->setChecked(true);
    ui.radioDeinterlace->setChecked(prevSettings.value("deinterlace").toBool());
    ui.radioInverseTelecine->setChecked(prevSettings.value("ivtc").toBool());
    ui.cboxDvdResolution->setCurrentIndex(prevSettings.value("dvdresolutionidx").toInt());

    emit closed();

    event->accept();
}

QMap<QString, QVariant> VideoSettingsWidget::getSettings() const
{
    QMap<QString, QVariant> settings;
    settings.insert("croptop", crop.top);
    settings.insert("cropright", crop.right);
    settings.insert("cropbottom", crop.bottom);
    settings.insert("cropleft", crop.left);
    settings.insert("resizewidth", ui.sboxResizeWidth->value());
    settings.insert("resizeheight", ui.sboxResizeHeight->value());
    settings.insert("ivtc", ui.radioInverseTelecine->isChecked());
    settings.insert("deinterlace", ui.radioDeinterlace->isChecked());
    settings.insert("dvdresolutionidx", ui.cboxDvdResolution->currentIndex());
    return settings;
}

void VideoSettingsWidget::resetSettings()
{
    crop = {};
    prevSettings = getSettings();

    ui.sboxCropBottom->setValue(0);
    ui.sboxCropLeft->setValue(0);
    ui.sboxCropRight->setValue(0);
    ui.sboxCropTop->setValue(0);
    ui.sboxResizeHeight->setValue(0);
    ui.sboxResizeWidth->setValue(0);
    ui.radioButton->setChecked(true);
    ui.cboxDvdResolution->setCurrentIndex(0);

    const QSize videoRes = config.value("video/resolution").toSize();
    sourceWidth = videoRes.width();
    sourceHeight = videoRes.height();
    ui.sboxResizeWidth->setValue(sourceWidth);
    ui.sboxResizeHeight->setValue(sourceHeight);
}

void VideoSettingsWidget::on_btnRevertCrop_clicked()
{
    ui.sboxCropBottom->setValue(crop.bottom);
    ui.sboxCropLeft->setValue(crop.left);
    ui.sboxCropRight->setValue(crop.right);
    ui.sboxCropTop->setValue(crop.top);

    crop = {};

    emit settingsChanged();
}

void VideoSettingsWidget::on_radioLockWidth_clicked()
{
    ui.sboxResizeWidth->setEnabled(false);
    ui.sboxResizeHeight->setEnabled(true);

    videoHeight = ui.sboxResizeHeight->value();
    videoWidth = ui.sboxResizeWidth->value();
}

void VideoSettingsWidget::on_radioLockHeight_clicked()
{
    ui.sboxResizeWidth->setEnabled(true);
    ui.sboxResizeHeight->setEnabled(false);

    videoHeight = ui.sboxResizeHeight->value();
    videoWidth = ui.sboxResizeWidth->value();
}

void VideoSettingsWidget::on_radioLockDefault_clicked()
{
    ui.sboxResizeWidth->setEnabled(true);
    ui.sboxResizeHeight->setEnabled(true);

    videoHeight = 0;
    videoWidth = 0;
}

void VideoSettingsWidget::on_sboxResizeWidth_valueChanged(const int width)
{
    if(ui.radioLockHeight->isChecked()) {
        const double ratio = static_cast<double>(videoHeight) / videoWidth;
        const int newHeight = std::ceil(ratio * width);
        ui.sboxResizeHeight->setValue(newHeight);
    }
}

void VideoSettingsWidget::on_sboxResizeHeight_valueChanged(const int height)
{
    if(ui.radioLockWidth->isChecked()) {
        const double ratio = static_cast<double>(videoHeight) / videoWidth;
        const int newWidth = std::ceil(height / ratio);
        ui.sboxResizeWidth->setValue(newWidth);
    }
}

} // namespace ui
} // namespace vfg
