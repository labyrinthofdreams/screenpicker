#ifndef VFG_UI_GIFMAKERWIDGET_HPP
#define VFG_UI_GIFMAKERWIDGET_HPP

#include <memory>
#include <QDialog>
#include <QSettings>

class QMovie;

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

public slots:
    void showPreview(const QString& path);

private slots:
    void on_spinStartFrame_valueChanged(int arg1);

    void on_spinLastFrame_valueChanged(int arg1);

    void on_buttonAutoDelay_clicked();

    void on_spinSkipFrames_valueChanged(int arg1);

    void on_buttonPreviewGif_clicked();

    void on_buttonSave_clicked();

private:
    Ui::GifMakerWidget *ui;

    std::unique_ptr<QMovie> preview;

    QSettings config;
    QSettings imageMagick;
    QSettings gifsicle;

    int totalFrames() const;

signals:
    void requestPreview(const QString& args, const QString& optArgs);

    void createGif();
};

} // namespace ui
} // namespace vfg

#endif // VFG_UI_GIFMAKERWIDGET_HPP
