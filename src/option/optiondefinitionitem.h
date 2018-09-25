/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef OPTIONDEFINITIONITEM_H
#define OPTIONDEFINITIONITEM_H

#include <QList>
#include <QVector>

namespace gams {
namespace studio {
namespace option {

class OptionDefinitionItem
{
public:
    OptionDefinitionItem(const QList<QVariant>& data, OptionDefinitionItem* parentItem = nullptr);
    ~OptionDefinitionItem();

    void appendChild(OptionDefinitionItem *child);

    OptionDefinitionItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    OptionDefinitionItem *parentItem();

private:
    QList<OptionDefinitionItem*> mChildItems;
    QList<QVariant> mItemData;
    OptionDefinitionItem *mParentItem;
};

} // namespace option
} // namespace studio
} // namespace gams

#endif // OPTIONDEFINITIONITEM_H
