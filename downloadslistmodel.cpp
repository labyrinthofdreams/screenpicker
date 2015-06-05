#include <memory>
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

    if (!index.isValid()) {
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
    downloads.push_back(download);
    endInsertRows();
}

void DownloadsListModel::updateData()
{
    beginResetModel();
    endResetModel();
}

void DownloadsListModel::clearFinished()
{
    // The reason this is split in two for loops is because whenever
    // removeAt is called, the size() and the indexes decrement by one.
    // The second for loop adjusts the indexes to correct ones as
    // items are removed with removeAt
    QList<int> toBeRemoved;
    for(int i = 0; i < downloads.size(); ++i) {
        if(downloads[i]->isFinished()) {
            toBeRemoved.append(i);
        }
    }

    for(int i = 0, removed = 0; i < toBeRemoved.size(); ++i, ++removed) {
        const auto realIndex = toBeRemoved[i] - removed;
        beginRemoveRows(QModelIndex(), realIndex, realIndex);
        auto dl = downloads.takeAt(realIndex);
        dl->deleteLater();
        endRemoveRows();
    }
}

} // namespace core
} // namespace vfg
