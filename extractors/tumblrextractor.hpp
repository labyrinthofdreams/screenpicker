#ifndef VFG_EXTRACTOR_TUMBLREXTRACTOR_HPP
#define VFG_EXTRACTOR_TUMBLREXTRACTOR_HPP

#include <memory>
#include <QNetworkReply>
#include <QUrl>
#include "baseextractor.hpp"

class QByteArray;
class QObject;
class QStringList;

namespace vfg {
namespace extractor {

class TumblrExtractor : public vfg::extractor::BaseExtractor
{
    Q_OBJECT

private:
    //! Tumblr post page reply
    std::unique_ptr<QNetworkReply> postReply;

    //! Tumblr video page reply
    std::unique_ptr<QNetworkReply> videoReply;

    //! Tumblr video source redirect reply
    std::unique_ptr<QNetworkReply> redirectReply;

    //! Found video URL
    QUrl url;

public:
    /**
     * @brief Constructor
     * @param parent Owner of the widget
     */
    explicit TumblrExtractor(QObject *parent = 0);

    bool isSame(const QUrl &url) const override;

    void fetchStreams(const QUrl &givenUrl) override;

    QStringList getStreams() const override;

    void download(const QString &streamName) override;

private slots:
    /**
     * @brief Triggered when post page reply has finished
     */
    void postReplyFinished();

    /**
     * @brief Triggered when video page reply has finished
     */
    void videoReplyFinished();

    /**
     * @brief Triggered when video source redirect has finished
     */
    void redirectFinished();
};

} // namespace extractor
} // namespace vfg

#endif // VFG_EXTRACTOR_TUMBLREXTRACTOR_HPP
