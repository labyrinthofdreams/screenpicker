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
    /**
     * Default constructor
     */
    ExtractorFactory() = default;

    /**
     * @brief Get extractor
     * @param url URL to get extractor for
     * @return Extractor for URL
     */
    std::unique_ptr<vfg::extractor::BaseExtractor> getExtractor(const QUrl& url) const;
};

} // namespace extractor
} // namespace vfg

#endif // VFG_NET_EXTRACTORFACTORY_HPP
