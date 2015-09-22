#include <QApplication>
#include <QModelIndex>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QStyleOptionViewItem>
#include "common.hpp"
#include "httpdownload.hpp"
#include "progressbardelegate.hpp"

namespace {

template <class NumberType>
QString formatNumberMB(const NumberType number,
                       const vfg::format::SuffixOption suffixOpt = vfg::format::SuffixOption::IncludeSuffix) {
    if(number < 1000000) {
        return vfg::format::formatNumber(number / 1000.0, suffixOpt);
    }

    return vfg::format::formatNumber(number, suffixOpt);
}

} // namespace

namespace vfg {
namespace ui {

ProgressBarDelegate::ProgressBarDelegate(QObject *parent) :
    QStyledItemDelegate(parent),
    height(70)
{
}

void ProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    auto rect = [&option](const int left, const int top, const int width = -1, const int height = -1){
        return option.rect.adjusted(left, top, width, height);
    };

    auto get = [&index](const int row, const int column){
        return index.model()->data(index.model()->index(row, column));
    };

    auto dl = get(index.row(), index.column()).value<std::shared_ptr<vfg::net::HttpDownload>>();

    // #000000 (black)
    painter->setPen(QPen(QColor::fromRgb(0, 0, 0), 1, Qt::SolidLine));
    painter->drawText(rect(5, 5), Qt::AlignLeft, dl->fileName());

    // #4D4D4D (gray)
    painter->setPen(QPen(QColor::fromRgb(77, 77, 77), 1, Qt::SolidLine));
    if(dl->getStatus() == vfg::net::HttpDownload::Status::Finished) {
        painter->drawText(rect(5, 25), Qt::AlignLeft,
                          QString("%1 - %2").arg(vfg::format::formatNumber(dl->bytesTotal()))
                                            .arg(dl->url().host()));
    }
    else if(dl->getStatus() == vfg::net::HttpDownload::Status::Aborted) {
        painter->drawText(rect(5, 25), Qt::AlignLeft,
                          QString("Canceled - %1").arg(dl->url().host()));
    }
    else if(dl->getStatus() == vfg::net::HttpDownload::Status::Running) {
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = rect(5, 25, -5);
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;

        QString text;
        if(dl->sizeKnown()) {
            progressBarOption.progress = dl->percentCompleted();
            text = QString("%4: %1 of %2 (%3/sec)").arg(formatNumberMB(dl->bytesDownloaded(), vfg::format::SuffixOption::ExcludeSuffix))
                   .arg(vfg::format::formatNumber(dl->bytesTotal())).arg(vfg::format::formatNumber(dl->downloadSpeed()))
                   .arg(dl->url().host());
        }
        else {
            // If the download size is not known, display infinite progress bar
            progressBarOption.maximum = 0;
            text = QString("%3: %1 (%2/sec)").arg(vfg::format::formatNumber(dl->bytesDownloaded()))
                   .arg(vfg::format::formatNumber(dl->downloadSpeed())).arg(dl->url().host());
        }

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

        painter->drawText(rect(10, 45), Qt::AlignLeft, text);
    }
    else if(dl->getStatus() == vfg::net::HttpDownload::Status::Pending) {
        painter->drawText(rect(5, 25), Qt::AlignLeft,
                          QString("Pending - %1").arg(dl->url().host()));
    }
}

QSize ProgressBarDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(QStyledItemDelegate::sizeHint(option, index).width(), height);
}

} // namespace ui
} // namespace vfg
