#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QProcess>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include <QSize>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>
#include "x264encoderdialog.hpp"
#include "ui_x264encoderdialog.h"

static QString prettySize(double size) {
    if(size < 1000) {
        return QString("%1 B").arg(size);
    }
    size /= 1000;
    if(size < 1000) {
        return QString("%1 KB").arg(QString::number(size, 'f', 2));
    }
    size /= 1000;
    return QString("%1 MB").arg(QString::number(size, 'f', 2));
}

namespace vfg {
namespace ui {

x264EncoderDialog::x264EncoderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::x264EncoderDialog),
    config("config.ini", QSettings::IniFormat),
    x264config("scripts/x264.ini", QSettings::IniFormat),
    x264(new QProcess),
    mediaPlayer(new QMediaPlayer),
    videoLayout(new QVBoxLayout),
    videoWidget(new QVideoWidget)
{
    ui->setupUi(this);

    mediaPlayer->setVideoOutput(videoWidget);

    videoLayout->addWidget(videoWidget);
    ui->groupBoxPreview->setLayout(videoLayout);

    connect(x264.get(), SIGNAL(readyReadStandardOutput()),
            this,       SLOT(parseProcessOutput()));

    connect(x264.get(), SIGNAL(finished(int, QProcess::ExitStatus)),
            this,       SLOT(processFinished(int, QProcess::ExitStatus)));

    ui->comboPreset->addItems(x264config.childGroups());

    ui->plainTextEditPreset->setPlainText(parseArgs(ui->comboPreset->currentText()));
}

x264EncoderDialog::~x264EncoderDialog()
{
    QFile::remove("preview.mkv");

    delete ui;
}

} // namespace ui
} // namespace vfg

void vfg::ui::x264EncoderDialog::on_comboPreset_activated(const QString &arg1)
{
    ui->plainTextEditPreset->setPlainText(parseArgs(arg1));
}

QString vfg::ui::x264EncoderDialog::parseArgs(const QString& section) const
{
    const QSize resolution = config.value("video/resolution").toSize();
    QString args = x264config.value(QString("%1/args").arg(section)).toString();
    args.replace("$quality", QString::number(ui->spinQuality->value()))
            .replace("$fps", QString::number(ui->spinFps->value()))
            .replace("$width", QString::number(resolution.width()))
            .replace("$height", QString::number(resolution.height()));

    // Add tune only if set
    if(ui->comboTune->currentIndex() != 0) {
        args.append(" --tune ").append(ui->comboTune->currentText().toLower());
    }

    return args;
}

void vfg::ui::x264EncoderDialog::parseProcessOutput()
{
    // Get first integer indicating progress from output
    // Matches from output: [3.3%] and [33.3%]
    static const QRegExp rx("^\\[(\\d+)\\.\\d+%\\]");
    const QString out = x264->readAllStandardOutput();
    if(out.at(0) == '[') {
        rx.indexIn(out);
        const int progress = rx.cap(1).toInt();
        ui->labelStatusText->setText(out);
        ui->progressBar->setValue(progress);
    }
}

void vfg::ui::x264EncoderDialog::processFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(status);

    ui->progressBar->setValue(100);
    ui->labelStatusText->setText(tr("Done!"));
    ui->buttonSaveAs->setEnabled(true);

    // Play preview
    const QUrl previewUrl = QUrl::fromLocalFile("preview.mkv");
    const QFileInfo info(previewUrl.toLocalFile());
    ui->labelEncodeSize->setText(prettySize(info.size()));
    videoLayout->setSizeConstraint(QLayout::SetFixedSize);
    videoWidget->setFixedSize(config.value("video/resolution").toSize());
    mediaPlayer->setMedia(previewUrl);
    mediaPlayer->play();
}

void vfg::ui::x264EncoderDialog::on_spinFps_valueChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    ui->plainTextEditPreset->setPlainText(parseArgs(ui->comboPreset->currentText()));
}

void vfg::ui::x264EncoderDialog::on_spinQuality_valueChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    ui->plainTextEditPreset->setPlainText(parseArgs(ui->comboPreset->currentText()));
}

void vfg::ui::x264EncoderDialog::on_comboTune_activated(const QString &arg1)
{
    Q_UNUSED(arg1);

    ui->plainTextEditPreset->setPlainText(parseArgs(ui->comboPreset->currentText()));
}

void vfg::ui::x264EncoderDialog::on_buttonEncode_clicked()
{
    QString args = ui->plainTextEditPreset->toPlainText();
    args.append(" --output preview.mkv ").append(config.value("last_opened_script").toString());

    x264->setProcessChannelMode(QProcess::MergedChannels);
    x264->start(config.value("x264path").toString(), args.split(" "));
}

void vfg::ui::x264EncoderDialog::on_buttonSaveAs_clicked()
{
    const QString savePath = QFileDialog::getSaveFileName(this, tr("Select save path"),
                                                          "", tr("MKV (*.mkv)"));
    if(!savePath.isEmpty() &&
            !QFile::copy("preview.mkv", savePath)) {
        QMessageBox::critical(this, tr("Saving failed"),
                              tr("Try again. If the problem persists, try a new filename."));
    }
}
