#include <iostream>
#include <string>
#include <QDebug>
#include <QList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QString>
#include <QUrl>
#include <QVariant>
#include "dailymotionextractor.hpp"
#include "libs/picojson/picojson.h"

namespace vfg {
namespace extractor {

DailyMotionExtractor::DailyMotionExtractor() :
    BaseExtractor()
{
}

bool DailyMotionExtractor::isSame(const QUrl& url) const
{
    return url.host() == "dailymotion.com" || url.host() == "www.dailymotion.com";
}

void DailyMotionExtractor::process(const QUrl& url)
{
    // Get the video id
    QRegExp rx("\\/video\\/([^\?]+)");
    rx.indexIn(url.toDisplayString());
    // Request the embed page html
    QUrl embedUrl(QString("http://www.dailymotion.com/embed/video/%1").arg(rx.cap(1)));
    reply.reset(net->get(QNetworkRequest(embedUrl)));
    connect(reply.get(), SIGNAL(finished()), this, SLOT(embedUrlFinished()));
}

void DailyMotionExtractor::embedUrlFinished()
{
    // Valid streams
    QList<std::string> streams {"stream_h264_hd1080_url", "stream_h264_hd_url", "stream_h264_hq_url",
                           "stream_h264_ld_url", "stream_h264_url"};
    QString html = reply->readAll();
    // Get the JSON object
    QRegExp jsonRx("var\\s*info\\s*=\\s*(\\{.+\\}),\\n");
    if(jsonRx.indexIn(html) == -1) {
        qDebug() << "found nothing";
        return;
    }

    // Parse the JSON object and get the highest quality stream
    QString streamUrl;
    std::string parsed = jsonRx.cap(1).toStdString();
    picojson::value json;
    picojson::parse(json, parsed);
    if (json.is<picojson::object>()) {
        for(const std::string& stream : streams) {
            picojson::value val = json.get(stream);
            if(val.is<std::string>()) {
                streamUrl = QString::fromStdString(val.get<std::string>());
                break;
            }
        }
    }

    // Make a request to the download URL to get the redirected URL
    if(!streamUrl.isEmpty()) {
        redirectReply.reset(net->get(QNetworkRequest(QUrl(streamUrl))));
        connect(redirectReply.get(), SIGNAL(finished()),
                this, SLOT(redirectFinished()));
    }
}

void DailyMotionExtractor::redirectFinished()
{
    emit requestReady(QNetworkRequest(redirectReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
}

} // namespace extractor
} // namespace vfg
