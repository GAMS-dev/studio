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
#include <QDebug>
#include "schemadefinitionmodel.h"

namespace gams {
namespace studio {
namespace connect {

SchemaDefinitionModel::SchemaDefinitionModel(Connect* connect, const QString& schemaName, QObject *parent)
    : QAbstractItemModel{parent},
      mCurrentSchemaName(schemaName),
      mConnect(connect)
{
    setupTreeItemModelData();
}

SchemaDefinitionModel::~SchemaDefinitionModel()
{
    qDeleteAll(mRootItems.begin(), mRootItems.end());
    mRootItems.clear();
}

QVariant SchemaDefinitionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        SchemaDefinitionItem* item = static_cast<SchemaDefinitionItem*>(index.internalPointer());
        return item->data(index.column());
    }
    case Qt::ToolTipRole: {
        SchemaDefinitionItem* item = static_cast<SchemaDefinitionItem*>(index.internalPointer());
        return item->data(index.column());
    }
    default:
         break;
    }
    return QVariant();
}

Qt::ItemFlags SchemaDefinitionModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::NoItemFlags;
    else
        return Qt::ItemIsDragEnabled | defaultFlags;  // ToDo
}

QVariant SchemaDefinitionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return mRootItems[mCurrentSchemaName]->data(section);

    return QVariant();
}

QModelIndex SchemaDefinitionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SchemaDefinitionItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItems[mCurrentSchemaName];
    else
        parentItem = static_cast<SchemaDefinitionItem*>(parent.internalPointer());

    SchemaDefinitionItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex SchemaDefinitionModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    SchemaDefinitionItem *childItem = static_cast<SchemaDefinitionItem*>(index.internalPointer());
    SchemaDefinitionItem *parentItem = childItem->parentItem();

    if (parentItem == mRootItems[mCurrentSchemaName])
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int SchemaDefinitionModel::rowCount(const QModelIndex &parent) const
{
    SchemaDefinitionItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = mRootItems[mCurrentSchemaName];
    else
        parentItem = static_cast<SchemaDefinitionItem*>(parent.internalPointer());

    return parentItem->childCount();

}

int SchemaDefinitionModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<SchemaDefinitionItem*>(parent.internalPointer())->columnCount();
    else
        return mRootItems[mCurrentSchemaName]->columnCount();
}

void SchemaDefinitionModel::setupTreeItemModelData()
{
    QList<QVariant> rootData;
    rootData << "Field" << "Type" << "Required" << "Allowed Values"  << "Schema" << "Min" << "Max";

    for(const QString& schemaName : mConnect->getSchemaNames()) {
        qDebug() << "schema=" << schemaName;
        SchemaDefinitionItem* rootItem = new SchemaDefinitionItem(schemaName, rootData);
        mRootItems[schemaName] = rootItem;

        ConnectSchema* schema = mConnect->getSchema(schemaName);
        for(const QString& key : schema->getFirstLevelKeyList()) {
            qDebug() << "   key=" << key;
            QList<QVariant> columnData;
            columnData << key;
            Schema* s = schema->getSchema(key);
            QStringList typeList;
            for(Type t : schema->getType(key)) { // s->types) {
                typeList << QString::fromLatin1(typeToString(t));
                 qDebug() << "   type=" << QString::fromLatin1(typeToString(t));
            }
            if (s->types.size() > 0) {
                if (s->types.size() == 1)
                   columnData << QString("%1").arg(typeList.at(0));
               else
                   columnData << QString("[%1]").arg(typeList.join(","));
            } else {
                columnData << "";
            }
            columnData << (s->required?"Yes":"No");
            columnData << ""; // s->allowedValues;
            columnData << ""; // s->Min;
            columnData << ""; // s->Max;

            SchemaDefinitionItem* item = new SchemaDefinitionItem(schemaName, columnData, mRootItems.last());
            mRootItems.last()->appendChild(item);
        }
    }
}

} // namespace connect
} // namespace studio
} // namespace gams
