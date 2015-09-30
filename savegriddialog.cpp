#include <QIcon>
#include <QListWidgetItem>
#include <QSpinBox>
#include "savegriddialog.hpp"

namespace vfg {
namespace ui {

SaveGridDialog::SaveGridDialog(QWidget *parent) :
    QDialog(parent),
    ui()
{
    ui.setupUi(this);
    ui.iconList->setUniformItemSizes(true);
    ui.iconList->setDragDropMode(QAbstractItemView::DragOnly);
    ui.iconList->setFixedWidth(180);
    ui.iconList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.gridWidget->setSpacing(ui.spacingSpinBox->value());

    connect(ui.spacingSpinBox,  SIGNAL(valueChanged(int)),
            ui.gridWidget,      SLOT(setSpacing(int)));

    connect(ui.resizeToWidth,   SIGNAL(valueChanged(int)),
            ui.gridWidget,      SLOT(setWidth(int)));
}

void SaveGridDialog::addPixmap(const QPixmap &img)
{
    auto item = new QListWidgetItem;
    QIcon icon(img);
    item->setIcon(icon);
    ui.iconList->insertItem(0, item);
    ui.iconList->setIconSize(img.scaledToWidth(150).size());
    ui.resizeToWidth->setValue(img.width());
}

} // namespace ui
} // namespace vfg
