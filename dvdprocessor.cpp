#include <cctype>
#include <utility>
#include <QString>
#include <QStringList>
#include <QProcess>
#include "dvdprocessor.h"
#include "ptrutil.hpp"

namespace vfg {

DvdProcessor::DvdProcessor(const QString &processorPath, QObject *parent) :
    QObject(parent),
    processor(processorPath),
    proc(util::make_unique<QProcess>())
{
    connect(proc.get(), &QProcess::readyReadStandardOutput,
            this,       &DvdProcessor::updateDialog);

    connect(proc.get(), static_cast<void(QProcess::*)(int)>(&QProcess::finished),
            this,       &DvdProcessor::handleProcessFinish);

    connect(proc.get(), static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            this,       &DvdProcessor::handleProcessError);
}

void DvdProcessor::process(const QStringList& files)
{
    if(files.empty()) {
        emit error(tr("Nothing to process."));
        return;
    }

    lastProgress = 0;

    QStringList args;
    args << "-ia" << "5" << "-fo" << "0" << "-yr" << "1" << "-om" << "0"
         << "-hide" << "-exit" << "-o" << outputPath << "-i" << files;

    proc->start(processor, args);
}

void DvdProcessor::setProcessor(const QString &executablePath)
{
    processor = executablePath;
}

void DvdProcessor::setOutputPath(const QString &newOutputPath)
{
    outputPath = newOutputPath;
}

void DvdProcessor::updateDialog()
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
    if(currentProgress < lastProgress) {
        return;
    }

    lastProgress = currentProgress;
    emit progressUpdate(currentProgress);
}

void DvdProcessor::handleProcessFinish(const int exitCode)
{
    // Non-zero exit code implies crash / abort
    if(exitCode == 0) {
        emit finished(savedPath());
    }
}

void DvdProcessor::handleAbortProcess()
{
    aborted = true;
    proc->close();
}

void DvdProcessor::handleProcessError(const QProcess::ProcessError errorCode)
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

QString DvdProcessor::savedPath() const
{
    return QString("%1.d2v").arg(outputPath);
}

} // namespace vfg
