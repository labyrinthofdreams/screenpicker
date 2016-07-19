#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>
#include "x264encoderdialog.hpp"

namespace {

QString prettySize(double size) {
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

} // namespace

namespace vfg {
namespace ui {

x264EncoderDialog::x264EncoderDialog(QWidget *parent) :
    QDialog(parent),
    x264(new QProcess),
    mediaPlayer(new QMediaPlayer),
    videoLayout(new QVBoxLayout),
    videoWidget(new QVideoWidget),
    logWindow(new QPlainTextEdit)
{
    ui.setupUi(this);

    logWindow->setWindowTitle(tr("x264 Log"));
    logWindow->resize(400, 500);

    mediaPlayer->setVideoOutput(videoWidget);

    videoLayout->addWidget(videoWidget);
    ui.groupBoxPreview->setLayout(videoLayout);

    connect(x264.get(), &QProcess::readyReadStandardOutput,
            this,       &x264EncoderDialog::parseProcessOutput);

    connect(x264.get(), &QProcess::started,
            this,       &x264EncoderDialog::processStarted);

    connect(x264.get(), static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this,       &x264EncoderDialog::processFinished);

    ui.comboPreset->addItems(x264config.childGroups());

    ui.plainTextEditPreset->setPlainText(parseArgs(ui.comboPreset->currentText()));
}

x264EncoderDialog::~x264EncoderDialog()
{
    QFile::remove(previewFile.absoluteFilePath());
}

void x264EncoderDialog::on_comboPreset_activated(const QString &arg1)
{
    ui.plainTextEditPreset->setPlainText(parseArgs(arg1));
}

QString x264EncoderDialog::parseArgs(const QString& section) const
{
    const QSize resolution = config.value("video/resolution").toSize();
    QString args = x264config.value(QString("%1/args").arg(section)).toString();
    args.replace("$quality", QString::number(ui.spinQuality->value()))
            .replace("$width", QString::number(resolution.width()))
            .replace("$height", QString::number(resolution.height()));

    // Add tune only if set
    if(ui.comboTune->currentIndex() != 0) {
        args.append(" --tune ").append(ui.comboTune->currentText().toLower());
    }

    // Add FPS only if auto-detect is not set
    if(!ui.checkBoxFpsAutoDetect->isChecked()) {
        args.append(" --fps ").append(QString::number(ui.spinFps->value()));
    }

    return args;
}

void x264EncoderDialog::parseProcessOutput()
{
    // Get first integer indicating progress from output
    // Matches from output: [3.3%] and [33.3%]
    static const QRegExp rx("^\\[(\\d+)\\.\\d+%\\]");
    const QString out = x264->readAllStandardOutput();
    const QStringList splitLines = out.split("\n");
    for(int i = 0; i < splitLines.size(); ++i) {
        const QString& current = splitLines.at(i);
        if(current.isEmpty()) {
            continue;
        }
        if(current.at(0) != '[') {
            logWindow->appendPlainText(current);
        }
        else {
            rx.indexIn(current);
            const int progress = rx.cap(1).toInt();
            ui.labelStatusText->setText(current);
            ui.progressBar->setValue(progress);
        }
    }
}

void x264EncoderDialog::processStarted()
{
    logWindow->clear();
    logWindow->show();

    ui.buttonStopEncode->setEnabled(true);
}

void x264EncoderDialog::processFinished(const int exitCode, const QProcess::ExitStatus status)
{
    ui.buttonStopEncode->setEnabled(false);

    if(exitCode == -1) {
        ui.labelStatusText->setText("Exited with error (see log window)");
        ui.progressBar->setValue(0);
        logWindow->appendPlainText("\nProcess exited.");

        return;
    }
    else if(status == QProcess::CrashExit) {
        ui.labelStatusText->setText("Cancelled");
        ui.progressBar->setValue(0);
        logWindow->appendPlainText("\nProcess cancelled.");

        return;
    }

    ui.progressBar->setValue(100);
    ui.labelStatusText->setText(tr("Done!"));
    ui.buttonSaveAs->setEnabled(true);

    previewFile.refresh();
    ui.labelEncodeSize->setText(prettySize(previewFile.size()));

    videoLayout->setSizeConstraint(QLayout::SetFixedSize);
    videoWidget->setFixedSize(config.value("video/resolution").toSize());

    mediaPlayer->setMedia(QUrl::fromLocalFile(previewFile.absoluteFilePath()));
    mediaPlayer->play();
}

void x264EncoderDialog::on_spinFps_valueChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    ui.plainTextEditPreset->setPlainText(parseArgs(ui.comboPreset->currentText()));
}

void x264EncoderDialog::on_spinQuality_valueChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    ui.plainTextEditPreset->setPlainText(parseArgs(ui.comboPreset->currentText()));
}

void x264EncoderDialog::on_comboTune_activated(const QString &arg1)
{
    Q_UNUSED(arg1);

    ui.plainTextEditPreset->setPlainText(parseArgs(ui.comboPreset->currentText()));
}

void x264EncoderDialog::on_buttonEncode_clicked()
{
    QStringList args = ui.plainTextEditPreset->toPlainText().split(" ");
    args << "--output" << previewFile.absoluteFilePath()
         << config.value("last_opened_script").toString();

    x264->setProcessChannelMode(QProcess::MergedChannels);
    x264->start(config.value("x264path").toString(), args);
}

void x264EncoderDialog::on_buttonSaveAs_clicked()
{
    const QString savePath = QFileDialog::getSaveFileName(this, tr("Select save path"),
                                                          "", tr("MKV (*.mkv)"));
    if(!savePath.isEmpty() &&
            !QFile::copy(previewFile.absoluteFilePath(), savePath)) {
        QMessageBox::critical(this, tr("Saving failed"),
                              tr("Try again. If the problem persists, try a new filename."));
    }
}

void x264EncoderDialog::on_checkBoxFpsAutoDetect_toggled(const bool checked)
{
    ui.spinFps->setEnabled(!checked);
    ui.plainTextEditPreset->setPlainText(parseArgs(ui.comboPreset->currentText()));
}

void x264EncoderDialog::on_buttonStopEncode_clicked()
{
    x264->kill();
}

} // namespace ui
} // namespace vfg
