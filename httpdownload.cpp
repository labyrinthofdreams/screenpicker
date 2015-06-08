#include <memory>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTime>
#include <QUrl>
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
    status(Status::Pending)
{
    if(!cachePath.exists()) {
        cachePath.mkpath(cachePath.path());
    }

    // TODO: If filename already exists, try another filename
    outFile.open(QIODevice::WriteOnly);
}

void HttpDownload::start(QNetworkAccessManager* netMan)
{
    reply.reset(netMan->get(request));

    connect(reply.get(), SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(updateProgress(qint64, qint64)));

    connect(reply.get(), SIGNAL(finished()), this, SLOT(downloadFinished()));

    timer.start();

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
    return outFile.fileName();
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

    // reply->readAll() fails to return complete data
    outFile.write(reply->read(reply->bytesAvailable()));

    emit updated();
}

} // namespace net
} // namespace vfg
