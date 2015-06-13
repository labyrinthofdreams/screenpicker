#include <string>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QRegExp>
#include <QString>
#include <QUrl>
#include <QVariant>
#include "dailymotionextractor.hpp"
#include "libs/picojson/picojson.h"

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

DailyMotionExtractor::DailyMotionExtractor(QObject *parent) :
    BaseExtractor("dailymotion", parent),
    reply(),
    redirectReply(),
    foundStreams()
{
}

bool DailyMotionExtractor::isSame(const QUrl& url) const
{
    return url.host() == "dailymotion.com" || url.host() == "www.dailymotion.com";
}

void DailyMotionExtractor::fetchStreams(const QUrl& url)
{
    // Get the video id
    const QRegExp rx("\\/video\\/([^\?]+)");
    if(rx.indexIn(url.toDisplayString()) == -1) {
        log("Invalid URL");
        return;
    }

    // Request the embed page html
    const QUrl embedUrl(QString("http://www.dailymotion.com/embed/video/%1").arg(rx.cap(1)));
    reply.reset(net->get(createRequest(embedUrl)));
    connect(reply.get(), SIGNAL(finished()), this, SLOT(embedUrlFinished()));
}

QStringList DailyMotionExtractor::getStreams() const
{
    return foundStreams.keys();
}

void DailyMotionExtractor::download(const QString& streamName)
{
    // Make a request to the download URL to get the redirected URL
    const QUrl streamUrl = foundStreams.value(streamName);
    if(!streamUrl.isEmpty()) {
        redirectReply.reset(net->get(createRequest(QUrl(streamUrl))));
        connect(redirectReply.get(), SIGNAL(finished()),
                this, SLOT(redirectFinished()));
    }
    else {
        log("Could not find the selected stream");
    }
}

void DailyMotionExtractor::embedUrlFinished()
{
    // Valid streams
    const QList<std::string> streams {"stream_h264_hd1080_url", "stream_h264_hd_url",
                                             "stream_h264_hq_url", "stream_h264_ld_url",
                                             "stream_h264_url"};
    const QString html = reply->readAll();
    // Get the JSON object
    const QRegExp jsonRx("var\\s*info\\s*=\\s*(\\{.+\\}),\\n");
    if(jsonRx.indexIn(html) == -1) {
        log("Could not find stream data");
        return;
    }

    // Parse the JSON object and get all streams
    const std::string parsed = jsonRx.cap(1).toStdString();
    picojson::value json;
    picojson::parse(json, parsed);
    if(json.is<picojson::object>()) {
        for(const std::string& stream : streams) {
            const picojson::value val = json.get(stream);
            if(val.is<std::string>()) {
                foundStreams.insert(QString::fromStdString(stream),
                                    QString::fromStdString(val.get<std::string>()));
            }
        }
    }

    emit streamsReady();
}

void DailyMotionExtractor::redirectFinished() const
{
    emit requestReady(createRequest(redirectReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
}

} // namespace extractor
} // namespace vfg
