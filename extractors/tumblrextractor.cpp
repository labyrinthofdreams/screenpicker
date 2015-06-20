#include <QByteArray>
#include <QObject>
#include <QRegExp>
#include <QStringList>
#include <QUrl>
#include "tumblrextractor.hpp"

// Thanks to:
// https://github.com/rg3/youtube-dl/blob/master/youtube_dl/extractor/tumblr.py
// https://github.com/soimort/you-get/blob/develop/src/you_get/extractors/tumblr.py

namespace {

QUrl getSourceUrl(const QByteArray& html)
{
    const QRegExp sourceRx("<source src=\"([^\"]*)\"");
    if(sourceRx.indexIn(html) == -1) {
        return {};
    }

    return {sourceRx.cap(1)};
}

} // namespace

namespace vfg {
namespace extractor {

TumblrExtractor::TumblrExtractor(QObject *parent) :
    BaseExtractor("tumblr", parent)
{
}

bool TumblrExtractor::isSame(const QUrl& url) const
{
    return url.host().endsWith("tumblr.com");
}

void TumblrExtractor::fetchStreams(const QUrl& givenUrl)
{
    postReply.reset(net->get(createRequest(givenUrl)));
    connect(postReply.get(), SIGNAL(finished()), this, SLOT(postReplyFinished()));
}

QStringList TumblrExtractor::getStreams() const
{
    return {"Default"};
}

void TumblrExtractor::download(const QString& streamName)
{
    Q_UNUSED(streamName);

    redirectReply.reset(net->get(createRequest(url)));
    connect(redirectReply.get(), SIGNAL(finished()), this, SLOT(redirectFinished()));
}

void TumblrExtractor::postReplyFinished()
{
    const QByteArray html = postReply->readAll();
    const QRegExp feedTypeRx("<meta property=\"og:type\" content=\"tumblr-feed:(\\w+)\" \\/>");
    if(feedTypeRx.indexIn(html) == -1) {
        // We might be on the video page already...
        const QUrl sourceUrl = getSourceUrl(html);
        if(sourceUrl.isEmpty()) {
            log("Could not find feed type / source url.");
            return;
        }
        else {
            url = sourceUrl;
            emit streamsReady();
            return;
        }
    }

    const QString feedType = feedTypeRx.cap(1);
    if(feedType == "audio") {
        log("Page contains only audio");
        return;
    }

    // Check for vid.me embedded video
    const QRegExp videoMeRx("src=[\'\"](https?://vid\\.me/[^\'\"]+)[\'\"]");
    if(videoMeRx.indexIn(html) > -1) {
        log("Vid.me embedded videos are not currently supported. Sorry!");
        return;
    }

    const QRegExp videoLinkRx("<iframe src='(https?:\\/\\/www\\.tumblr\\.com\\/video\\/[^\']*)'");
    if(videoLinkRx.indexIn(html) == -1) {
        log("Could not find video URL");
        return;
    }

    const QString videoLink = videoLinkRx.cap(1);
    videoReply.reset(net->get(createRequest(QUrl(videoLink))));
    connect(videoReply.get(), SIGNAL(finished()), this, SLOT(videoReplyFinished()));
}

void TumblrExtractor::videoReplyFinished()
{
    const QByteArray html = videoReply->readAll();
    const QUrl sourceUrl = getSourceUrl(html);
    if(sourceUrl.isEmpty()) {
        log("Could not find source URL");
        return;
    }

    url = sourceUrl;

    emit streamsReady();
}

void TumblrExtractor::redirectFinished()
{
    emit requestReady(createRequest(redirectReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
}

} // namespace extractor
} // namespace vfg
