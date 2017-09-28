#include "treeitemdelegate.h"

namespace gams {
namespace studio {

TreeItemDelegate::TreeItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{}

void TreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 opt(option);
    opt.textElideMode = Qt::ElideMiddle;
    QStyledItemDelegate::paint(painter, opt, index);
}

} // namespace studio
} // namespace gams
