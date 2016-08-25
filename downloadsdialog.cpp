#include <memory>
#include <utility>
#include <QAction>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>
#include <QVariant>
#include "downloadsdialog.hpp"
#include "downloadslistmodel.hpp"
#include "httpdownload.hpp"
#include "progressbardelegate.hpp"
#include "ptrutil.hpp"

namespace vfg {
namespace ui {

DownloadsDialog::DownloadsDialog(QWidget *parent) :
    QDialog(parent),
    netMan(new QNetworkAccessManager),
    model(new vfg::core::DownloadsListModel)
{
    ui.setupUi(this);

    ui.downloadList->setItemDelegate(new vfg::ui::ProgressBarDelegate(ui.downloadList));
    ui.downloadList->setModel(model.get());
    ui.downloadList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.downloadList->setUniformItemSizes(true);

    connect(ui.downloadList, &QListView::customContextMenuRequested,
            this, &DownloadsDialog::contextMenuRequested);
}

void DownloadsDialog::addDownload(const QNetworkRequest &request)
{
    const QUrl url = request.url();
    if(!(url.scheme() == "http" || url.scheme() == "https" ||
            url.scheme() == "ftp" || url.scheme() == "ftps")) {
        QMessageBox::information(this, tr("Unsupported scheme"),
                                 tr("This scheme is not supported"));
        return;
    }

    QSettings config("config.ini", QSettings::IniFormat);
    auto httpReq = std::make_shared<vfg::net::HttpDownload>(request,
                                                        QDir(config.value("cachedirectory").toString()));
    httpReq->start(netMan.get());
    model->addItem(std::move(httpReq));
}

void DownloadsDialog::on_pushButton_clicked()
{
    model->clearFinished();
}

void DownloadsDialog::contextMenuRequested(const QPoint& pos)
{
    const QModelIndex index = ui.downloadList->indexAt(pos);
    if(!index.isValid()) {
        return;
    }

    const QVariant data = model->data(index);
    const auto download = data.value<std::shared_ptr<vfg::net::HttpDownload>>();
    const auto status = download->getStatus();
    QMenu menu;
    auto action = vfg::make_unique<QAction>(&menu);
    if(status == vfg::net::HttpDownload::Status::Running) {        
        action->setText(tr("Stop"));
        connect(action.get(), &QAction::triggered, [&download]() {
            download->abort();
        });
    }
    else if(status == vfg::net::HttpDownload::Status::Finished) {
        action->setText(tr("Play"));
        connect(action.get(), &QAction::triggered, [&download, this]() {
            emit play(download->path());
        });
    }
    else if(status == vfg::net::HttpDownload::Status::Aborted) {
        action->setText(tr("Retry"));
        connect(action.get(), &QAction::triggered, [&download]() {
            download->retry();
        });
    }
    menu.addAction(action.release());
    menu.exec(QCursor::pos());
}

} // namespace ui
} // namespace vfg
