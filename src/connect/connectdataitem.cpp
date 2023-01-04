/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include "connectdataitem.h"

namespace gams {
namespace studio {
namespace connect {

ConnectDataItem::ConnectDataItem( const QList<QVariant> &data, int id, ConnectDataItem *parentItem):
    mID(id),
    mItemData(data),
    mParentItem(parentItem)
{
}

ConnectDataItem::~ConnectDataItem()
{
    qDeleteAll(mChildItems);
}

void ConnectDataItem::appendChild(ConnectDataItem *child)
{
    mChildItems.append(child);
}

ConnectDataItem *ConnectDataItem::child(int row)
{
    return mChildItems.value(row);
}

int ConnectDataItem::childNumber() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<ConnectDataItem*>(this));
    return 0;
}

int ConnectDataItem::id() const
{
    return mID;
}

int ConnectDataItem::childCount() const
{
    return mChildItems.count();
}

int ConnectDataItem::columnCount() const
{
    return mItemData.count();
}

QVariant ConnectDataItem::data(int column) const
{
    return mItemData.value(column);
}

int ConnectDataItem::row() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<ConnectDataItem*>(this));

    return 0;
}

ConnectDataItem *ConnectDataItem::parentItem()
{
    return mParentItem;
}

void ConnectDataItem::setParent(ConnectDataItem *parent)
{
    mParentItem = parent;
}

bool ConnectDataItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= mItemData.size())
        return false;

    mItemData[column] = value;
    return true;
}

void ConnectDataItem::insertChild(int row, ConnectDataItem *item)
{
    item->setParent(this);
    mChildItems.insert(row, item);
}

bool ConnectDataItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > mChildItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete mChildItems.takeAt(position);

    return true;
}

} // namespace connect
} // namespace studio
} // namespace gams
