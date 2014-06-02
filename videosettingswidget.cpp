#include <QCloseEvent>
#include <QMap>
#include <QRect>
#include <QShowEvent>
#include <QSize>
#include <QString>
#include "ptrutil.hpp"
#include "videosettingswidget.h"
#include "ui_videosettingswidget.h"

/**
 * @brief noneg prevents negative values being used
 * @param x variable to check for negative value
 * @return 0 if x is less than 0, otherwise x
 */
constexpr int noneg(const int x)
{
    return x < 0 ? 0 : x;
}

vfg::ui::VideoSettingsWidget::VideoSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(nullptr),
    prevSettings()
{
    ui = util::make_unique<Ui::VideoSettingsWidget>();
    ui->setupUi(this);

    ui->cboxDvdResolution->insertItem(Default_AR, tr("Default"));
    ui->cboxDvdResolution->insertItem(NTSC_16_9, tr("NTSC 16:9"));
    ui->cboxDvdResolution->insertItem(NTSC_4_3, tr("NTSC 4:3"));
    ui->cboxDvdResolution->insertItem(PAL_16_9, tr("PAL 16:9"));
    ui->cboxDvdResolution->insertItem(PAL_4_3, tr("PAL 4:3"));

    connect(ui->sboxCropBottom, SIGNAL(valueChanged(int)),
            this,               SLOT(handleCropChange()));

    connect(ui->sboxCropLeft,   SIGNAL(valueChanged(int)),
            this,               SLOT(handleCropChange()));

    connect(ui->sboxCropTop,    SIGNAL(valueChanged(int)),
            this,               SLOT(handleCropChange()));

    connect(ui->sboxCropRight,  SIGNAL(valueChanged(int)),
            this,               SLOT(handleCropChange()));
}

vfg::ui::VideoSettingsWidget::~VideoSettingsWidget() {

}

void vfg::ui::VideoSettingsWidget::on_cboxDvdResolution_activated(const int index)
{
    static const QMap<int, QSize> resolutions {
        {Default_AR, {0, 0}},
        {NTSC_16_9, {854, 480}},
        {NTSC_4_3, {720, 540}},
        {PAL_16_9, {1024, 576}},
        {PAL_4_3, {768, 576}}
    };

    const auto& res = resolutions.value(index);
    ui->sboxResizeWidth->setValue(res.width());
    ui->sboxResizeHeight->setValue(res.height());
}

void vfg::ui::VideoSettingsWidget::on_pushButton_clicked()
{
    // Override previous settings when applying new settings
    prevSettings = getSettings();

    crop.left += ui->sboxCropLeft->value();
    crop.top += ui->sboxCropTop->value();
    crop.right += ui->sboxCropRight->value();
    crop.bottom += ui->sboxCropBottom->value();

    // Negative crop values must be checked only after
    // the current crop values have been added to existing values,
    // otherwise it is not possible to use negative values at all
    crop.left = noneg(crop.left);
    crop.top = noneg(crop.top);
    crop.right = noneg(crop.right);
    crop.bottom = noneg(crop.bottom);

    ui->sboxCropBottom->setValue(0);
    ui->sboxCropLeft->setValue(0);
    ui->sboxCropRight->setValue(0);
    ui->sboxCropTop->setValue(0);

    if(crop.left || crop.top || crop.right || crop.bottom) {
        ui->btnRevertCrop->setEnabled(true);
    }

    emit settingsChanged();
}

void vfg::ui::VideoSettingsWidget::handleCropChange()
{
    const QRect area(noneg(ui->sboxCropLeft->value()),
                     noneg(ui->sboxCropTop->value()),
                     noneg(ui->sboxCropRight->value()),
                     noneg(ui->sboxCropBottom->value()));

    emit cropChanged(area);
}

void vfg::ui::VideoSettingsWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    // Temporarily store previous settings that can be restored
    // when the window is closed
    prevSettings = getSettings();
    event->accept();
}

void vfg::ui::VideoSettingsWidget::closeEvent(QCloseEvent *event)
{
    emit closed();

    // Restore previous settings that were applied
    ui->sboxCropBottom->setValue(0);
    ui->sboxCropLeft->setValue(0);
    ui->sboxCropRight->setValue(0);
    ui->sboxCropTop->setValue(0);
    ui->sboxResizeHeight->setValue(prevSettings.value("resizeheight"));
    ui->sboxResizeWidth->setValue(prevSettings.value("resizewidth"));
    ui->radioButton->setChecked(true);
    ui->radioDeinterlace->setChecked(static_cast<bool>(prevSettings.value("deinterlace")));
    ui->radioInverseTelecine->setChecked(static_cast<bool>(prevSettings.value("ivtc")));
    ui->cboxDvdResolution->setCurrentIndex(prevSettings.value("dvdresolutionidx"));

    event->accept();
}

QMap<QString, int> vfg::ui::VideoSettingsWidget::getSettings() const
{
    QMap<QString, int> settings;
    settings.clear();
    settings.insert("croptop", crop.top);
    settings.insert("cropright", crop.right);
    settings.insert("cropbottom", crop.bottom);
    settings.insert("cropleft", crop.left);
    settings.insert("resizewidth", ui->sboxResizeWidth->value());
    settings.insert("resizeheight", ui->sboxResizeHeight->value());
    settings.insert("ivtc", static_cast<int>(ui->radioInverseTelecine->isChecked()));
    settings.insert("deinterlace", static_cast<int>(ui->radioDeinterlace->isChecked()));
    settings.insert("dvdresolutionidx", ui->cboxDvdResolution->currentIndex());
    return settings;
}

void vfg::ui::VideoSettingsWidget::resetSettings()
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

void vfg::ui::VideoSettingsWidget::on_btnRevertCrop_clicked()
{
    ui->sboxCropBottom->setValue(crop.bottom);
    ui->sboxCropLeft->setValue(crop.left);
    ui->sboxCropRight->setValue(crop.right);
    ui->sboxCropTop->setValue(crop.top);

    crop.bottom = crop.left = crop.right = crop.top = 0;

    emit settingsChanged();
}
