#include "aboutwidget.hpp"
#include "ui_aboutwidget.h"

namespace vfg {
namespace ui {

AboutWidget::AboutWidget(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::AboutWidget)
{
    ui->setupUi(this);
}

AboutWidget::~AboutWidget()
{
    delete ui;
}

} // namespace ui
} // namespace vfg
