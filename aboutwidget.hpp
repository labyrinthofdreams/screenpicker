#ifndef VFG_UI_ABOUTWIDGET_HPP
#define VFG_UI_ABOUTWIDGET_HPP

#include <QDialog>

namespace vfg {
namespace ui {

namespace Ui {
class AboutWidget;
}

class AboutWidget : public QDialog
{
    Q_OBJECT

public:
    explicit AboutWidget(QWidget *parent = 0);
    ~AboutWidget();

private:
    Ui::AboutWidget *ui;
};


} // namespace ui
} // namespace vfg
#endif // VFG_UI_ABOUTWIDGET_HPP
