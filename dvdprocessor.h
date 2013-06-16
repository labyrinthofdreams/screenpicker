#ifndef VFG_DVDPROCESSOR_H
#define VFG_DVDPROCESSOR_H

#include <QObject>
#include <QProcess>
#include <QProgressDialog>

// Forward declarations
class QString;
class QStringList;

namespace vfg {

/**
 * @brief The DvdProcessor class
 * Converts DVD VOBs into D2V files
 */
class DvdProcessor : public QObject
{
    Q_OBJECT
public:
    explicit DvdProcessor(QString processorPath, QObject *parent = 0);
    ~DvdProcessor();
    void process(QStringList files, QString outfile);
    void setProcessor(QString executablePath);
    
signals:
    void finished(QString filename);
    void error(QString errorMsg);
    
public slots:

private:
    QString processor;
    QString outputPath;
    QProcess *proc;
    QProgressDialog progress;

private slots:
    void updateDialog();
    void handleProcessFinish(int exitCode);
    void handleAbortProcess();
    void handleProcessError(QProcess::ProcessError);
    
};

} // namespace vfg

#endif // VFG_DVDPROCESSOR_H
