#include <cctype>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QProgressDialog>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include "dvdprocessor.h"

// TODO: Processing DVD or Blu-ray...

namespace vfg {

DvdProcessor::DvdProcessor(QString processorPath, QObject *parent) :
    QObject(parent),
    processor(processorPath),
    outputPath("dgindex_tmp"),
    progress(tr("Processing DVD..."), tr("Abort"), 0, 100),
    aborted(false)
{
    proc = new QProcess(this);

    connect(proc, SIGNAL(readyReadStandardOutput()),
            this, SLOT(updateDialog()));
    connect(proc, SIGNAL(finished(int)),
            this, SLOT(handleProcessFinish(int)));
    connect(proc, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(handleProcessError(QProcess::ProcessError)));
    connect(&progress, SIGNAL(canceled()),
            this, SLOT(handleAbortProcess()));
}

DvdProcessor::~DvdProcessor()
{
    if(QFile::exists("dgindex_tmp.d2v"))
    {
        QFile::remove("dgindex_tmp.d2v");
    }
}

void DvdProcessor::process(QStringList files)
{
    if(files.empty()) {
        emit error(tr("Nothing to process."));
        return;
    }

    QFileInfo info(files.at(0));
    if(info.suffix() == "m2ts") {
        progress.setLabelText(tr("Processing Blu-ray..."));
    }
    else {
        progress.setLabelText(tr("Processing DVD..."));
    }

    QSettings cfg("config.ini", QSettings::IniFormat);
    if(cfg.value("savedgindexfiles").toBool()) {
        QString out = QFileDialog::getSaveFileName(0, tr("Select DGIndex project output path"),
                                     info.absoluteDir().absoluteFilePath("dgindex_project"));
        if(out.isEmpty()) {
            return;
        }
        outputPath = out;
    }

    QStringList args;
    args << "-ia" << "5" << "-fo" << "0" << "-yr" << "1" << "-om" << "0" << "-hide" << "-exit"
             << "-o" << outputPath << "-i" << files;

    progress.setValue(0);
    progress.setVisible(true);
    proc->start(processor, args);
}

void DvdProcessor::setProcessor(QString executablePath)
{
    processor = executablePath;
}

void DvdProcessor::updateDialog()
{
    // Read last integer value from process output
    QByteArray rawOutput = proc->readAllStandardOutput();
    QString parsedOutput;
    for(int last = rawOutput.size() - 2; last >= 0; --last)
    {
        const char c = rawOutput.at(last);
        if(std::isdigit(c)) {
            parsedOutput.prepend(c);
        }
        else {
            break;
        }
    }

    // DGIndex.exe can sometimes output false values
    const int nextValue = parsedOutput.toInt();
    if(nextValue > progress.value()) {
        progress.setValue(nextValue);
    }
}

void DvdProcessor::handleProcessFinish(int exitCode)
{
    progress.setValue(100);
    // Non-zero exit code implies crash / abort
    if(exitCode == 0) {
        emit finished(outputPath.append(".d2v"));
    }
}

void DvdProcessor::handleAbortProcess()
{
    aborted = true;
    proc->close();
    progress.cancel();
}

void DvdProcessor::handleProcessError(QProcess::ProcessError errorCode)
{
    proc->close();
    progress.cancel();

    switch(errorCode)
    {
    case QProcess::FailedToStart:
        emit error(tr("DGIndex.exe failed to start. Please check that the program filepath is correct and that you have sufficient permissions."));
        break;
    case QProcess::Crashed:
        if(!aborted) {
            emit error(tr("DGIndex.exe crashed. Please try again."));
        }
        else {
            aborted = false;
        }
        break;
    default:
        emit error(tr("Error occurred while running DGIndex.exe. Error code: %1").arg(errorCode));
    }
}

} // namespace vfg
