#include <QDebug>
#include <QSettings>
#include <QUrl>
#include "opendialog.hpp"
#include "ui_opendialog.h"

vfg::ui::OpenDialog::OpenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenDialog)
{
    ui->setupUi(this);
}

vfg::ui::OpenDialog::~OpenDialog()
{
    delete ui;
}

void vfg::ui::OpenDialog::on_buttonBox_accepted()
{
    emit openUrl(QUrl(ui->networkUrl->text()));
}
