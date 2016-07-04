#include <algorithm>
#include <memory>
#include <utility>
#include <QAbstractListModel>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include "downloadslistmodel.hpp"
#include "httpdownload.hpp"

namespace vfg {
namespace core {

DownloadsListModel::DownloadsListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

int DownloadsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return downloads.size();
}

int DownloadsListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QVariant DownloadsListModel::data(const QModelIndex &index, const int role) const
{
    Q_UNUSED(role);

    if(!index.isValid()) {
        return {};
    }

    if(role == Qt::DisplayRole) {
        return QVariant::fromValue(downloads[index.row()]);
    }

    return {};
}

void DownloadsListModel::addItem(std::shared_ptr<vfg::net::HttpDownload> download)
{
    beginInsertRows(QModelIndex(), downloads.size(), downloads.size());
    downloads.append(std::move(download));
    endInsertRows();
}

void DownloadsListModel::updateData()
{
    beginResetModel();
    endResetModel();
}

void DownloadsListModel::clearFinished()
{
    downloads.erase(std::remove_if(downloads.begin(), downloads.end(),
                                   [](const std::shared_ptr<vfg::net::HttpDownload> &dl) {
                        return dl->isFinished();
                    }), downloads.end());
    beginResetModel();
    endResetModel();
}

} // namespace core
} // namespace vfg
