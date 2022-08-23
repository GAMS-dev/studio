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
#ifndef GAMS_STUDIO_CONNECT_CONNECTDATAMODEL_H
#define GAMS_STUDIO_CONNECT_CONNECTDATAMODEL_H

#include <QAbstractItemModel>

#include "connect.h"
#include "connectdataitem.h"

namespace gams {
namespace studio {
namespace connect {

enum class DataCheckState {
    Root         = 0,
    SchemaName   = 1,
    ListItem     = 2,
    KeyItem      = 3,
    ElementKey   = 4,
    ElementValue = 5,
    ElementMap   = 6,
    ListAppend   = 7,
    MapAppend    = 8
};

enum class DataItemColumn {
    Key           = 0,
    Value         = 1,
    CheckState   = 2,
    SchemaType   = 3,
    AllowedValue = 4,
    Delete        = 5,
    MoveDown     = 6,
    MoveUp       = 7,
    Expand        = 8
};

class ConnectDataModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ConnectDataModel(const QString& filename, Connect* c, QObject *parent = nullptr);
    ~ConnectDataModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex indexForTreeItem(ConnectDataItem* item);

    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    ConnectDataItem* getItem(const QModelIndex &index) const;
    void insertItem(int position, ConnectDataItem* item, const QModelIndex &parent);
    bool removeItem(const QModelIndex &index);

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                              const QModelIndex &destinationParent, int destinationChild) override;

public slots:
    void addFromSchema(ConnectData* data, int insertPosition);
    void appendMapElement(const QModelIndex& index);
    void appendListElement(ConnectData* data, const QModelIndex& index);

protected:
    void informDataChanged(const QModelIndex& parent);

    void setupTreeItemModelData();
    void insertSchemaModelData(ConnectData* data, int position);

    int              mItemIDCount;
    QString          mLocation;
    Connect*         mConnect;
    ConnectData*     mConnectData;
    ConnectDataItem* mRootItem;

};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_CONNECT_CONNECTDATAMODEL_H
