#ifndef VFG_UI_X264ENCODERDIALOG_HPP
#define VFG_UI_X264ENCODERDIALOG_HPP

#include <memory>
#include <QDialog>
#include <QProcess>
#include <QSettings>

class QMediaPlayer;
class QString;
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

    void processFinished(int exitCode, QProcess::ExitStatus status);

    void on_buttonSaveAs_clicked();

private:
    Ui::x264EncoderDialog *ui;

    QSettings config;
    QSettings x264config;

    std::unique_ptr<QProcess> x264;

    std::unique_ptr<QMediaPlayer> mediaPlayer;

    QVideoWidget *videoWidget;

    QString parseArgs(const QString& section) const;
};

} // namespace ui
} // namespace vfg

#endif // VFG_UI_X264ENCODERDIALOG_HPP
