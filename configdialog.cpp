#include <utility>
#include <QSettings>
#include <QFileDialog>
#include <QString>
#include "configdialog.h"
#include "ptrutil.hpp"

vfg::ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    // Load settings
    QSettings cfg("config.ini", QSettings::IniFormat);
    const bool showEditor = cfg.value("showscripteditor").toBool();
    const int maxThumbnails = cfg.value("maxthumbnails").toInt();
    //const int afterMaxLimit = cfg.value("aftermaxlimit").toInt();
    const bool pauseAfterLimit = cfg.value("pauseafterlimit").toBool();
    const bool removeOldestAfterLimit = cfg.value("removeoldestafterlimit").toBool();
    const bool jumpToLastOnFinish = cfg.value("jumptolastonfinish").toBool();
    const bool jumpToLastOnPause = cfg.value("jumptolastonpause").toBool();
    const bool jumpToLastOnStop = cfg.value("jumptolastonstop").toBool();
    const bool jumpToLastOnReachingMax = cfg.value("jumptolastonreachingmax").toBool();
    const bool saveDgIndexFiles = cfg.value("savedgindexfiles").toBool();

    ui.avisynthPluginsPathLineEdit->setText(cfg.value("avisynthpluginspath").toString());
    ui.dgindexExecPath->setText(cfg.value("dgindexexecpath").toString());
    ui.showScriptEditorCheckBox->setChecked(showEditor);
    ui.maxThumbnailsSpinBox->setValue(maxThumbnails);
    //ui.buttonGroup->button(afterMaxLimit)->setChecked(true);
    ui.radioPauseAfterLimit->setChecked(pauseAfterLimit);
    ui.radioRemoveOldestAfterLimit->setChecked(removeOldestAfterLimit);
    ui.cbJumpAfterFinish->setChecked(jumpToLastOnFinish);
    ui.cbJumpAfterPause->setChecked(jumpToLastOnPause);
    ui.cbJumpAfterStop->setChecked(jumpToLastOnStop);
    ui.cbStopAfterReachingMax->setChecked(jumpToLastOnReachingMax);
    ui.cbSaveDgindexFiles->setChecked(saveDgIndexFiles);
    ui.cbShowVideoSettings->setChecked(cfg.value("showvideosettings").toBool());
    ui.cbResumeGeneratorAfterClear->setChecked(cfg.value("resumegeneratorafterclear").toBool());
    ui.editImageMagickPath->setText(cfg.value("imagemagickpath").toString());
    ui.editGifsiclePath->setText(cfg.value("gifsiclepath").toString());

    ui.spinImageMagickTimeout->setValue(cfg.value("imagemagicktimeout").toInt());
    ui.spinGifsicleTimeout->setValue(cfg.value("gifsicletimeout").toInt());

    ui.editX264Path->setText(cfg.value("x264path").toString());
    ui.cacheFolder->setText(cfg.value("cachedirectory").toString());
}

void vfg::ConfigDialog::on_buttonBox_rejected()
{
    reject();
}

void vfg::ConfigDialog::on_buttonBox_accepted()
{
    QSettings cfg("config.ini", QSettings::IniFormat);
    cfg.setValue("avisynthpluginspath", ui.avisynthPluginsPathLineEdit->text());
    cfg.setValue("dgindexexecpath", ui.dgindexExecPath->text());
    cfg.setValue("showscripteditor", ui.showScriptEditorCheckBox->isChecked());
    cfg.setValue("maxthumbnails", ui.maxThumbnailsSpinBox->value());
    //cfg.setValue("aftermaxlimit", ui.buttonGroup->checkedId());
    cfg.setValue("pauseafterlimit", ui.radioPauseAfterLimit->isChecked());
    cfg.setValue("removeoldestafterlimit", ui.radioRemoveOldestAfterLimit->isChecked());
    cfg.setValue("jumptolastonfinish", ui.cbJumpAfterFinish->isChecked());
    cfg.setValue("jumptolastonpause", ui.cbJumpAfterPause->isChecked());
    cfg.setValue("jumptolastonstop", ui.cbJumpAfterStop->isChecked());
    cfg.setValue("jumptolastonreachingmax", ui.cbStopAfterReachingMax->isChecked());
    cfg.setValue("savedgindexfiles", ui.cbSaveDgindexFiles->isChecked());
    cfg.setValue("showvideosettings", ui.cbShowVideoSettings->isChecked());
    cfg.setValue("resumegeneratorafterclear", ui.cbResumeGeneratorAfterClear->isChecked());
    cfg.setValue("imagemagickpath", ui.editImageMagickPath->text());
    cfg.setValue("gifsiclepath", ui.editGifsiclePath->text());
    cfg.setValue("x264path", ui.editX264Path->text());
    cfg.setValue("cachedirectory", ui.cacheFolder->text());
}

void vfg::ConfigDialog::on_btnDgindexPath_clicked()
{
    const auto path = QFileDialog::getOpenFileName(this, tr("DGIndex Executable Path"),
                                                   "/", "dgindex.exe");
    ui.dgindexExecPath->setText(path);
}

void vfg::ConfigDialog::on_btnAvisynthPluginsPath_clicked()
{
    const auto path = QFileDialog::getExistingDirectory(this, tr("Avisynth Plugins Directory"),
                                                        "/");
    ui.avisynthPluginsPathLineEdit->setText(path);
}

void vfg::ConfigDialog::on_buttonImageMagickBrowse_clicked()
{
    const auto path = QFileDialog::getOpenFileName(this, tr("ImageMagick executable path"),
                                                   "/", "convert.exe");
    ui.editImageMagickPath->setText(path);
}

void vfg::ConfigDialog::on_buttonGifsicleBrowse_clicked()
{
    const auto path = QFileDialog::getOpenFileName(this, tr("Gifsicle executable path"),
                                                   "/", "gifsicle.exe");
    ui.editGifsiclePath->setText(path);
}

void vfg::ConfigDialog::on_buttonBrowseX264_clicked()
{
    const auto path = QFileDialog::getOpenFileName(this, tr("x264 executable path"),
                                                   "/", "x264.exe");
    ui.editX264Path->setText(path);
}

void vfg::ConfigDialog::on_browseCacheFolder_clicked()
{
    const auto path = QFileDialog::getOpenFileName(this, tr("Cache directory"), "/");
    if(!path.isEmpty()) {
        ui.cacheFolder->setText(path);
    }
}
