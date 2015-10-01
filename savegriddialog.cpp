#include <QColor>
#include <QFileDialog>
#include <QIcon>
#include <QImage>
#include <QMap>
#include <QListWidgetItem>
#include <QPixmap>
#include <QSpinBox>
#include "libs/qimagegrid/qimagegrid.hpp"
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

void SaveGridDialog::on_pushButton_clicked()
{
    static const QMap<QString, Qt::GlobalColor> colors {{"Transparent", Qt::transparent},
                                                        {"White", Qt::white},
                                                        {"Black", Qt::black},
                                                        {"Red", Qt::red},
                                                        {"Dark red", Qt::darkRed},
                                                        {"Green", Qt::green},
                                                        {"Dark green", Qt::darkGreen},
                                                        {"Blue", Qt::blue},
                                                        {"Dark blue", Qt::darkBlue},
                                                        {"Cyan", Qt::cyan},
                                                        {"Dark cyan", Qt::darkCyan},
                                                        {"Magenta", Qt::magenta},
                                                        {"Dark magenta", Qt::darkMagenta},
                                                        {"Yellow", Qt::yellow},
                                                        {"Dark yellow", Qt::darkYellow},
                                                        {"Gray", Qt::gray},
                                                        {"Dark gray", Qt::darkGray},
                                                        {"Light gray", Qt::lightGray}};
    const QString path = QFileDialog::getSaveFileName(this, tr("Save as..."), {}, "PNG Images (*.png)");
    QImageGrid imageGrid;
    imageGrid.setSpacing(ui.spacingSpinBox->value());
    const QColor color(colors.value(ui.comboBox->currentText()));
    if(color == Qt::transparent) {
        imageGrid.setImageFormat(QImage::Format_ARGB32);
    }

    imageGrid.setSpacingColor(color);
    const auto rows = ui.gridWidget->getRowCount();
    for(auto idx = 0; idx < rows; ++idx) {
        const auto cols = ui.gridWidget->getColumnCount(idx);
        for(auto idx2 = 0; idx2 < cols; ++idx2) {
            const QIcon icon = ui.gridWidget->iconAt(idx, idx2);
            imageGrid.addImage(idx, idx2, icon.pixmap(icon.availableSizes().first()).toImage());
        }
    }

    const QImage image = imageGrid.generateImage().scaledToWidth(ui.resizeToWidth->value());
    image.save(path);
}

} // namespace ui
} // namespace vfg
