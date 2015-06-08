#include <memory>
#include <QUrl>
#include "extractors/baseextractor.hpp"
#include "extractors/dailymotionextractor.hpp"
#include "extractorfactory.hpp"

namespace vfg {
namespace extractor {

std::unique_ptr<vfg::extractor::BaseExtractor> ExtractorFactory::getExtractor(const QUrl& url) const
{
    std::unique_ptr<vfg::extractor::BaseExtractor> out;
    out.reset(new vfg::extractor::DailyMotionExtractor);
    if(out->isSame(url)) {
        return out;
    }

    out.reset(new vfg::extractor::BaseExtractor);
    return out;
}

} // namespace net
} // namespace vfg
