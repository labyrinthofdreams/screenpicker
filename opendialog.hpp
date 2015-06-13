#ifndef OPENDIALOG_HPP
#define OPENDIALOG_HPP

#include <memory>
#include <QDialog>
#include "extractors/baseextractor.hpp"

class QNetworkRequest;
class QString;
class QUrl;

namespace Ui {
class OpenDialog;
}

namespace vfg {
namespace ui {

class OpenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpenDialog(QWidget *parent = 0);
    ~OpenDialog();

private:
    ::Ui::OpenDialog *ui;

    std::unique_ptr<vfg::extractor::BaseExtractor> extractor;

private slots:
    void on_networkUrl_textEdited(const QString &arg1);

    void streamsReady();

    void on_openButton_clicked();

    void on_cancelButton_clicked();

signals:
    void openUrl(const QNetworkRequest& url);
};

} // namespace ui
} // namespace vfg

#endif // OPENDIALOG_HPP
