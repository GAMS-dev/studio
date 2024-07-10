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
#include "lxitreeitem.h"

namespace gams {
namespace studio {
namespace lxiviewer {

LxiTreeItem::LxiTreeItem(const QString &index, int lineNr, const QString &text, LxiTreeItem *parentItem)
    : mIndex(index), mLineNr(lineNr), mText(text), mParentItem(parentItem)
{

}

LxiTreeItem::~LxiTreeItem()
{
    qDeleteAll(mChildItems);
}

void LxiTreeItem::appendChild(LxiTreeItem *child)
{
    mChildItems.append(child);
}

LxiTreeItem *LxiTreeItem::child(int row)
{
    return mChildItems.value(row);
}

int LxiTreeItem::childCount() const
{
    return mChildItems.count();
}

int LxiTreeItem::row() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<LxiTreeItem*>(this));

    return 0;
}

LxiTreeItem *LxiTreeItem::parentItem()
{
    return mParentItem;
}

QString LxiTreeItem::index() const
{
    return mIndex;
}

QString LxiTreeItem::text() const
{
    return mText;
}

int LxiTreeItem::lineNr() const
{
    return mLineNr;
}

QModelIndex LxiTreeItem::modelIndex() const
{
    return mModelIndex;
}

void LxiTreeItem::setModelIndex(QModelIndex value)
{
    mModelIndex = value;
}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
