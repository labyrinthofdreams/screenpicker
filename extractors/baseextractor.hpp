#ifndef BASEEXTRACTOR_HPP
#define BASEEXTRACTOR_HPP

#include <memory>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

class QObject;
class QUrl;

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
     * @brief Process URL (http requests, etc...)
     * @param url URL to process
     */
    virtual void process(const QUrl& url);

protected:
    void log(const QString &msg);

signals:
    /**
     * @brief Emitted when request is ready for the final URL
     * @param request Request
     */
    void requestReady(const QNetworkRequest &request);

    /**
     * @brief Emitted when a log message is created
     * @param msg Message
     */
    void logReady(const QString& msg);
};

} // namespace extractor
} // namespace vfg

#endif // BASEEXTRACTOR_HPP
