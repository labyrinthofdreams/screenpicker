#ifndef COMMON_HPP
#define COMMON_HPP

#include <QString>

namespace vfg {
namespace format {

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

} // namespace format
} // namespace vfg

#endif // COMMON_HPP
