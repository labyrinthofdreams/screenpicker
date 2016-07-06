#include <QSettings>
#include "jumptoframedialog.hpp"

namespace vfg {
namespace ui {

JumpToFrameDialog::JumpToFrameDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
}

void JumpToFrameDialog::on_frame_clicked()
{
    ui.stackedWidget->setCurrentIndex(0);
}

void JumpToFrameDialog::on_time_clicked()
{
    ui.stackedWidget->setCurrentIndex(1);
}

void JumpToFrameDialog::on_goButton_clicked()
{
    if(ui.time->isChecked()) {
        const auto time = ui.hours->value() * 60 * 60 +
                 ui.minutes->value() * 60 +
                 ui.seconds->value();
        emit jumpTo(time, TimeFormat::Time);
    }
    else {
        emit jumpTo(ui.frameSpinBox->value(), TimeFormat::Frames);
    }

    accept();
}

} // namespace ui
} // namespace vfg
