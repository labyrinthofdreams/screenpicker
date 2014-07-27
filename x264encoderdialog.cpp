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

vfg::ui::x264EncoderDialog::x264EncoderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::x264EncoderDialog),
    config("config.ini", QSettings::IniFormat),
    x264config("scripts/x264.ini", QSettings::IniFormat),
    x264(new QProcess),
    mediaPlayer(new QMediaPlayer),
    videoLayout(new QVBoxLayout),
    videoWidget(new QVideoWidget),
    logWindow(new QPlainTextEdit)
{
    ui->setupUi(this);

    logWindow->setWindowTitle(tr("x264 Log"));
    logWindow->resize(400, 500);

    mediaPlayer->setVideoOutput(videoWidget);

    videoLayout->addWidget(videoWidget);
    ui->groupBoxPreview->setLayout(videoLayout);

    connect(x264.get(), SIGNAL(readyReadStandardOutput()),
            this,       SLOT(parseProcessOutput()));

    connect(x264.get(), SIGNAL(started()),
            this,       SLOT(processStarted()));

    connect(x264.get(), SIGNAL(finished(int, QProcess::ExitStatus)),
            this,       SLOT(processFinished(int, QProcess::ExitStatus)));

    ui->comboPreset->addItems(x264config.childGroups());

    ui->plainTextEditPreset->setPlainText(parseArgs(ui->comboPreset->currentText()));
}

vfg::ui::x264EncoderDialog::~x264EncoderDialog()
{
    QFile::remove("preview.mkv");

    delete ui;
}

void vfg::ui::x264EncoderDialog::on_comboPreset_activated(const QString &arg1)
{
    ui->plainTextEditPreset->setPlainText(parseArgs(arg1));
}

QString vfg::ui::x264EncoderDialog::parseArgs(const QString& section) const
{
    const QSize resolution = config.value("video/resolution").toSize();
    QString args = x264config.value(QString("%1/args").arg(section)).toString();
    args.replace("$quality", QString::number(ui->spinQuality->value()))
            .replace("$width", QString::number(resolution.width()))
            .replace("$height", QString::number(resolution.height()));

    // Add tune only if set
    if(ui->comboTune->currentIndex() != 0) {
        args.append(" --tune ").append(ui->comboTune->currentText().toLower());
    }

    // Add FPS only if auto-detect is not set
    if(!ui->checkBoxFpsAutoDetect->isChecked()) {
        args.append(" --fps ").append(QString::number(ui->spinFps->value()));
    }

    return args;
}

void vfg::ui::x264EncoderDialog::parseProcessOutput()
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
            ui->labelStatusText->setText(current);
            ui->progressBar->setValue(progress);
        }
    }
}

void vfg::ui::x264EncoderDialog::processStarted()
{
    logWindow->clear();
    logWindow->show();
}

void vfg::ui::x264EncoderDialog::processFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(status);

    ui->progressBar->setValue(100);
    ui->labelStatusText->setText(tr("Done!"));
    ui->buttonSaveAs->setEnabled(true);

    logWindow->show();

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

void vfg::ui::x264EncoderDialog::on_checkBoxFpsAutoDetect_toggled(const bool checked)
{
    ui->spinFps->setEnabled(!checked);
    ui->plainTextEditPreset->setPlainText(parseArgs(ui->comboPreset->currentText()));
}
