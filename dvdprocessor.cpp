#include <cctype>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QProgressDialog>
#include <QDebug>
#include "dvdprocessor.h"

namespace vfg {

DvdProcessor::DvdProcessor(QString processorPath, QObject *parent) :
    QObject(parent),
    processor(processorPath),
    outputPath(),
    progress(tr("Processing DVD..."), tr("Abort"), 0, 100)
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
}

void DvdProcessor::process(QStringList files, QString outfile)
{
    outputPath = outfile;
    QStringList args;
    args << "-ia" << "5" << "-fo" << "0" << "-yr" << "1" << "-om" << "0" << "-hide" << "-exit"
             << "-o" << outfile << "-i" << files;

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

    qDebug() << rawOutput << " = " << parsedOutput;

    progress.setValue(parsedOutput.toInt());
}

void DvdProcessor::handleProcessFinish(int exitCode)
{
    progress.setValue(100);
    emit finished(outputPath.append(".d2v"));
}

void DvdProcessor::handleAbortProcess()
{
    proc->close();
    progress.cancel();
}

void DvdProcessor::handleProcessError(QProcess::ProcessError errorCode)
{
    switch(errorCode)
    {
    case QProcess::FailedToStart:
        emit error(tr("DGIndex.exe failed to start. Please check that the program filepath is correct and that you have sufficient permissions."));
        return;
    case QProcess::Crashed:
        emit error(tr("DGIndex.exe crashed. Please try again."));
        return;
    default:
        emit error(tr("Error occurred while running DGIndex.exe. Error code: %1").arg(errorCode));
    }

    proc->close();
    progress.cancel();
}

} // namespace vfg
