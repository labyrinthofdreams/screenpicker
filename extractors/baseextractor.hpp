#ifndef BASEEXTRACTOR_HPP
#define BASEEXTRACTOR_HPP

#include <memory>
#include <QNetworkAccessManager>
#include <QObject>

class QUrl;

namespace vfg {
namespace extractor {

class BaseExtractor : public QObject
{
    Q_OBJECT

protected:
    std::unique_ptr<QNetworkAccessManager> net;

public:
    BaseExtractor();

    virtual bool isSame(const QUrl& url) const;

    virtual void process(const QUrl& url);

signals:
    void requestReady(const QNetworkRequest &request);
};

}
}

#endif // BASEEXTRACTOR_HPP
