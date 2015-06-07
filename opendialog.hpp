#ifndef OPENDIALOG_HPP
#define OPENDIALOG_HPP

#include <QDialog>
#include <QSettings>

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

private slots:
    void on_buttonBox_accepted();

private:
    ::Ui::OpenDialog *ui;

    QSettings config;

signals:
    void openUrl(const QUrl& url);
};

} // namespace ui
} // namespace vfg

#endif // OPENDIALOG_HPP
