#include <QSettings>
#include "jumptoframedialog.hpp"
#include "ui_jumptoframedialog.h"

namespace vfg {
namespace ui {

JumpToFrameDialog::JumpToFrameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JumpToFrameDialog)
{
    ui->setupUi(this);

    // Hide time spin boxes
    ui->hours->setHidden(true);
    ui->minutes->setHidden(true);
    ui->seconds->setHidden(true);
}

JumpToFrameDialog::~JumpToFrameDialog()
{
    delete ui;
}

void JumpToFrameDialog::on_frame_clicked()
{
    ui->hours->setHidden(true);
    ui->minutes->setHidden(true);
    ui->seconds->setHidden(true);
    ui->frameSpinBox->setHidden(false);
}

void JumpToFrameDialog::on_time_clicked()
{
    ui->hours->setHidden(false);
    ui->minutes->setHidden(false);
    ui->seconds->setHidden(false);
    ui->frameSpinBox->setHidden(true);
}

void JumpToFrameDialog::on_goButton_clicked()
{
    QSettings config("config.ini", QSettings::IniFormat);
    config.setValue("jumptoformat", ui->time->isChecked() ? "time" : "frame");
    int jumpTo;
    if(ui->time->isChecked()) {
        jumpTo = ui->hours->value() * 60 * 60 +
                 ui->minutes->value() * 60 +
                 ui->seconds->value();
    }
    else {
        jumpTo = ui->frameSpinBox->value();
    }

    config.setValue("jumpto", jumpTo);

    accept();
}

} // namespace ui
} // namespace vfg
