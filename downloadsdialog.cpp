#include <QAction>
#include <QMenu>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QVariant>
#include "downloadsdialog.hpp"
#include "downloadslistmodel.hpp"
#include "httpdownload.hpp"
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
    ui->downloadList->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->downloadList, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));
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

void DownloadsDialog::on_pushButton_clicked()
{
    model->clearFinished();
}

void DownloadsDialog::contextMenuRequested(const QPoint& pos)
{
    const QModelIndex index = ui->downloadList->indexAt(pos);
    if(!index.isValid()) {
        return;
    }

    const QVariant data = model->data(index);
    const vfg::net::HttpDownload* download = data.value<vfg::net::HttpDownload*>();
    if(!download->isFinished()) {
        return;
    }

    QMenu menu;
    QAction* playAction = new QAction(tr("Play"), this);
    playAction->setData(1);
    menu.addAction(playAction);
    QAction* selected = menu.exec(QCursor::pos());
    if(selected && selected->data().toInt() == 1) {
        emit play(download->fileName());
    }
}

} // namespace ui
} // namespace vfg
