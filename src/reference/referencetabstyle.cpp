#include "referencetabstyle.h"
#include <QPainter>

namespace gams {
namespace studio {
namespace reference {

void ReferenceTabStyle::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    opt.displayAlignment = Qt::AlignCenter;
    if (option.state & QStyle::State_MouseOver && option.state & QStyle::State_Enabled) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.brush(QPalette::AlternateBase));
        painter->drawRect(option.rect);
        painter->restore();
    }
    QStyledItemDelegate::paint(painter, opt, index);
    painter->save();
    painter->setPen(option.palette.color(QPalette::Mid));
    painter->drawLine(option.rect.left(), option.rect.bottom(), option.rect.right(), option.rect.bottom());
    painter->restore();
}

} // namespace reference
} // namespace studio
} // namespace gams
