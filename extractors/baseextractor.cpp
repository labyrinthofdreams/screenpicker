#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>
#include "baseextractor.hpp"

static
QNetworkRequest createRequest(const QUrl& url) {
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Accept"), QByteArray("text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"));
    request.setRawHeader(QByteArray("Accept-Charset"), QByteArray("UTF-8,*;q=0.5"));
    request.setRawHeader(QByteArray("Accept-Language"), QByteArray("en-US,en;q=0.8"));
    request.setRawHeader(QByteArray("User-Agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64; rv:13.0) Gecko/20100101 Firefox/13.0"));
    return request;
}

namespace vfg {
namespace extractor {

BaseExtractor::BaseExtractor(const QString& name, QObject *parent) :
    QObject(parent),
    net(new QNetworkAccessManager),
    name(name)
{
}

bool BaseExtractor::isSame(const QUrl &url) const {
    Q_UNUSED(url);

    return true;
}

void BaseExtractor::fetchStreams(const QUrl &givenUrl) {
    url = givenUrl;

    emit streamsReady();
}

QStringList BaseExtractor::getStreams() const
{
    return {"Default"};
}

void BaseExtractor::download(const QString& streamName)
{
    Q_UNUSED(streamName);

    emit requestReady(createRequest(url));
}

void BaseExtractor::log(const QString& msg) const
{
    emit logReady(QString("[%1] %2").arg(name).arg(msg));
}

} // namespace extractor
} // namespace vfg
