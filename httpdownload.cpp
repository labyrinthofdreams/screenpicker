#include <memory>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTime>
#include <QUrl>
#include <QVariant>
#include "httpdownload.hpp"

namespace vfg {
namespace net {

HttpDownload::HttpDownload(const QNetworkRequest& request, const QDir& cachePath, QObject *parent) :
    QObject(parent),
    reply(nullptr),
    request(request),
    received(0),
    total(0),
    timer(),
    outFile(cachePath.absoluteFilePath(request.url().fileName())),
    status(Status::Pending),
    speedTimer(),
    speed(),
    downloaded(0)
{
    if(!cachePath.exists()) {
        cachePath.mkpath(cachePath.path());
    }
    else if(outFile.exists()) {
        const QFileInfo info(outFile);
        for(int i = 1; ; ++i) {
            const QString newFileName = QString("%1-%2.%3").arg(info.baseName())
                                        .arg(QString::number(i)).arg(info.completeSuffix());
            const QString newPath = cachePath.absoluteFilePath(newFileName);
            if(!QFile::exists(newPath)) {
                outFile.setFileName(newPath);
                break;
            }
        }
    }

    outFile.open(QIODevice::WriteOnly);
}

HttpDownload::~HttpDownload()
{
    outFile.remove();
}

void HttpDownload::start(QNetworkAccessManager* netMan)
{
    reply.reset(netMan->get(request));

    connect(reply.get(), SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(updateProgress(qint64, qint64)));

    connect(reply.get(), SIGNAL(finished()), this, SLOT(downloadFinished()));

    timer.start();
    speedTimer.start();

    status = Status::Running;
}

double HttpDownload::percentCompleted() const
{
    return static_cast<double>(received) / total * 100;
}

bool HttpDownload::sizeKnown() const
{
    return total >= 0;
}

int HttpDownload::bytesDownloaded() const
{
    return received;
}

int HttpDownload::bytesTotal() const
{
    return total;
}

bool HttpDownload::isFinished() const
{
    return reply->isFinished();
}

int HttpDownload::duration() const
{
    return dlDuration;
}

QString HttpDownload::fileName() const
{
    const QFileInfo info(outFile);
    return info.fileName();
}

void HttpDownload::abort()
{
    if(reply->isFinished()) {
        return;
    }

    reply->abort();

    status = Status::Aborted;

    outFile.remove();

    emit updated();
}

HttpDownload::Status HttpDownload::getStatus() const
{
    return status;
}

QUrl HttpDownload::url() const
{
    return request.url();
}

double HttpDownload::downloadSpeed() const
{
    return speed;
}

bool HttpDownload::hasError() const
{
    return reply->error() != QNetworkReply::NoError;
}

QString HttpDownload::errorString() const
{
    return reply->errorString();
}

int HttpDownload::statusCode() const
{
    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

QString HttpDownload::reason() const
{
    return reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
}

void HttpDownload::downloadFinished()
{
    dlDuration = timer.elapsed();

    // Prevent aborted requests from being marked as finished
    if(status == Status::Running) {
        status = Status::Finished;
    }
}

void HttpDownload::updateProgress(const qint64 bytesReceived, const qint64 bytesTotal)
{
    received = bytesReceived;
    total = bytesTotal;

    dlDuration = timer.elapsed();

    // Calculate download speed in the last second
    if(speedTimer.elapsed() > 1000) {
        downloaded = received - downloaded;
        speed = static_cast<double>(downloaded) / (speedTimer.elapsed() / 1000.0);
        downloaded = received;
        speedTimer.restart();
    }

    // reply->readAll() fails to return complete data
    outFile.write(reply->read(reply->bytesAvailable()));

    emit updated();
}

} // namespace net
} // namespace vfg
