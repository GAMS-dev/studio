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
#include "librarymodel.h"

namespace gams {
namespace studio {
namespace modeldialog {

LibraryModel::LibraryModel(const QList<LibraryItem> &data, QObject *parent)
    : QAbstractTableModel(parent), mData(data)
{
}

QVariant LibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            return mData.at(0).library()->columns().at(section);
    }
    else if (role == Qt::ToolTipRole) {
        if (orientation == Qt::Horizontal)
            return mData.at(0).library()->toolTips().at(section);
    }
    return QVariant();
}

int LibraryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mData.size();
}

int LibraryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mData.at(0).library()->nrColumns();
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    else if (role == Qt::DisplayRole)
        return mData.at(index.row()).values().at(mData.at(index.row()).library()->colOrder().at(index.column()));
    else if (role == Qt::UserRole)
        return QVariant::fromValue(index.internalPointer());
    return QVariant();
}

QModelIndex LibraryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column, (void *)&mData.at(row));
    return QModelIndex();
}

} // namespace modeldialog
} // namespace studio
} // namespace gams
