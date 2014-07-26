#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

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

    /**
     * @brief Destructor
     *
     * Definition is required for the unique_ptr
     */
    ~ConfigDialog();

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
    void on_btnDgindexPath_clicked();
    void on_btnAvisynthPluginsPath_clicked();

    void on_buttonImageMagickBrowse_clicked();

    void on_buttonGifsicleBrowse_clicked();

    void on_buttonBrowseX264_clicked();

private:
    Ui::ConfigDialog* ui;
};

} // namespace vfg

#endif // CONFIGDIALOG_H
