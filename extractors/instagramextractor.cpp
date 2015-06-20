#include <QByteArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QRegExp>
#include <QStringList>
#include <QUrl>
#include "instagramextractor.hpp"

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
