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
#ifndef GAMS_STUDIO_CONNECT_CONNECTDATAITEM_H
#define GAMS_STUDIO_CONNECT_CONNECTDATAITEM_H

#include <QVariant>

namespace gams {
namespace studio {
namespace connect {

class ConnectDataItem
{
public:
    ConnectDataItem(const QList<QVariant>& data, int id, ConnectDataItem* parentItem = nullptr);
    ~ConnectDataItem();

    void appendChild(ConnectDataItem *child);

    ConnectDataItem* child(int row);
    int childNumber() const;
    int id() const;

    int childCount() const;
    int columnCount() const;

    QVariant data(int column) const;
    int row() const;

    ConnectDataItem *parentItem();
    void setParent(ConnectDataItem* parent);

    bool setData(int column, const QVariant &value);

    void insertChild(int row, ConnectDataItem* item);
    bool removeChildren(int position, int count);

    bool moveChildren(int source, int destination);

    bool isFirstChild() const;
    bool isLastChild() const;
private:
    int                     mID;
    QList<QVariant>         mItemData;
    QList<ConnectDataItem*> mChildItems;
    ConnectDataItem*        mParentItem;

};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_CONNECT_CONNECTDATAITEM_H
