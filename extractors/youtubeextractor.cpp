#include <algorithm>
#include <string>
#include <QByteArray>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QNetworkRequest>
#include <QRegExp>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include "libs/picojson/picojson.h"
#include "youtubeextractor.hpp"

// Thanks to: https://github.com/soimort/you-get/blob/develop/src/you_get/extractors/youtube.py

template <class T>
static
QMap<T, T> parseQueryString(const T& query) {
    QMap<T, T> entries;
    const QList<T> items = query.split('&');
    for(const T& item : items) {
        const QList<T> pair = item.split('=');
        entries.insert(pair[0], pair[1]);
    }

    return entries;
}

QNetworkRequest createRequest(const QUrl& url) {
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Accept"), QByteArray("text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"));
    request.setRawHeader(QByteArray("Accept-Charset"), QByteArray("UTF-8,*;q=0.5"));
    request.setRawHeader(QByteArray("Accept-Language"), QByteArray("en-US,en;q=0.8"));
    request.setRawHeader(QByteArray("User-Agent"), QByteArray("Mozilla/5.0 (X11; Linux x86_64; rv:13.0) Gecko/20100101 Firefox/13.0"));
    return request;
}

QNetworkRequest makeYoutubeRequest(const QString& videoId) {
    QUrl ytUrl("https://www.youtube.com/watch");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("v", videoId);
    ytUrl.setQuery(urlQuery);
    return createRequest(ytUrl);
}

namespace vfg {
namespace extractor {

YoutubeExtractor::YoutubeExtractor(QObject *parent) :
    BaseExtractor(parent)
{
}

bool YoutubeExtractor::isSame(const QUrl &url) const
{
    return url.host() == "youtube.com" || url.host() == "www.youtube.com";
}

void YoutubeExtractor::process(const QUrl &url)
{
    // Get video id
    const QRegExp videoIdRx("watch\\?v=([a-zA-Z0-9-_]{11})");
    if(videoIdRx.indexIn(url.toDisplayString()) == -1) {
        log("Invalid URL");
        return;
    }

    videoId = videoIdRx.cap(1);
    QUrl ytUrl("https://www.youtube.com/get_video_info");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("video_id", videoId);
    ytUrl.setQuery(urlQuery);

    videoInfoReply.reset(net->get(createRequest(ytUrl)));
    connect(videoInfoReply.get(), SIGNAL(finished()), this, SLOT(videoInfoFinished()));
}

void YoutubeExtractor::videoInfoFinished()
{
    // Parse get_video_info output
    const QByteArray data = videoInfoReply->readAll();
    const QMap<QByteArray, QByteArray> videoInfo = parseQueryString(data);
    if(!videoInfo.contains("status")) {
        log("Unknown status");
    }
    else if(videoInfo.value("status") == "ok") {
        // If use_cipher_signature is missing or disabled parse stream list
        // from the get_video_info output, otherwise parse it from the video page
        if(!videoInfo.contains("use_cipher_signature")
                || videoInfo.value("use_cipher_signature") == "False") {
            log("use_cipher_signature: Disabled");
            log(QString("Title: ").append(videoInfo.value("title")));
            QByteArray decoded;
            decoded.append(QUrl::fromPercentEncoding(videoInfo.value("url_encoded_fmt_stream_map")));
            processStreamList(decoded.split(','));
        }
        else {
            videoPageReply.reset(net->get(makeYoutubeRequest(videoId)));
            connect(videoPageReply.get(), SIGNAL(finished()),
                    this, SLOT(videoPageFinished()));
        }
    }
    else if(videoInfo.value("status") == "fail") {
        if(videoInfo.value("errorcode") == "150") {
            videoPageReply.reset(net->get(makeYoutubeRequest(videoId)));
            connect(videoPageReply.get(), SIGNAL(finished()),
                    this, SLOT(videoPageFinished()));
        }
        else if(videoInfo.value("errorcode") == "100") {
            log("This video does not exist");
        }
        else {
            log(QString("Fail: ").append(videoInfo.value("reason")));
        }
    }
    else {
        log(QString("Invalid status: ").append(videoInfo.value("status")));
        log(QString("Error code:").append(videoInfo.value("errorcode")));
    }
}

void YoutubeExtractor::videoPageFinished()
{
    // Get the ytplayer.config JSON object from the page
    const QByteArray data = videoPageReply->readAll();
    const QRegExp jsonRx("ytplayer.config = (.+);");
    if(jsonRx.indexIn(data) == -1) {
        log("Could not find ytplayer.config");
        return;
    }

    // Parse the JSON object and get the stream list
    const std::string parsed = jsonRx.cap(1).toStdString();
    picojson::value json;
    picojson::parse(json, parsed);
    if (json.is<picojson::object>()) {
        const picojson::value args = json.get("args");
        if(args.is<picojson::object>()) {
            const picojson::value title = args.get("title");
            if(title.is<std::string>()) {
                log(QString("Title: ").append(QString::fromStdString(title.get<std::string>())));
                const picojson::value streamMap = args.get("url_encoded_fmt_stream_map");
                if(streamMap.is<std::string>()) {
                    QByteArray decoded;
                    decoded.append(QString::fromStdString(streamMap.get<std::string>()));
                    processStreamList(decoded.split(','));
                }
            }
            else {
                log("The uploader has not made this video available in your country");
            }
        }
    }
}

void YoutubeExtractor::processStreamList(const QList<QByteArray> &streamList)
{
    // Get all valid streams from the streamList
    QList<QMap<QByteArray, QByteArray>> streams;
    log(QString("Found %1 streams").arg(streamList.size()));
    for(const QByteArray& stream : streamList) {
        const QMap<QByteArray, QByteArray> parsed = parseQueryString(stream);
        if(parsed.contains("itag") && parsed.contains("url")) {
            streams.append(parsed);
        }
    }

    // Select the best quality stream, currently only tries to find
    // the non-dash streams
    // 18 MP4 360p H.264 Baseline 0.5Mbps AAC 96Kbps
    // 22 MP4 720p H.264 High 2-3Mbps AAC 192Kbps
    const QList<int> nonDashStreams {22, 18};
    QMap<QByteArray, QByteArray> best;
    for(const int i : nonDashStreams) {
        for(const auto& stream : streams) {
            if(stream.value("itag").toInt() == i) {
                log(QString("Selected stream #%1").arg(QString(stream.value("itag"))));
                best = stream;
                break;
            }
        }

        if(!best.empty()) {
            break;
        }
    }

    if(best.contains("sig")) {
        QByteArray url = best.value("url");
        url.append("&signature=");
        url.append(best.value("signature"));
        emit requestReady(QNetworkRequest(QUrl(QString(url))));
    }
    else if(best.contains("s")) {
        log("Encrypted videos are not currently supported");
    }
    else if(!best.empty()) {
        emit requestReady(QNetworkRequest(QUrl(QUrl::fromPercentEncoding(best.value("url")))));
    }
    else {
        log("No streams to download");
    }
}

} // namespace extractor
} // namespace vfg
