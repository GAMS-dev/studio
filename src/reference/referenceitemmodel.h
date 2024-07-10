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
#ifndef REFERENCEITEMMODEL_H
#define REFERENCEITEMMODEL_H

#include <QList>
#include "reference.h"

namespace gams {
namespace studio {
namespace reference {

class ReferenceItemModel
{
public:
    ReferenceItemModel(const QList<QVariant>& data, ReferenceItemModel* parentItem = nullptr);
    ~ReferenceItemModel();

    void appendChild(ReferenceItemModel* child);

    ReferenceItemModel* child(int row);
    ReferenceItemModel *parent();
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;

    bool removeChildren(int position, int count);

private:
    QList<ReferenceItemModel*> mChildItems;
    QList<QVariant> mItemData;
    ReferenceItemModel* mParentItem = nullptr;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // REFERENCEITEMMODEL_H
