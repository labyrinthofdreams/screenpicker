#ifndef VFG_UI_GIFMAKERWIDGET_HPP
#define VFG_UI_GIFMAKERWIDGET_HPP

#include <QDialog>

namespace vfg {
namespace ui {

namespace Ui {
class GifMakerWidget;
}

class GifMakerWidget : public QDialog
{
    Q_OBJECT

public:
    explicit GifMakerWidget(QWidget *parent = 0);
    ~GifMakerWidget();

    void updateStartFrame(int value);

    void updateLastFrame(int value);

private slots:
    void on_spinStartFrame_valueChanged(int arg1);

    void on_spinLastFrame_valueChanged(int arg1);

    void on_buttonAutoDelay_clicked();

    void on_buttonBrowse_clicked();

    void on_spinSkipFrames_valueChanged(int arg1);

private:
    Ui::GifMakerWidget *ui;

    int totalFrames() const;
};

} // namespace ui
} // namespace vfg

#endif // VFG_UI_GIFMAKERWIDGET_HPP
