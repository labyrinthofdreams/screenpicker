#ifndef VFG_UI_X264ENCODERDIALOG_HPP
#define VFG_UI_X264ENCODERDIALOG_HPP

#include <memory>
#include <QDialog>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include "ui_x264encoderdialog.h"

class QMediaPlayer;
class QPlainTextEdit;
class QString;
class QVBoxLayout;
class QVideoWidget;

namespace vfg {
namespace ui {

namespace Ui {
class x264EncoderDialog;
}

class x264EncoderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit x264EncoderDialog(QWidget *parent = 0);

    ~x264EncoderDialog();

private slots:
    void on_comboPreset_activated(const QString &arg1);

    void on_spinFps_valueChanged(const QString &arg1);

    void on_spinQuality_valueChanged(const QString &arg1);

    void on_comboTune_activated(const QString &arg1);

    void on_buttonEncode_clicked();

    void parseProcessOutput();

    void processStarted();

    void processFinished(int exitCode, QProcess::ExitStatus status);

    void on_buttonSaveAs_clicked();

    void on_checkBoxFpsAutoDetect_toggled(bool checked);

    void on_buttonStopEncode_clicked();

private:
    Ui::x264EncoderDialog ui;

    std::unique_ptr<QProcess> x264;

    std::unique_ptr<QMediaPlayer> mediaPlayer;

    QVBoxLayout *videoLayout;
    QVideoWidget *videoWidget;

    std::unique_ptr<QPlainTextEdit> logWindow;

    QSettings config {"config.ini", QSettings::IniFormat};
    QSettings x264config {"scripts/x264.ini", QSettings::IniFormat};
    QFileInfo previewFile {"preview.mkv"};

    QString parseArgs(const QString& section) const;
};

} // namespace ui
} // namespace vfg

#endif // VFG_UI_X264ENCODERDIALOG_HPP
