#include "treeitemdelegate.h"

namespace gams {
namespace ide {

TreeItemDelegate::TreeItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{}

void TreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 opt(option);
    opt.textElideMode = Qt::ElideMiddle;
    QStyledItemDelegate::paint(painter, opt, index);
}

} // namespace ide
} // namespace gams
