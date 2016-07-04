#ifndef VFG_DVDPROCESSOR_H
#define VFG_DVDPROCESSOR_H

#include <memory>
#include <QObject>
#include <QProcess>

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
    explicit DvdProcessor(const QString &processorPath, QObject *parent = 0);

    /**
     * @brief process Processes a list of VOB files to produce a d2v file
     * @param files Files to process
     */
    void process(const QStringList& files);

    /**
     * @brief Set processor executable path
     * @param executablePath Path to the executable
     */
    void setProcessor(const QString &executablePath);

    /**
     * @brief Set output path
     *
     * Note that the output path must not have a suffix because DGIndex
     * will append .d2v to it automatically
     * @param outputPath Output path without a suffix
     */
    void setOutputPath(const QString &outputPath);

    /**
     * @brief Get output path with proper extension
     * @return Output path as a string
     */
    QString savedPath() const;

public slots:
    /**
     * @brief Handle abort request by stopping the processor
     */
    void handleAbortProcess();
    
signals:
    /**
     * @brief Emits current progress in a range 0-100
     * @param progress Progress as an integer
     */
    void progressUpdate(int progress);

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
    QString outputPath {"dgindex_tmp"};

    //! The interface between the app and the executable
    std::unique_ptr<QProcess> proc {};

    //! Flag to indicate whether the process was aborted by user
    bool aborted {false};

    //! Tracks the last processed value from process output
    //! as dgindex can return values smaller than last value
    int lastProgress {0};

private slots:
    void updateDialog();
    void handleProcessFinish(int exitCode);
    void handleProcessError(QProcess::ProcessError);
    
};

} // namespace vfg

#endif // VFG_DVDPROCESSOR_H
