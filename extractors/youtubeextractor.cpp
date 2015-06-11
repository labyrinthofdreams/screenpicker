#include <string>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QNetworkRequest>
#include <QObject>
#include <QRegExp>
#include <QScriptEngine>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include "libs/picojson/picojson.h"
#include "youtubeextractor.hpp"

// Thanks to:
// https://github.com/soimort/you-get/blob/develop/src/you_get/extractors/youtube.py
// https://github.com/rg3/youtube-dl/blob/master/youtube_dl/extractor/youtube.py

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

QStringList match(const QString& regex, const QString& text) {
    const QRegExp rx(regex);
    if(rx.indexIn(text) > -1) {
        return rx.capturedTexts();
    }
    else {
        return {};
    }
}

namespace vfg {
namespace extractor {

YoutubeExtractor::YoutubeExtractor(QObject *parent) :
    BaseExtractor("youtube", parent)
{
}

bool YoutubeExtractor::isSame(const QUrl &url) const
{
    return url.host() == "youtube.com" || url.host() == "www.youtube.com" ||
            url.host() == "youtu.be";
}

void YoutubeExtractor::process(const QUrl &url)
{
    // Get video id
    QRegExp videoIdRx("watch\\?v=([a-zA-Z0-9-_]{11})");
    if(videoIdRx.indexIn(url.toDisplayString()) == -1) {
        videoIdRx.setPattern("youtu\\.be\\/([a-zA-Z0-9-_]{11})");
        if(videoIdRx.indexIn(url.toDisplayString()) == -1) {
            log("Invalid URL");
            return;
        }
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
    const auto videoInfo = parseQueryString(data);
    if(!videoInfo.contains("status")) {
        log("Unknown status");
    }
    else if(videoInfo.value("status") == "ok") {
        // If use_cipher_signature is missing or disabled parse stream list
        // from the get_video_info output, otherwise parse it from the video page
        if(!videoInfo.contains("use_cipher_signature")
                || videoInfo.value("use_cipher_signature") == "False") {
            log("use_cipher_signature: Disabled");
            log(QString("Title: ").append(QUrl::fromPercentEncoding(videoInfo.value("title"))));
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
    const QByteArray videoPageHtml = videoPageReply->readAll();
    if(videoPageHtml.contains("player-age-gate-content\">")) {
        log("Found age-restriced video");
        embedPageReply.reset(net->get(createRequest(QUrl(QString("https://www.youtube.com/embed/").append(videoId)))));
        connect(embedPageReply.get(), SIGNAL(finished()), this, SLOT(embedPageFinished()));
    }
    else {
        // Get the ytplayer.config JSON object from the page
        const QRegExp jsonRx("ytplayer.config = (\\{.+\\});");
        if(jsonRx.indexIn(videoPageHtml) == -1) {
            log("Could not find ytplayer.config");
            return;
        }

        // Parse the JSON object and get the stream list
        const QByteArray decoded = parseYtPlayerConfig(jsonRx.cap(1));
        processStreamList(decoded.split(','));
    }
}

void YoutubeExtractor::html5JsFinished()
{
    // The following regexes extract the functions used to decrypt the signature
    const QByteArray js = html5JsReply->readAll();
    const QString f1 = match("\\w+\\.sig\\|\\|([$\\w]+)\\(\\w+\\.\\w+\\)", js).at(1);
    QString f1def = match(QString("(function %1\\(\\w+\\)\\{[^\\{]+\\})").arg(QRegExp::escape(f1)), js).at(1);
    f1def.replace(QRegExp("([$\\w]+\\.)([$\\w]+\\(\\w+,\\d+\\))"), "\\2");
    QString code = f1def;
    const QRegExp a("([$\\w]+)\\(\\w+,\\d+\\)");
    int pos = 0;
    while((pos = a.indexIn(f1def, pos)) != -1) {
        pos += a.matchedLength();
        const QString f2 = a.cap(1);
        const QString f2e = QRegExp::escape(f2);
        QStringList f2def = match(QString("[^$\\w]%1:function\\((\\w+,\\w+)\\)(\\{[^\\{\\}]+\\})").arg(f2e), js);
        if(!f2def.isEmpty()) {
            const QString tmp = QString("function %1(%2)%3}").arg(f2e).arg(f2def.at(1)).arg(f2def.at(2));
            f2def.clear();
            f2def << tmp;
        }
        else {
            f2def = match(QString("[^$\\w]%1:function\\((\\w+)\\)(\\{[^\\{\\}]+\\})").arg(f2e), js);
            f2def = QStringList(QString("function %1(%2,b)%3").arg(f2e).arg(f2def.at(1)).arg(f2def.at(2)));
        }

        code.append(f2def.at(0));
    }

    code.append(QString("var sig=%1(s)").arg(f1));
    code.replace("}}", "}");

    // Evaluate the code to compute the decrypted signature value
    QScriptEngine engine;
    engine.globalObject().setProperty("s", QString(bestStream.value("s")));
    engine.evaluate(code);

    const QString decryptedSignature = engine.globalObject().property("sig").toString();
    const QUrl url(QUrl::fromPercentEncoding(bestStream.value("url")).append("&signature=").append(decryptedSignature));

    log("Downloading...");
    emit requestReady(createRequest(url));
}

void YoutubeExtractor::embedPageFinished()
{
    const QByteArray embedPage = embedPageReply->readAll();
    // Get the ytplayer.config JSON object from the page
    const QRegExp jsonRx("ytplayer.config = (\\{.+\\});");
    if(jsonRx.indexIn(embedPage) == -1) {
        const QStringList m = match("\"assets\":\\{.*\"js\":\"([^\"]+)\".*\\}", embedPage);
        if(m.size() == 2) {
            html5Player = QString("https:").append(m.at(1).replace("\\/", "/"));
        }

        QUrlQuery query;
        query.addQueryItem("video_id", videoId);
        query.addQueryItem("eurl", QString("https://youtube.googleapis.com/v/").append(videoId));
        query.addQueryItem("sts", match("\"sts\"\\s*:\\s*(\\d+)", embedPage).at(1));
        QUrl url("https://www.youtube.com/get_video_info");
        url.setQuery(query);
        embedVideoInfoReply.reset(net->get(createRequest(url)));
        connect(embedVideoInfoReply.get(), SIGNAL(finished()), this, SLOT(embedVideoInfoFinished()));
    }
    else {
        // Parse the JSON object and get the stream list
        const QByteArray decoded = parseYtPlayerConfig(jsonRx.cap(1));
        processStreamList(decoded.split(','));
    }
}

void YoutubeExtractor::embedVideoInfoFinished()
{
    const auto videoInfo = parseQueryString(embedVideoInfoReply->readAll());
    if(!html5Player.isEmpty()) {
        log(QString("Title: ").append(videoInfo.value("title")));
        QByteArray decoded;
        decoded.append(QUrl::fromPercentEncoding(videoInfo.value("url_encoded_fmt_stream_map")));
        bestStream = getBestStream(getValidStreams(decoded.split(',')));
        handleStreamDownload();
    }
    else {
        log("Could not find html5player.js");
    }
}

void YoutubeExtractor::processStreamList(const QList<QByteArray> &streamList)
{
    bestStream = getBestStream(getValidStreams(streamList));
    handleStreamDownload();
}

void YoutubeExtractor::handleStreamDownload()
{
    if(bestStream.contains("sig")) {
        QByteArray url;
        url.append(QUrl::fromPercentEncoding(bestStream.value("url")));
        url.append("&signature=");
        url.append(bestStream.value("signature"));
        log("Downloading...");
        emit requestReady(QNetworkRequest(QUrl(QString(url))));
    }
    else if(bestStream.contains("s")) {
        log("Found encrypted stream. Decrypting...");
        html5JsReply.reset(net->get(createRequest(QUrl(html5Player))));
        connect(html5JsReply.get(), SIGNAL(finished()), this, SLOT(html5JsFinished()));
    }
    else if(!bestStream.empty()) {
        log("Downloading...");
        emit requestReady(QNetworkRequest(QUrl(QUrl::fromPercentEncoding(bestStream.value("url")))));
    }
    else {
        log("No streams to download");
    }
}

QByteArray YoutubeExtractor::parseYtPlayerConfig(const QString& jsonStr)
{
    QByteArray decoded;
    const std::string parsed = jsonStr.toStdString();
    picojson::value json;
    picojson::parse(json, parsed);
    if(json.is<picojson::object>()) {
        const picojson::value args = json.get("args");
        if(args.is<picojson::object>()) {
            const picojson::value title = args.get("title");
            if(title.is<std::string>()) {
                QByteArray ba;
                ba.append(QString::fromStdString(title.get<std::string>()));
                log(QString("Title: ").append(QUrl::fromPercentEncoding(ba)));
                const picojson::value streamMap = args.get("url_encoded_fmt_stream_map");
                if(streamMap.is<std::string>()) {
                    decoded.append(QString::fromStdString(streamMap.get<std::string>()));
                }
            }
            else {
                log("The uploader has not made this video available in your country");
                return decoded;
            }
        }

        const picojson::value assets = json.get("assets");
        if(assets.is<picojson::object>()) {
            const picojson::value js = assets.get("js");
            if(js.is<std::string>()) {
                html5Player = QString("https:").append(QString::fromStdString(js.get<std::string>()));
            }
        }
    }

    return decoded;
}

QMap<QByteArray, QByteArray> YoutubeExtractor::getBestStream(
        const QList<QMap<QByteArray, QByteArray> >& streamList) const
{
    // Select the best quality stream, currently only tries to find
    // the non-dash streams
    // 18 MP4 360p H.264 Baseline 0.5Mbps AAC 96Kbps
    // 22 MP4 720p H.264 High 2-3Mbps AAC 192Kbps
    const QList<int> nonDashStreams {22, 18};
    for(const int i : nonDashStreams) {
        for(const auto& stream : streamList) {
            if(stream.value("itag").toInt() == i) {
                log(QString("Selected stream #%1").arg(QString(stream.value("itag"))));
                return stream;
            }
        }
    }

    return {};
}

QList<QMap<QByteArray, QByteArray> > YoutubeExtractor::getValidStreams(
        const QList<QByteArray>& streamList) const
{
    QList<QMap<QByteArray, QByteArray>> streams;
    log(QString("Found %1 streams").arg(streamList.size()));
    for(const QByteArray& stream : streamList) {
        const QMap<QByteArray, QByteArray> parsed = parseQueryString(stream);
        if(parsed.contains("itag") && parsed.contains("url")) {
            streams.append(parsed);
        }
    }

    return streams;
}

} // namespace extractor
} // namespace vfg
