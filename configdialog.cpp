#include <QSettings>
#include "configdialog.h"
#include "ui_configdialog.h"

using namespace vfg;

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    // Load settings
    QSettings cfg("config.ini", QSettings::IniFormat);
    QString avisynthPath = cfg.value("avisynthpluginspath").toString();
    const bool saveScripts = cfg.value("savescripts").toBool();
    const bool showEditor = cfg.value("showscripteditor").toBool();
    const int maxThumbnails = cfg.value("maxthumbnails").toInt();
    //const int afterMaxLimit = cfg.value("aftermaxlimit").toInt();
    const bool pauseAfterLimit = cfg.value("pauseafterlimit").toBool();
    const bool removeOldestAfterLimit = cfg.value("removeoldestafterlimit").toBool();
    const bool jumpToLastOnFinish = cfg.value("jumptolastonfinish").toBool();
    const bool jumpToLastOnPause = cfg.value("jumptolastonpause").toBool();
    const bool jumpToLastOnStop = cfg.value("jumptolastonstop").toBool();

    ui->avisynthPluginsPathLineEdit->setText(avisynthPath);
    ui->saveAvisynthScriptsCheckBox->setChecked(saveScripts);
    ui->showScriptEditorCheckBox->setChecked(showEditor);
    ui->maxThumbnailsSpinBox->setValue(maxThumbnails);
    //ui->buttonGroup->button(afterMaxLimit)->setChecked(true);
    ui->radioPauseAfterLimit->setChecked(pauseAfterLimit);
    ui->radioRemoveOldestAfterLimit->setChecked(removeOldestAfterLimit);
    ui->cbJumpAfterFinish->setChecked(jumpToLastOnFinish);
    ui->cbJumpAfterPause->setChecked(jumpToLastOnPause);
    ui->cbJumpAfterStop->setChecked(jumpToLastOnStop);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void vfg::ConfigDialog::on_buttonBox_rejected()
{
    reject();
}

void vfg::ConfigDialog::on_buttonBox_accepted()
{
    QSettings cfg("config.ini", QSettings::IniFormat);
    cfg.setValue("avisynthpluginspath", ui->avisynthPluginsPathLineEdit->text());
    cfg.setValue("savescripts", ui->saveAvisynthScriptsCheckBox->isChecked());
    cfg.setValue("showscripteditor", ui->showScriptEditorCheckBox->isChecked());
    cfg.setValue("maxthumbnails", ui->maxThumbnailsSpinBox->value());
    //cfg.setValue("aftermaxlimit", ui->buttonGroup->checkedId());
    cfg.setValue("pauseafterlimit", ui->radioPauseAfterLimit->isChecked());
    cfg.setValue("removeoldestafterlimit", ui->radioRemoveOldestAfterLimit->isChecked());
    cfg.setValue("jumptolastonfinish", ui->cbJumpAfterFinish->isChecked());
    cfg.setValue("jumptolastonpause", ui->cbJumpAfterPause->isChecked());
    cfg.setValue("jumptolastonstop", ui->cbJumpAfterStop->isChecked());
}
