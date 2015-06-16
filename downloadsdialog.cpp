#include <memory>
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

    ui->downloadList->setItemDelegate(new vfg::ui::ProgressBarDelegate(ui->downloadList));
    ui->downloadList->setModel(model.get());
    ui->downloadList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->downloadList->setUniformItemSizes(true);

    connect(ui->downloadList, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));
}

DownloadsDialog::~DownloadsDialog()
{
    delete ui;
}

void DownloadsDialog::addDownload(std::shared_ptr<vfg::net::HttpDownload> request)
{
    request->start(netMan.get());

    model->addItem(request);

    connect(request.get(), SIGNAL(updated()), this, SLOT(updateList()));
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
    auto download = data.value<std::shared_ptr<vfg::net::HttpDownload>>();
    const auto status = download->getStatus();
    if(status == vfg::net::HttpDownload::Status::Running) {
        QMenu menu;
        QAction* stopAction = new QAction(tr("Stop"), this);
        stopAction->setData(1);
        menu.addAction(stopAction);
        QAction* selected = menu.exec(QCursor::pos());
        if(selected && selected->data().toInt() == 1) {
            download->abort();
        }
    }
    else if(status == vfg::net::HttpDownload::Status::Finished &&
            status != vfg::net::HttpDownload::Status::Aborted) {
        QMenu menu;
        QAction* playAction = new QAction(tr("Play"), this);
        playAction->setData(1);
        menu.addAction(playAction);
        QAction* selected = menu.exec(QCursor::pos());
        if(selected && selected->data().toInt() == 1) {
            emit play(download->fileName());
        }
    }
}

} // namespace ui
} // namespace vfg
