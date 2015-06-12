#ifndef VFG_NET_HTTPDOWNLOAD_HPP
#define VFG_NET_HTTPDOWNLOAD_HPP

#include <memory>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTime>

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

class QDir;
class QNetworkAccessManager;
class QString;
class QUrl;

namespace vfg {
namespace net {

class HttpDownload : public QObject
{
    Q_OBJECT

public:
    enum class Status {
        Pending,
        Running,
        Finished,
        Aborted
    };

private:
    //! Network reply for the request
    std::unique_ptr<QNetworkReply> reply;

    //! Network request
    QNetworkRequest request;

    //! Bytes downloaded
    qint64 received;

    //! Bytes total
    qint64 total;

    //! Keeps track of the download time
    QTime timer;

    //! Download duration
    qint64 dlDuration;

    //! Output file
    QFile outFile;

    //! Download status
    Status status;

    //! Used to calculate download speed every second
    QTime speedTimer;

    //! Download speed (bytes per second)
    double speed;

    //! Download amount in the last second
    int downloaded;

public:
    /**
     * @brief Constructor
     * @param url Request URL
     * @param cachePath Path to cache directory
     * @param parent Owner of the object
     */
    explicit HttpDownload(const QNetworkRequest& url, const QDir& cachePath, QObject *parent = 0);

    /**
     * @brief Start request
     * @param netMan Network manager to use
     */
    void start(QNetworkAccessManager* netMan);

    /**
     * @brief Percent completed
     * @return Percent completed
     */
    double percentCompleted() const;

    /**
     * @brief Is the download size known
     * @return True if yes, otherwise false
     */
    bool sizeKnown() const;

    /**
     * @brief Bytes downloaded
     * @return Bytes downloaded
     */
    int bytesDownloaded() const;

    /**
     * @brief Bytes total (size of the download)
     * @return Bytes total
     */
    int bytesTotal() const;

    /**
     * @brief Is download finished
     * @return True if yes, otherwise false
     */
    bool isFinished() const;

    /**
     * @brief Duration of the download
     * @return Duration of the download
     */
    int duration() const;

    /**
     * @brief Filename for the file being downloaded
     * @return Filename
     */
    QString fileName() const;

    /**
     * @brief Abort download
     */
    void abort();

    /**
     * @brief Get reply status
     * @return Reply status
     */
    Status getStatus() const;

    /**
     * @brief Get request URL
     * @return Request URL
     */
    QUrl url() const;

    /**
     * @brief Get download speed
     * @return Download speed
     */
    double downloadSpeed() const;

private slots:
    /**
     * @brief Triggered after download has finished
     */
    void downloadFinished();

signals:
    /**
     * @brief Emitted when data has changed
     */
    void updated();

public slots:
    /**
     * @brief Triggered while downloading data
     * @param bytesReceived Bytes downloaded
     * @param bytesTotal Download file size
     */
    void updateProgress(qint64 bytesReceived, qint64 bytesTotal);

};

} // namespace net
} // namespace vfg

#endif // VFG_NET_HTTPDOWNLOAD_HPP
