#include <QDebug>
#include <QSettings>
#include <QUrl>
#include "opendialog.hpp"
#include "ui_opendialog.h"

vfg::ui::OpenDialog::OpenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenDialog),
    config("config.ini", QSettings::IniFormat)
{
    ui->setupUi(this);
}

vfg::ui::OpenDialog::~OpenDialog()
{
    delete ui;
}

void vfg::ui::OpenDialog::on_buttonBox_accepted()
{
//    const auto selectedTab = ui->tabWidget->currentWidget()->objectName();
//    if(selectedTab == "networkTab") {
//        config.setValue("open/mode", "network");
//        config.setValue("open/url", ui->networkUrl->text());
//    }
    emit openUrl(QUrl(ui->networkUrl->text()));
}
