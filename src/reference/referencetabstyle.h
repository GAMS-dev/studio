#ifndef REFERENCETABSTYLE_H
#define REFERENCETABSTYLE_H

#include <QStyledItemDelegate>

namespace gams {
namespace studio {
namespace reference {

class ReferenceTabStyle: public QStyledItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // REFERENCETABSTYLE_H
