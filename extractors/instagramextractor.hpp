#ifndef VFG_EXTRACTOR_INSTAGRAMEXTRACTOR_HPP
#define VFG_EXTRACTOR_INSTAGRAMEXTRACTOR_HPP

#include <memory>
#include <QNetworkReply>
#include <QUrl>
#include "baseextractor.hpp"

class QObject;
class QString;
class QStringList;

namespace vfg {
namespace extractor {

class InstagramExtractor : public vfg::extractor::BaseExtractor
{
    Q_OBJECT

private:
    //! Video page reply
    std::unique_ptr<QNetworkReply> reply;

    //! Download URL
    QUrl dlUrl;

public:
    /**
     * @brief Constructor
     * @param parent Owner of the widget
     */
    explicit InstagramExtractor(QObject *parent = 0);

    bool isSame(const QUrl &url) const override;

    void fetchStreams(const QUrl& url) override;

    QStringList getStreams() const override;

    void download(const QString &streamName) override;

private slots:
    /**
     * @brief Triggered the request to the video page finished
     */
    void pageReply();
};

} // namespace extractor
} // namespace vfg

#endif // VFG_EXTRACTOR_INSTAGRAMEXTRACTOR_HPP
