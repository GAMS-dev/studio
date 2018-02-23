#include "optionsortfilterproxymodel.h"

namespace gams {
namespace studio {

OptionSortFilterProxyModel::OptionSortFilterProxyModel(QObject *parent)
      : QSortFilterProxyModel(parent)
{
}

bool OptionSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // accept is itself matches
    if (filterAcceptsSelfRow(sourceRow, sourceParent))
       return true;

    // accept if any of its parents matches
    QModelIndex parent = sourceParent;
    while (parent.isValid()) {
        if (filterAcceptsSelfRow(parent.row(), parent.parent()))
            return true;
        parent = parent.parent();
    }

    // accept if any of its children matches
    if (hasAcceptedChildren(sourceRow, sourceParent))
        return true;
    else
        return false;
}

bool OptionSortFilterProxyModel::filterAcceptsSelfRow(int sourceRow, const QModelIndex &sourceParent) const
{
    for (int col=0; col < sourceModel()->columnCount(); ++col) {
        QModelIndex keyIndex = sourceModel()->index(sourceRow, col, sourceParent);
        if (!keyIndex.isValid())
           continue;

        if (sourceModel()->data(keyIndex).toString().contains(filterRegExp()))
           return true;
    }
    return false;
}

bool OptionSortFilterProxyModel::hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const
{
    for (int col=0; col < sourceModel()->columnCount(); ++col) {
        QModelIndex index = sourceModel()->index(sourceRow, col, sourceParent);
        if (!index.isValid())
          continue;

        int childCount = index.model()->rowCount(index);
        if (childCount == 0)
           continue;

        for (int i = 0; i < childCount; ++i) {
            if (filterAcceptsSelfRow(i, index))
               return true;

            // DFS - recursive, better BFS?
            if (hasAcceptedChildren(i, index))
               return true;
        }
    }

    return false;
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
