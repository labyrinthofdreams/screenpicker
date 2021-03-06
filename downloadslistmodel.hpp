#ifndef VFG_CORE_DOWNLOADSLISTMODEL_HPP
#define VFG_CORE_DOWNLOADSLISTMODEL_HPP

#include <memory>
#include <QAbstractListModel>
#include <QList>
#include <QNetworkAccessManager>
#include <QVariant>

class QModelIndex;
class QObject;

namespace vfg {
namespace net {
    class HttpDownload;
}
}

namespace vfg {
namespace core {

class DownloadsListModel : public QAbstractListModel
{
    Q_OBJECT

private:
    //! Active download requests
    QList<std::shared_ptr<vfg::net::HttpDownload>> downloads {};

    //! Network access manager
    std::unique_ptr<QNetworkAccessManager> netMan;

public:
    /**
     * @brief Constructor
     * @param parent Owner of the object
     */
    explicit DownloadsListModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const override;

    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Add new request
     * @param download New request
     */
    void addItem(std::shared_ptr<vfg::net::HttpDownload> download);

    /**
     * @brief Clear finished requests
     */
    void clearFinished();

private slots:
    /**
     * @brief Update model data
     */
    void updateData();
};

} // namespace core
} // namespace vfg

#endif // VFG_CORE_DOWNLOADSLISTMODEL_HPP
