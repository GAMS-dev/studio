#include "optionsortfilterproxymodel.h"

namespace gams {
namespace studio {

OptionSortFilterProxyModel::OptionSortFilterProxyModel(QObject *parent)
      : QSortFilterProxyModel(parent)
{
}

bool OptionSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!index.isValid())
       return false;

    if ( sourceModel()->rowCount(index) > 0 ) {
        return sourceModel()->data(index).toString().contains(filterRegExp());
    } else {
        if (sourceModel()->parent(index).isValid())
             return sourceModel()->data(sourceParent).toString().contains(filterRegExp());
        else
           return sourceModel()->data(index).toString().contains(filterRegExp());
    }
}

bool OptionSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if (leftData.type() == QVariant::String) {
        return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
    } else {
        return QSortFilterProxyModel::lessThan(left, right);
    }
}

} // namespace studio
} // namespace gams
