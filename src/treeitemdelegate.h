#ifndef TREEITEMDELEGATE_H
#define TREEITEMDELEGATE_H

#include <QtWidgets>

namespace gams {
namespace ide {

class TreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TreeItemDelegate(QObject* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

} // namespace ide
} // namespace gams

#endif // TREEITEMDELEGATE_H
