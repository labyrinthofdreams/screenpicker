#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include "ui_configdialog.h"

namespace Ui {
class ConfigDialog;
}

namespace vfg {

/**
 * @brief The ConfigDialog class
 */
class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Owner of the object
     */
    explicit ConfigDialog(QWidget *parent = 0);

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
    void on_btnDgindexPath_clicked();
    void on_btnAvisynthPluginsPath_clicked();

    void on_buttonImageMagickBrowse_clicked();

    void on_buttonGifsicleBrowse_clicked();

    void on_buttonBrowseX264_clicked();

    void on_browseCacheFolder_clicked();

private:
    Ui::ConfigDialog ui;
};

} // namespace vfg

#endif // CONFIGDIALOG_H
