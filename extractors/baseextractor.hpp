#ifndef BASEEXTRACTOR_HPP
#define BASEEXTRACTOR_HPP

#include <memory>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>

class QNetworkRequest;
class QObject;
class QStringList;

namespace vfg {
namespace extractor {

/**
 * @brief The BaseExtractor class
 *
 * Base URL extractor. All extractors must inherit this class.
 */
class BaseExtractor : public QObject
{
    Q_OBJECT

protected:
    //! Network manager
    std::unique_ptr<QNetworkAccessManager> net;

private:
    //! Extractor name
    QString name;

    //! URL
    QUrl url;

public:
    /**
     * @brief Constructor
     * @param name Extractor name
     */
    explicit BaseExtractor(const QString& name = "default", QObject *parent = 0);

    /**
     * @brief Matches URL for the current extractor
     * @param url URL to match
     * @return True if matches, otherwise false
     */
    virtual bool isSame(const QUrl& url) const;

    /**
     * @brief Fetch streams for a given URL
     * @param givenUrl URL to fetch
     */
    virtual void fetchStreams(const QUrl& givenUrl);

    /**
     * @brief Get a list of found streams (names)
     * @return List of found streams
     */
    virtual QStringList getStreams() const;

    /**
     * @brief Get request for selected stream
     * @param streamName Stream to get request for
     */
    virtual void download(const QString& streamName);

protected:
    /**
     * @brief Log message
     * @param msg Message to log
     */
    void log(const QString &msg) const;

    /**
     * @brief Create HTTP request
     * @param url URL to create request for
     * @return HTTP request
     */
    virtual QNetworkRequest createRequest(const QUrl& url) const;

signals:
    /**
     * @brief Emitted when request is ready for the final URL
     * @param request Request
     */
    void requestReady(const QNetworkRequest &request) const;

    /**
     * @brief Emitted when a log message is created
     * @param msg Message
     */
    void logReady(const QString& msg) const;

    /**
     * @brief Emitted when stream URLs have been found
     */
    void streamsReady() const;
};

} // namespace extractor
} // namespace vfg

#endif // BASEEXTRACTOR_HPP
