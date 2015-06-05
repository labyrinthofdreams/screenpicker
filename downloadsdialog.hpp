#ifndef VFG_UI_DOWNLOADSDIALOG_HPP
#define VFG_UI_DOWNLOADSDIALOG_HPP

#include <memory>
#include <QDialog>
#include <QString>
#include "httpdownload.hpp"

class QNetworkAccessManager;
class QNetworkReply;
class QPoint;
class QUrl;

namespace vfg {
namespace core {
    class DownloadsListModel;
}
}

namespace vfg {
namespace ui {

namespace Ui {
class DownloadsDialog;
}

class DownloadsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Owner of the widget
     */
    explicit DownloadsDialog(QWidget *parent = 0);

    /**
     * @brief Destructor
     */
    ~DownloadsDialog();

    /**
     * @brief Add new download request
     * @param request Request to add
     */
    void addDownload(std::shared_ptr<vfg::net::HttpDownload> request);

private:
    //! UI
    Ui::DownloadsDialog *ui;

    //! Network access manager
    std::unique_ptr<QNetworkAccessManager> netMan;

    //! Model data
    std::unique_ptr<vfg::core::DownloadsListModel> model;

private slots:
    /**
     * @brief Update model data
     */
    void updateList();

    /**
     * @brief Clear finished downloads
     */
    void on_pushButton_clicked();

    /**
     * @brief Display context menu
     * @param pos Cursor position
     */
    void contextMenuRequested(const QPoint& pos);

signals:
    void play(QString path);
};


} // namespace ui
} // namespace vfg
#endif // VFG_UI_DOWNLOADSDIALOG_HPP
