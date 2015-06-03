#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include "downloadsdialog.hpp"
#include "downloadslistmodel.hpp"
#include "progressbardelegate.hpp"
#include "ui_downloadsdialog.h"

namespace vfg {
namespace ui {

DownloadsDialog::DownloadsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadsDialog),
    netMan(new QNetworkAccessManager),
    model(new vfg::core::DownloadsListModel)
{
    ui->setupUi(this);

    ui->downloadList->setItemDelegate(new vfg::ui::ProgressBarDelegate(50, ui->downloadList));
    ui->downloadList->setModel(model.get());
}

DownloadsDialog::~DownloadsDialog()
{
    delete ui;
}

void DownloadsDialog::addDownload(vfg::net::HttpDownload *request)
{
    request->start(netMan.get());

    model->addItem(request);

    connect(request, SIGNAL(updated()), this, SLOT(updateList()));
}

void DownloadsDialog::updateList()
{
    model->updateData();
}

} // namespace ui
} // namespace vfg

void vfg::ui::DownloadsDialog::on_pushButton_clicked()
{
    model->clearFinished();
}
