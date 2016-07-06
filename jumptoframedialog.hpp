#ifndef VFG_UI_JUMPTOFRAMEDIALOG_HPP
#define VFG_UI_JUMPTOFRAMEDIALOG_HPP

#include <QDialog>
#include "ui_jumptoframedialog.h"

namespace vfg {
namespace ui {

enum class TimeFormat {
    Time,
    Frames
};

class JumpToFrameDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Owner of the widget
     */
    explicit JumpToFrameDialog(QWidget *parent = 0);

private slots:

    void on_frame_clicked();

    void on_time_clicked();

    void on_goButton_clicked();

signals:
    void jumpTo(int position, TimeFormat timeFormat);

private:
    Ui::JumpToFrameDialog ui;
};


} // namespace ui
} // namespace vfg
#endif // VFG_UI_JUMPTOFRAMEDIALOG_HPP
