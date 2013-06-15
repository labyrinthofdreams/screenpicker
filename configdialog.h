#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

namespace vfg
{
    class ConfigDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit ConfigDialog(QWidget *parent = 0);
        ~ConfigDialog();

    private slots:
        void on_buttonBox_rejected();

        void on_buttonBox_accepted();

        void on_btnDgindexPath_clicked();

        void on_btnAvisynthPluginsPath_clicked();

    private:
        Ui::ConfigDialog *ui;
    };
}

#endif // CONFIGDIALOG_H
