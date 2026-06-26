/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "labelfilterproxymodel.h"

namespace gams {
namespace studio {
namespace gdxviewer {

LabelFilterProxyModel::LabelFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{}

bool LabelFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int leftCheckState = sourceModel()->data(source_left, Qt::CheckStateRole).toInt();
    int rightCheckState = sourceModel()->data(source_right, Qt::CheckStateRole).toInt();

    if (leftCheckState != rightCheckState)
        return leftCheckState < rightCheckState;
    return source_left.row() > source_right.row();
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
