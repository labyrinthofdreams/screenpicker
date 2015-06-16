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
#include "httpdownload.hpp"
#include "progressbardelegate.hpp"

namespace {

enum class SuffixOption
{
    IncludeSuffix,
    ExcludeSuffix
};

template <class NumberType>
QString formatNumber(const NumberType number,
                     const SuffixOption suffixOpt = SuffixOption::IncludeSuffix) {
    const auto KB = number / 1000.0;
    if(KB / 1000.0 < 1.0) {
        return QString("%1%2").arg(QString::number(KB, 'f', 2))
                .arg(suffixOpt == SuffixOption::IncludeSuffix ? " KB" : "");
    }

    const auto MB = KB / 1000.0;
    if(MB / 1000.0 < 1.0) {
        return QString("%1%2").arg(QString::number(MB, 'f', 2))
                .arg(suffixOpt == SuffixOption::IncludeSuffix ? " MB" : "");
    }

    const auto GB = MB / 1000.0;
    return QString("%1%2").arg(QString::number(GB, 'f', 2))
            .arg(suffixOpt == SuffixOption::IncludeSuffix ? " GB" : "");
}

template <class NumberType>
QString formatNumberMB(const NumberType number,
                       const SuffixOption suffixOpt = SuffixOption::IncludeSuffix) {
    if(number < 1000000) {
        return formatNumber(number / 1000.0, suffixOpt);
    }

    return formatNumber(number, suffixOpt);
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
    auto rect = [&](const int left, const int top, const int width, const int height){
        return QRect(left, (index.row() * this->height) + top, option.rect.width() + width, height);
    };

    auto get = [&index](const int row, const int column){
        return index.model()->data(index.model()->index(row, column));
    };

    auto dl = get(index.row(), index.column()).value<std::shared_ptr<vfg::net::HttpDownload>>();

    // #000000 (black)
    painter->setPen(QPen(QColor::fromRgb(0, 0, 0), 1, Qt::SolidLine));
    painter->drawText(rect(5, 5, 0, 20), Qt::AlignLeft, dl->fileName());

    // #4D4D4D (gray)
    painter->setPen(QPen(QColor::fromRgb(77, 77, 77), 1, Qt::SolidLine));
    if(dl->getStatus() == vfg::net::HttpDownload::Status::Finished) {
        painter->drawText(rect(5, 25, 0, 20), Qt::AlignLeft,
                          QString("%1 - %2").arg(formatNumber(dl->bytesTotal()))
                                            .arg(dl->url().host()));
    }
    else if(dl->getStatus() == vfg::net::HttpDownload::Status::Aborted) {
        painter->drawText(rect(5, 25, 0, 20), Qt::AlignLeft,
                          QString("Canceled - %1").arg(dl->url().host()));
    }
    else if(dl->getStatus() == vfg::net::HttpDownload::Status::Running) {
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = rect(5, 25, -5, 15);
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;

        QString text;
        if(dl->sizeKnown()) {
            progressBarOption.progress = dl->percentCompleted();
            text = QString("%4: %1 of %2 (%3/sec)").arg(formatNumberMB(dl->bytesDownloaded(), SuffixOption::ExcludeSuffix))
                   .arg(formatNumber(dl->bytesTotal())).arg(formatNumber(dl->downloadSpeed()))
                   .arg(dl->url().host());
        }
        else {
            // If the download size is not known, display infinite progress bar
            progressBarOption.maximum = 0;
            text = QString("%3: %1 (%2/sec)").arg(formatNumber(dl->bytesDownloaded()))
                   .arg(formatNumber(dl->downloadSpeed())).arg(dl->url().host());
        }

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

        painter->drawText(rect(5, 45, 0, 20), Qt::AlignLeft, text);
    }
    else if(dl->getStatus() == vfg::net::HttpDownload::Status::Pending) {
        painter->drawText(rect(5, 25, 0, 20), Qt::AlignLeft,
                          QString("Pending - %1").arg(dl->url().host()));
    }
}

QSize ProgressBarDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(QStyledItemDelegate::sizeHint(option, index).width(), height);
}

} // namespace ui
} // namespace vfg
