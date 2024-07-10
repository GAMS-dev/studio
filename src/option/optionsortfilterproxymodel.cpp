/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "optionsortfilterproxymodel.h"

namespace gams {
namespace studio {
namespace option {

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

        if (sourceModel()->data(keyIndex).toString().contains(filterRegularExpression()))
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

    if (leftData.typeId() == QMetaType::QString) {
        return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
    } else {
        return QSortFilterProxyModel::lessThan(left, right);
    }
}

} // namespace option
} // namespace studio
} // namespace gams
