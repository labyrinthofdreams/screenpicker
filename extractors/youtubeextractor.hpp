#ifndef VFG_EXTRACTOR_YOUTUBEEXTRACTOR_HPP
#define VFG_EXTRACTOR_YOUTUBEEXTRACTOR_HPP

#include <memory>
#include <QByteArray>
#include <QMap>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStringList>
#include "extractors/baseextractor.hpp"

class QUrl;

namespace vfg {
namespace extractor {

/**
 * @brief The YoutubeExtractor class
 *
 * Given a YouTube url downloads the highest quality stream
 *
 * Currently supports at least non-encrypted streams,
 * encrypted streams and age-restricted streams
 */
class YoutubeExtractor : public vfg::extractor::BaseExtractor
{
    Q_OBJECT

private:
    //! get_video_info reply
    std::unique_ptr<QNetworkReply> videoInfoReply;

    //! watch?v=xyz page reply
    std::unique_ptr<QNetworkReply> videoPageReply;

    //! HTML5 JS video player file reply
    std::unique_ptr<QNetworkReply> html5JsReply;

    //! Embed page reply
    std::unique_ptr<QNetworkReply> embedPageReply;

    //! Embed page video info reply
    std::unique_ptr<QNetworkReply> embedVideoInfoReply;

    //! Video ID
    QString videoId;

    //! URL to html5 video player JS file
    QString html5Player;

    //! Highest quality stream
    QMap<QByteArray, QByteArray> bestStream;

public:
    /**
     * @brief Constructor
     * @param parent Owner of the widget
     */
    explicit YoutubeExtractor(QObject *parent = 0);

    bool isSame(const QUrl &url) const override;

    void process(const QUrl &url) override;

private:
    /**
     * @brief Process stream list
     * @param streamList Stream list to process
     */
    void processStreamList(const QList<QByteArray> &streamList);

    /**
     * @brief Get the highest quality stream from given streamList
     * @param streamList Stream list to check
     * @return Highest quality stream
     */
    QMap<QByteArray, QByteArray> getBestStream(const QList<QMap<QByteArray, QByteArray>>& streamList) const;

    /**
     * @brief Get a parsed list of valid streams from streamList
     * @param streamList List of streams
     * @return List of parsed and valid streams
     */
    QList<QMap<QByteArray, QByteArray>> getValidStreams(const QList<QByteArray>& streamList) const;

    /**
     * @brief Finalizes the stream URL before downloading it
     */
    void handleStreamDownload();

    /**
     * @brief Parses JSON object jsonStr returning url_encoded_fmt_stream_map field
     * @param jsonStr JSON object to parse
     */
    QByteArray parseYtPlayerConfig(const QString& jsonStr);

private slots:
    /**
     * @brief get_video_info request finished
     */
    void videoInfoFinished();

    /**
     * @brief watch?v=xyz request finished
     */
    void videoPageFinished();

    /**
     * @brief HTML5 player JS file finished downloading
     */
    void html5JsFinished();

    /**
     * @brief Embed page finished downloading
     */
    void embedPageFinished();

    /**
     * @brief Embed page video info finished downloading
     */
    void embedVideoInfoFinished();
};

} // namespace extractor
} // namespace vfg

#endif // VFG_EXTRACTOR_YOUTUBEEXTRACTOR_HPP
