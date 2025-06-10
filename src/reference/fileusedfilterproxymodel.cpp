/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "fileusedfilterproxymodel.h"
#include "filereferenceitem.h"

namespace gams {
namespace studio {
namespace reference {

FileUsedFilterProxyModel::FileUsedFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}, mShowFileOnce(true)
{
}

void FileUsedFilterProxyModel::showFileOnceChanged(bool showOnce)
{
    mShowFileOnce = showOnce;
}

bool FileUsedFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // accept every row if mShowFileOnce is not checked
    if (!mShowFileOnce)
        return true;

    // accept is itself matches
    if (filterAcceptsSelfRow(sourceRow, sourceParent))
        return true;

    // accept if any of its children matches
    if (hasAcceptedChildren(sourceRow, sourceParent))
        return true;

    return false;
}

bool FileUsedFilterProxyModel::filterAcceptsSelfRow(int sourceRow, const QModelIndex &sourceParent) const
{
    int col = static_cast<int>(FileReferenceItemColumn::FirstEntry);
    QModelIndex keyIndex = sourceModel()->index(sourceRow, col, sourceParent);
    if (!keyIndex.isValid())
        return false;

    return (keyIndex.data().toBool());
}

bool FileUsedFilterProxyModel::hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const
{
    for (int col=0; col < sourceModel()->columnCount(); ++col) {
        const QModelIndex index = sourceModel()->index(sourceRow, col, sourceParent);
        if (!index.isValid())
            continue;

        const int childCount = index.model()->rowCount(index);
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

} // namespace reference
} // namespace studio
} // namespace gams
