#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>
#include "baseextractor.hpp"

namespace vfg {
namespace extractor {

BaseExtractor::BaseExtractor(QObject *parent) :
    QObject(parent),
    net(new QNetworkAccessManager)
{
}

bool BaseExtractor::isSame(const QUrl &url) const {
    Q_UNUSED(url);

    return true;
}

void BaseExtractor::process(const QUrl &url) {
    emit requestReady(QNetworkRequest(url));
}

} // namespace extractor
} // namespace vfg
