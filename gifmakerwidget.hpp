#ifndef VFG_UI_GIFMAKERWIDGET_HPP
#define VFG_UI_GIFMAKERWIDGET_HPP

#include <memory>
#include <QDialog>
#include <QSettings>
#include "ui_gifmakerwidget.h"

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

    void on_buttonReset_clicked();

    void on_comboImageMagick_activated(const QString &arg1);

    void on_spinFrameDelay_valueChanged(const QString &arg1);

private:
    Ui::GifMakerWidget ui;

    //! Last generated preview
    std::unique_ptr<QMovie> preview;

    //! Previously generated preview for comparison
    std::unique_ptr<QMovie> previousPreview;

    QSettings config {"config.ini", QSettings::IniFormat};
    QSettings imageMagick {"scripts/imagemagick.ini", QSettings::IniFormat};
    QSettings gifsicle {"scripts/gifsicle.ini", QSettings::IniFormat};

    int totalFrames() const;

    QString parseImageMagickArgs(const QString& argName);

signals:
    void requestPreview(const QString& args, const QString& optArgs);

    void createGif();
};

} // namespace ui
} // namespace vfg

#endif // VFG_UI_GIFMAKERWIDGET_HPP
