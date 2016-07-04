#ifndef VFG_UI_PROGRESSBARDELEGATE_HPP
#define VFG_UI_PROGRESSBARDELEGATE_HPP

#include <QStyledItemDelegate>

class QModelIndex;
class QObject;
class QPainter;
class QSize;
class QStyleOptionViewItem;

namespace vfg {
namespace ui {

class ProgressBarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

private:
    //! Height of the delegate in pixels
    int height {70};

public:
    /**
     * @brief Constructor
     * @param height Height of the widget that's drawn
     * @param parent Owner of the widget
     */
    explicit ProgressBarDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

} // namespace ui
} // namespace vfg

#endif // VFG_UI_PROGRESSBARDELEGATE_HPP
