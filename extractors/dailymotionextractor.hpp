#ifndef VFG_EXTRACTOR_DAILYMOTIONEXTRACTOR_HPP
#define VFG_EXTRACTOR_DAILYMOTIONEXTRACTOR_HPP

#include <memory>
#include <QNetworkReply>
#include "extractors/baseextractor.hpp"

class QObject;
class QUrl;

namespace vfg {
namespace extractor {

/**
 * @brief The DailyMotionExtractor class
 *
 * Given a Dailymotion URL to video gets the raw URL for the video
 */
class DailyMotionExtractor : public vfg::extractor::BaseExtractor
{
    Q_OBJECT

private:
    //! Initial reply
    std::unique_ptr<QNetworkReply> reply;

    //! Redirect reply
    std::unique_ptr<QNetworkReply> redirectReply;

public:
    /**
     * @brief Constructor
     */
    explicit DailyMotionExtractor(QObject *parent = 0);

    bool isSame(const QUrl& url) const override;

    void process(const QUrl& url) override;

private slots:
    /**
     * @brief Triggered after request to the embed page has finished
     */
    void embedUrlFinished();

    /**
     * @brief Triggered after request to the redirect page has finished
     */
    void redirectFinished() const;
};

} // namespace extractor
} // namespace vfg

#endif // VFG_EXTRACTOR_DAILYMOTIONEXTRACTOR_HPP
