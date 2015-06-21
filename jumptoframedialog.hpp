#ifndef VFG_UI_JUMPTOFRAMEDIALOG_HPP
#define VFG_UI_JUMPTOFRAMEDIALOG_HPP

#include <QDialog>

namespace vfg {
namespace ui {

namespace Ui {
class JumpToFrameDialog;
}

class JumpToFrameDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Owner of the widget
     */
    explicit JumpToFrameDialog(QWidget *parent = 0);

    /**
     * Destructor
     */
    ~JumpToFrameDialog();

private slots:

    void on_frame_clicked();

    void on_time_clicked();

    void on_goButton_clicked();

private:
    Ui::JumpToFrameDialog *ui;
};


} // namespace ui
} // namespace vfg
#endif // VFG_UI_JUMPTOFRAMEDIALOG_HPP
