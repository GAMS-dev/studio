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
#include  <QDebug>
#include "referenceitemmodel.h"

namespace gams {
namespace studio {
namespace reference {

ReferenceItemModel::ReferenceItemModel(const QList<QVariant> &data, ReferenceItemModel *parent) :
    mItemData(data), mParentItem(parent)
{
}

ReferenceItemModel::~ReferenceItemModel()
{
    mItemData.clear();

    qDeleteAll(mChildItems);
    mChildItems.clear();
}

void ReferenceItemModel::appendChild(ReferenceItemModel *child)
{
    mChildItems.append(child);
}

ReferenceItemModel *ReferenceItemModel::child(int row)
{
    return mChildItems.value(row);
}

ReferenceItemModel *ReferenceItemModel::parent()
{
    return mParentItem;
}

int ReferenceItemModel::childCount() const
{
    return mChildItems.count();
}

int ReferenceItemModel::columnCount() const
{
    return mItemData.count();
}

QVariant ReferenceItemModel::data(int column) const
{
    return mItemData.value(column);
}

int ReferenceItemModel::row() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<ReferenceItemModel*>(this));

    return 0;
}

bool ReferenceItemModel::removeChildren(int position, int count)
{
    if (position < 0 || position + count > mChildItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete mChildItems.takeAt(position);

    return true;
}

} // namespace reference
} // namespace studio
} // namespace gams
