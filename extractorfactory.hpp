#ifndef VFG_NET_EXTRACTORFACTORY_HPP
#define VFG_NET_EXTRACTORFACTORY_HPP

#include <memory>

class QUrl;

namespace vfg {
namespace extractor {
    class BaseExtractor;
}
}

namespace vfg {
namespace extractor {

class ExtractorFactory
{
public:
    ExtractorFactory() = default;

    std::unique_ptr<vfg::extractor::BaseExtractor> getExtractor(const QUrl& url) const;
};

} // namespace net
} // namespace vfg

#endif // VFG_NET_EXTRACTORFACTORY_HPP
