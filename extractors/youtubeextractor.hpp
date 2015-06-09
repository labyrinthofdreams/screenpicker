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

    //! Video ID
    QString videoId;

    //! URL to html5 video player JS file
    QString html5Player;

    //! Encrypted signature
    QString encryptedSig;

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
};

} // namespace extractor
} // namespace vfg

#endif // VFG_EXTRACTOR_YOUTUBEEXTRACTOR_HPP
