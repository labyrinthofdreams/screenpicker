#ifndef VFG_UI_SAVEGRIDDIALOG_HPP
#define VFG_UI_SAVEGRIDDIALOG_HPP

#include <QDialog>
#include "ui_savegriddialog.h"

class QPixmap;

namespace vfg {
namespace ui {

namespace Ui {
class SaveGridDialog;
}

class SaveGridDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SaveGridDialog(QWidget *parent = 0);
    ~SaveGridDialog() = default;

    void addPixmap(const QPixmap &img);

private slots:
    void on_pushButton_clicked();

private:
    Ui::SaveGridDialog ui;
};


} // namespace ui
} // namespace vfg

#endif // VFG_UI_SAVEGRIDDIALOG_HPP
