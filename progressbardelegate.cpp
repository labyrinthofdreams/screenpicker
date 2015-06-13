#include <QApplication>
#include <QModelIndex>
#include <QPainter>
#include <QSize>
#include <QString>
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QStyleOptionViewItem>
#include "httpdownload.hpp"
#include "progressbardelegate.hpp"

template <class NumberType>
QString formatNumber(NumberType number) {
    const auto KB = number / 1000.0;
    if(KB / 1000.0 < 1.0) {
        return QString("%1KB").arg(QString::number(KB, 'f', 2));
    }

    const auto MB = KB / 1000.0;
    if(MB / 1000.0 < 1.0) {
        return QString("%1MB").arg(QString::number(MB, 'f', 2));
    }

    const auto GB = MB / 1000.0;
    return QString("%1GB").arg(QString::number(GB, 'f', 2));
}

namespace vfg {
namespace ui {

ProgressBarDelegate::ProgressBarDelegate(const int height, QObject *parent) :
    QStyledItemDelegate(parent),
    height(height)
{
}

void ProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    // Set up a QStyleOptionProgressBar to precisely mimic the
    // environment of a progress bar.
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.state = QStyle::State_Enabled;
    progressBarOption.direction = QApplication::layoutDirection();
    progressBarOption.rect = QRect(0, height * index.row(), option.rect.width(), height);
    progressBarOption.fontMetrics = QApplication::fontMetrics();
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;

    auto get = [&index](const int row, const int column){
        return index.model()->data(index.model()->index(row, column));
    };

    auto dl = get(index.row(), index.column()).value<std::shared_ptr<vfg::net::HttpDownload>>();

    // Set the progress and text values of the style option.
    const auto bytesDownloaded = formatNumber(dl->bytesDownloaded());
    const auto bytesTotal = formatNumber(dl->bytesTotal());
    const auto speed = formatNumber(dl->downloadSpeed());
    if(dl->getStatus() == vfg::net::HttpDownload::Status::Aborted) {
        if(!dl->sizeKnown()) {
            progressBarOption.progress = 0;
        }
        else {
            progressBarOption.progress = dl->percentCompleted();
        }

        progressBarOption.text = QString("%1: Aborted").arg(dl->url().host());
    }
    else if(dl->isFinished()) {
        progressBarOption.progress = dl->percentCompleted();
        progressBarOption.text = QString("%3: %1 (%2/s)").arg(bytesTotal)
                                 .arg(speed)
                                 .arg(dl->url().host());
    }
    else {
        if(dl->sizeKnown()) {
            progressBarOption.progress = dl->percentCompleted();
            progressBarOption.text = QString("%4: %1 of %2 (%3/s)").arg(bytesDownloaded)
                                     .arg(bytesTotal)
                                     .arg(speed)
                                     .arg(dl->url().host());
        }
        else {
            // Size of the download is not known, display infinite progress bar
            progressBarOption.maximum = 0;
            progressBarOption.text = QString("%3: %1 (%2/s)").arg(bytesDownloaded)
                                     .arg(speed)
                                     .arg(dl->url().host());
        }
    }

    // Draw the progress bar onto the view.
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}

QSize ProgressBarDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(QStyledItemDelegate::sizeHint(option, index).width(), height);
}

} // namespace ui
} // namespace vfg
