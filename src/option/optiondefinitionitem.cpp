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
#include "optiondefinitionitem.h"

namespace gams {
namespace studio {
namespace option {

OptionDefinitionItem::OptionDefinitionItem(const QList<QVariant>& data, OptionDefinitionItem* parent)
{
    mParentItem = parent;
    mItemData = data;
}

OptionDefinitionItem::~OptionDefinitionItem()
{
    qDeleteAll(mChildItems);
}

void OptionDefinitionItem::appendChild(OptionDefinitionItem *item)
{
    mChildItems.append(item);
}

OptionDefinitionItem *OptionDefinitionItem::child(int row)
{
    return mChildItems.value(row);
}

OptionDefinitionItem *OptionDefinitionItem::parent()
{
    return mParentItem;
}

int OptionDefinitionItem::childCount() const
{
    return mChildItems.count();
}

int OptionDefinitionItem::row() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<OptionDefinitionItem*>(this));

    return 0;
}

int OptionDefinitionItem::columnCount() const
{
    return mItemData.count();
}

QVariant OptionDefinitionItem::data(int column) const
{
    return mItemData.value(column);
}

OptionDefinitionItem *OptionDefinitionItem::parentItem()
{
    return mParentItem;
}

bool OptionDefinitionItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= mItemData.size())
        return false;

    mItemData[column] = value;
    return true;
}

void OptionDefinitionItem::setParent(OptionDefinitionItem *parent)
{
    mParentItem = parent;
}

void OptionDefinitionItem::insertChild(int row, OptionDefinitionItem* item)
{
    item->setParent(this);
    mChildItems.insert(row, item);
}

bool OptionDefinitionItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > mChildItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete mChildItems.takeAt(position);

    return true;
}

bool OptionDefinitionItem::modified() const
{
    return mModified;
}

void OptionDefinitionItem::setModified(bool modified)
{
    mModified = modified;
}

} // namespace option
} // namespace studio
} // namespace gams
