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

    ui->avisynthPluginsPathLineEdit->setText(avisynthPath);
    ui->saveAvisynthScriptsCheckBox->setChecked(saveScripts);
    ui->showScriptEditorCheckBox->setChecked(showEditor);
    ui->maxThumbnailsSpinBox->setValue(maxThumbnails);
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
}
