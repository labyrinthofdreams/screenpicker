#ifndef BASEEXTRACTOR_HPP
#define BASEEXTRACTOR_HPP

#include <memory>
#include <QNetworkAccessManager>
#include <QObject>

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

public:
    /**
     * @brief Constructor
     */
    BaseExtractor();

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

signals:
    /**
     * @brief Emitted when request is ready for the final URL
     * @param request Request
     */
    void requestReady(const QNetworkRequest &request);
};

}
}

#endif // BASEEXTRACTOR_HPP
