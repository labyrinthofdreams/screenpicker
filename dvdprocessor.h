#ifndef VFG_DVDPROCESSOR_H
#define VFG_DVDPROCESSOR_H

#include <memory>
#include <QObject>
#include <QProcess>
#include <QProgressDialog>

// Forward declarations
class QString;
class QStringList;

namespace vfg {

/**
 * @brief The DvdProcessor class converts DVD VOBs into D2V files
 */
class DvdProcessor : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param processorPath Path to the processor executable file
     * @param parent Owner of the object
     */
    explicit DvdProcessor(QString processorPath, QObject *parent = 0);

    /**
     * @brief Destructor
     */
    ~DvdProcessor();

    /**
     * @brief process Processes a list of VOB files to produce a d2v file
     * @param files Files to process
     */
    void process(const QStringList& files);

    /**
     * @brief Set processor executable path
     * @param executablePath Path to the executable
     */
    void setProcessor(QString executablePath);

    /**
     * @brief Get output path with proper extension
     * @return Output path as a string
     */
    QString savedPath() const;
    
signals:
    /**
     * @brief Emits a signal that the processor finished
     * @param filename Path to the processed file
     */
    void finished(const QString& filename);

    /**
     * @brief Emits an error
     * @param errorMsg Error message
     */
    void error(const QString& errorMsg);

private:
    //! Path to the executable processor
    QString processor;
    //! Path to save the processed file
    QString outputPath;
    //! The interface between the app and the executable
    std::unique_ptr<QProcess> proc;
    //! Display progress in a dialog
    QProgressDialog progress;
    //! Flag to indicate whether the process was aborted by user
    bool aborted;

private slots:
    void updateDialog();
    void handleProcessFinish(int exitCode);
    void handleAbortProcess();
    void handleProcessError(QProcess::ProcessError);
    
};

} // namespace vfg

#endif // VFG_DVDPROCESSOR_H
