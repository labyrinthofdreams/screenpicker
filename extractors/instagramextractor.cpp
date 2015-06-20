#include <QByteArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QRegExp>
#include <QStringList>
#include <QUrl>
#include "instagramextractor.hpp"

namespace {

QNetworkRequest createRequest(const QUrl& url) {
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Accept"), QByteArray("text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"));
    request.setRawHeader(QByteArray("Accept-Charset"), QByteArray("UTF-8,*;q=0.5"));
    request.setRawHeader(QByteArray("Accept-Language"), QByteArray("en-US,en;q=0.8"));
    request.setRawHeader(QByteArray("User-Agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64; rv:13.0) Gecko/20100101 Firefox/13.0"));
    return request;
}

} // namespace

namespace vfg {
namespace extractor {

InstagramExtractor::InstagramExtractor(QObject *parent) :
    BaseExtractor("instagram", parent),
    reply()
{
}

bool InstagramExtractor::isSame(const QUrl& url) const
{
    return url.host() == "instagram.com" || url.host() == "www.instagram.com";
}

void InstagramExtractor::fetchStreams(const QUrl& url)
{
    reply.reset(net->get(createRequest(url)));
    connect(reply.get(), SIGNAL(finished()), this, SLOT(pageReply()));
}

QStringList InstagramExtractor::getStreams() const
{
    return {"Default"};
}

void InstagramExtractor::download(const QString& streamName)
{
    Q_UNUSED(streamName);

    emit requestReady(createRequest(dlUrl));
}

void InstagramExtractor::pageReply()
{
    const QByteArray html = reply->readAll();
    const QRegExp urlRx("<meta property=\"og:video\" content=\"([^\"]+)\" \\/>");
    if(urlRx.indexIn(html) == -1) {
        log("Could not find stream URL");
        return;
    }

    dlUrl.setUrl(urlRx.cap(1));

    emit streamsReady();
}

} // namespace extractor
} // namespace vfg
