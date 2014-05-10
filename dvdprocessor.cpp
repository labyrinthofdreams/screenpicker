#include <cctype>
#include <utility>
#include <QString>
#include <QStringList>
#include <QProcess>
#include "dvdprocessor.h"
#include "ptrutil.hpp"

vfg::DvdProcessor::DvdProcessor(QString processorPath, QObject *parent) :
    QObject(parent),
    processor(std::move(processorPath)),
    outputPath("dgindex_tmp"),
    aborted(false)
{
    proc = util::make_unique<QProcess>();

    connect(proc.get(), SIGNAL(readyReadStandardOutput()),
            this,       SLOT(updateDialog()));

    connect(proc.get(), SIGNAL(finished(int)),
            this,       SLOT(handleProcessFinish(int)));

    connect(proc.get(), SIGNAL(error(QProcess::ProcessError)),
            this,       SLOT(handleProcessError(QProcess::ProcessError)));
}

vfg::DvdProcessor::~DvdProcessor() {

}

void vfg::DvdProcessor::process(const QStringList& files)
{
    if(files.empty()) {
        emit error(tr("Nothing to process."));
        return;
    }

    QStringList args;
    args << "-ia" << "5" << "-fo" << "0" << "-yr" << "1" << "-om" << "0"
         << "-hide" << "-exit" << "-o" << outputPath << "-i" << files;

    proc->start(processor, args);
}

void vfg::DvdProcessor::setProcessor(QString executablePath)
{
    processor = std::move(executablePath);
}

void vfg::DvdProcessor::setOutputPath(QString newOutputPath)
{
    outputPath = std::move(newOutputPath);
}

void vfg::DvdProcessor::updateDialog()
{
    // Read last integer value from process output
    const QByteArray rawOutput = proc->readAllStandardOutput();
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

    const int currentProgress = parsedOutput.toInt();
    emit progressUpdate(currentProgress);
}

void vfg::DvdProcessor::handleProcessFinish(const int exitCode)
{
    // Non-zero exit code implies crash / abort
    if(exitCode == 0) {
        emit finished(savedPath());
    }
}

void vfg::DvdProcessor::handleAbortProcess()
{
    aborted = true;
    proc->close();
}

void vfg::DvdProcessor::handleProcessError(const QProcess::ProcessError errorCode)
{
    proc->close();

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

QString vfg::DvdProcessor::savedPath() const
{
    return QString("%1.d2v").arg(outputPath);
}
