#ifndef OPTIONSORTFILTERPROXYMODEL_H
#define OPTIONSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace gams {
namespace studio {

class OptionSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    OptionSortFilterProxyModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
    bool filterAcceptsSelfRow(int sourcRrow, const QModelIndex& sourceParent) const;
    bool hasAcceptedChildren(int sourceRow, const QModelIndex& sourceParent) const;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

};

} // namespace studio
} // namespace gams

#endif // OPTIONSORTFILTERPROXYMODEL_H
