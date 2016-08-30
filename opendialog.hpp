#ifndef OPENDIALOG_HPP
#define OPENDIALOG_HPP

#include <memory>
#include <QDialog>
#include <QSettings>
#include "extractors/baseextractor.hpp"
#include "ui_opendialog.h"

class QNetworkRequest;
class QString;
class QStringList;
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

    enum class Tab {
        OpenDisc,
        OpenStream
    };

    void setActiveTab(Tab tabName);

private:
    ::Ui::OpenDialog ui;

    std::unique_ptr<vfg::extractor::BaseExtractor> extractor;

    QSettings config {"config.ini", QSettings::IniFormat};

private slots:
    void on_networkUrl_textEdited(const QString &arg1);

    void streamsReady();

    void on_openButton_clicked();

    void on_cancelButton_clicked();

    void on_browseFiles_clicked();

    void on_clearButton_clicked();

    void on_removeButton_clicked();

signals:
    void openUrl(const QNetworkRequest& url);

    void processDiscFiles(const QStringList& files);
};

} // namespace ui
} // namespace vfg

#endif // OPENDIALOG_HPP
