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
#include "filereferenceitem.h"

namespace gams {
namespace studio {
namespace reference {

FileReferenceItem::FileReferenceItem(const QList<QVariant> &data, FileReferenceItem *parentItem) :
    mItemData(data),
    mParentItem(parentItem)
{
}

FileReferenceItem::~FileReferenceItem()
{
}

void FileReferenceItem::appendChild(FileReferenceItem *child)
{
    mChildItems.append(child);
}

FileReferenceItem *FileReferenceItem::child(int row)
{
    return mChildItems.value(row);
}

int FileReferenceItem::childNumber() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<FileReferenceItem*>(this));
    return 0;
}

FileReferenceItem *FileReferenceItem::parent()
{
     return mParentItem;
}

int FileReferenceItem::childCount() const
{
    return mChildItems.count();
}

int FileReferenceItem::columnCount() const
{
    return mItemData.count();
}

QVariant FileReferenceItem::data(int column) const
{
    return mItemData.value(column);
}

int FileReferenceItem::row() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<FileReferenceItem*>(this));

    return 0;

}

bool FileReferenceItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= mItemData.size())
        return false;

    mItemData[column] = value;
    return true;
}

void FileReferenceItem::setParent(FileReferenceItem *parent)
{
    mParentItem = parent;
}

void FileReferenceItem::insertChild(int row, FileReferenceItem *item)
{
    item->setParent(this);
    mChildItems.insert(row, item);
}

} // namespace reference
} // namespace studio
} // namespace gams
