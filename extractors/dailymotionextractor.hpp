#ifndef VFG_EXTRACTOR_DAILYMOTIONEXTRACTOR_HPP
#define VFG_EXTRACTOR_DAILYMOTIONEXTRACTOR_HPP

#include <memory>
#include <QNetworkReply>
#include "extractors/baseextractor.hpp"

class QUrl;

namespace vfg {
namespace extractor {

class DailyMotionExtractor : public vfg::extractor::BaseExtractor
{
    Q_OBJECT

private:
    std::unique_ptr<QNetworkReply> reply;
    std::unique_ptr<QNetworkReply> redirectReply;

public:
    DailyMotionExtractor();

    bool isSame(const QUrl& url) const override;

    void process(const QUrl& url) override;

private slots:
    void embedUrlFinished();

    void redirectFinished();
};

} // namespace extractor
} // namespace vfg

#endif // VFG_EXTRACTOR_DAILYMOTIONEXTRACTOR_HPP
