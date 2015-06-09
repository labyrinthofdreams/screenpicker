#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>
#include "baseextractor.hpp"

namespace vfg {
namespace extractor {

BaseExtractor::BaseExtractor(const QString& name, QObject *parent) :
    QObject(parent),
    net(new QNetworkAccessManager),
    name(name)
{
}

bool BaseExtractor::isSame(const QUrl &url) const {
    Q_UNUSED(url);

    return true;
}

void BaseExtractor::process(const QUrl &url) {
    emit requestReady(QNetworkRequest(url));
}

void BaseExtractor::log(const QString& msg)
{
    emit logReady(QString("[%1] %2").arg(name).arg(msg));
}

} // namespace extractor
} // namespace vfg
