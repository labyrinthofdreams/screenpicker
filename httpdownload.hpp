#ifndef VFG_NET_HTTPDOWNLOAD_HPP
#define VFG_NET_HTTPDOWNLOAD_HPP

#include <memory>
#include <QFile>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QTime>
#include <QUrl>

class QNetworkAccessManager;

namespace vfg {
namespace net {

class HttpDownload : public QObject
{
    Q_OBJECT

private:
    //! Download URL
    QUrl url;

    //! Network reply for the request
    std::unique_ptr<QNetworkReply> reply;

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

public:
    /**
     * @brief Default constructor
     */
    HttpDownload();

    /**
     * @brief Constructor
     * @param url Request URL
     * @param parent Owner of the object
     */
    explicit HttpDownload(const QUrl& url, QObject *parent = 0);

    /**
     * @brief Copy constructor
     * @param other Object to copy from
     */
    HttpDownload(const HttpDownload& other);

    /**
     * Default destructor
     */
    ~HttpDownload() = default;

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
