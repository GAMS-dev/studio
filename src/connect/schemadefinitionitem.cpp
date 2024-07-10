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
#include "schemadefinitionitem.h"

namespace gams {
namespace studio {
namespace connect {

SchemaDefinitionItem::SchemaDefinitionItem(const QString& name, const QList<QVariant>& data, SchemaDefinitionItem* parentItem):
    mSchemaName(name),
    mItemData(data),
    mParentItem(parentItem)
{
}

SchemaDefinitionItem::~SchemaDefinitionItem()
{
    for(int i=0; i<childCount(); ++i)
        removeChildren(i, 1);
    qDeleteAll(mChildItems);
}

void SchemaDefinitionItem::appendChild(SchemaDefinitionItem *child)
{
    mChildItems.append(child);
}

SchemaDefinitionItem *SchemaDefinitionItem::child(int row)
{
    return mChildItems.value(row);
}

SchemaDefinitionItem *SchemaDefinitionItem::parent()
{
     return mParentItem;
}

int SchemaDefinitionItem::childCount() const
{
    return mChildItems.count();
}

int SchemaDefinitionItem::columnCount() const
{
    return mItemData.count();
}

QVariant SchemaDefinitionItem::data(int column) const
{
    return mItemData.value(column);
}

int SchemaDefinitionItem::row() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<SchemaDefinitionItem*>(this));

    return 0;
}

SchemaDefinitionItem *SchemaDefinitionItem::parentItem()
{
    return mParentItem;
}

bool SchemaDefinitionItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= mItemData.size())
        return false;

    mItemData[column] = value;
    return true;
}

void SchemaDefinitionItem::setParent(SchemaDefinitionItem *parent)
{
    mParentItem = parent;
}

void SchemaDefinitionItem::insertChild(int row, SchemaDefinitionItem *item)
{
    item->setParent(this);
    mChildItems.insert(row, item);
}

bool SchemaDefinitionItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > mChildItems.size())
        return false;

    for (int row = position+count-1; row >=position; --row) {
        SchemaDefinitionItem* item = mChildItems.takeAt(row);
        for (int i = item->childCount(); i>=0; --i) {
            item->removeChildren(i, 1);
        }
        delete item;
    }

    return true;
}

} // namespace connect
} // namespace studio
} // namespace gams
