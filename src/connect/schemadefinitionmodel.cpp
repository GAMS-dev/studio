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
#include <QColor>
#include <QApplication>
#include <QPalette>
#include <QMimeData>

#include "schemadefinitionmodel.h"
#include "theme.h"

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
        if (index.column()==item->columnCount()-1)
            return QVariant(item->data(index.column()).toStringList().join(":"));
        else
            return item->data(index.column());
    }
    case Qt::ForegroundRole: {
        SchemaDefinitionItem* item = static_cast<SchemaDefinitionItem*>(index.internalPointer());
        if (item->data(index.column()).toString().compare("schema")==0)
            return  QVariant::fromValue(Theme::color(Theme::Disable_Gray));
        else
            return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
    }
    case Qt::BackgroundRole: {
        SchemaDefinitionItem* item = static_cast<SchemaDefinitionItem*>(index.internalPointer());
        SchemaDefinitionItem *parentItem = item->parentItem();
        if (parentItem == mRootItems[mCurrentSchemaName]) {
            if (index.row() % 2 == 0)
               return QVariant::fromValue(QApplication::palette().color(QPalette::Base));
            else
                return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Window));
        } else {
            return QVariant::fromValue(QApplication::palette().color(QPalette::Base));
        }
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

QStringList SchemaDefinitionModel::mimeTypes() const
{
    QStringList types;
    types <<  "application/vnd.gams-connect.text";
    return types;
}

QMimeData *SchemaDefinitionModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.column() != 6)
            continue;
        QString text = QString("schema=%1").arg(index.data(Qt::DisplayRole).toString());
        qDebug() << "1 setmimedata:("<< index.row() << "," << index.column() << ")" << text;
        stream << text;
    }
    qDebug() << "2 setmimedata:"<< QString(encodedData);
    mimeData->setData( "application/vnd.gams-connect.text", encodedData);
    return mimeData;
}

void SchemaDefinitionModel::loadSchemaFromName(const QString &name)
{
    if (!mRootItems.keys().contains(name))
        return;

    mCurrentSchemaName = name;
    beginResetModel();
    endResetModel();
}

void SchemaDefinitionModel::addTypeList(QList<SchemaType>& typeList, QList<QVariant> &data)
{
    QStringList schemaTypeList;
    for(SchemaType t : typeList) {
        schemaTypeList << QString::fromLatin1(ConnectSchema::typeToString(t));
    }
    if (typeList.size() > 0) {
        if (typeList.size() == 1)
           data << QString("%1").arg(schemaTypeList.at(0));
       else
           data << QString("[%1]").arg(schemaTypeList.join(","));
    } else {
        data << "";
    }
}

void SchemaDefinitionModel::addValueList(QList<ValueWrapper> &valueList, QList<QVariant> &data)
{
    QStringList valueStrList;
    for(ValueWrapper& value :  valueList) {
        if (value.type==SchemaValueType::NoValue) {
            valueStrList << "";
        } else if (value.type==SchemaValueType::Integer) {
            valueStrList << QString::number(value.value.intval);
        } else if (value.type==SchemaValueType::Float) {
            valueStrList << QString::number(value.value.doubleval);
        } else if (value.type==SchemaValueType::String) {
            valueStrList << QString(value.value.stringval);
        } else if (value.type==SchemaValueType::Boolean) {
            valueStrList << QString(value.value.boolval);
        } else  {
            valueStrList << "";
        }
    }
    if (valueList.size() > 0) {
        if (valueList.size() == 1)
           data << QString("%1").arg(valueStrList.at(0));
       else
           data << QString("[%1]").arg(valueStrList.join(","));
    } else {
        data << "";
    }
}

void SchemaDefinitionModel::addValue(ValueWrapper& value, QList<QVariant>& data)
{
    if (value.type==SchemaValueType::NoValue) {
        data << "";
    } else if (value.type==SchemaValueType::Integer) {
        data << value.value.intval;
    } else if (value.type==SchemaValueType::Float) {
        data << value.value.doubleval;
    } else if (value.type==SchemaValueType::String) {
        data << value.value.stringval;
    } else if (value.type==SchemaValueType::Boolean) {
        data << value.value.boolval;
    } else  {
        data << "";
    }
}

void SchemaDefinitionModel::setupTreeItemModelData()
{
    QList<QVariant> rootData;
    rootData << "Field" << "Required"  << "Type" << "default"
             << "Allowed Values"  << "min" /*<< "max"*/ << "SchemaKey";

    foreach(const QString& schemaName, mConnect->getSchemaNames()) {
        SchemaDefinitionItem* rootItem = new SchemaDefinitionItem(schemaName, rootData);
        mRootItems[schemaName] = rootItem;

        QList<SchemaDefinitionItem*> parents;
        parents << rootItem;

        ConnectSchema* schema = mConnect->getSchema(schemaName);
        QStringList schemaKeys;
        schemaKeys << schemaName;
        foreach(const QString& key, schema->getFirstLevelKeyList()) {
            QList<QVariant> columnData;
            schemaKeys << key;
            columnData << key;
            Schema* s = schema->getSchema(key);
            columnData << (s->required?"Y":"");
            addTypeList(s->types, columnData);
            addValue(s->defaultValue, columnData);
            addValueList(s->allowedValues, columnData);
            addValue(s->min, columnData);
            columnData << QVariant(schemaKeys);
            SchemaDefinitionItem* item = new SchemaDefinitionItem(schemaName, columnData, parents.last());
            parents.last()->appendChild(item);

            if (s->schemaDefined)
                setupTree(schemaName, key, schemaKeys, parents, schema);

            schemaKeys.removeLast();
        }
    }
}

void SchemaDefinitionModel::setupTree(const QString& schemaName, const QString& key,
                                      QStringList& schemaKeys, QList<SchemaDefinitionItem*>& parents, ConnectSchema* schema) {
    QString prefix = key+":-";
    Schema* schemaHelper = schema->getSchema(prefix);
    if (schemaHelper) {
        schemaKeys << "-";
        parents << parents.last()->child(parents.last()->childCount()-1);
        QList<QVariant> listData;
        listData << "schema";
        listData << (schemaHelper->required?"Y":"");;
        addTypeList(schemaHelper->types, listData);
        addValue(schemaHelper->defaultValue, listData);
        addValueList(schemaHelper->allowedValues, listData);
        addValue(schemaHelper->min, listData);
        listData << QVariant(schemaKeys);
        parents.last()->appendChild(new SchemaDefinitionItem(schemaName, listData, parents.last()));

        QStringList nextlevelList = schema->getNextLevelKeyList(prefix);
        if (nextlevelList.size() > 0) {
            parents << parents.last()->child(parents.last()->childCount()-1);
            foreach(const QString& k,  nextlevelList) {
                schemaHelper = schema->getSchema(k);
                QString schemaKeyStr = k.mid(prefix.length()+1);
                if (k.endsWith(":-")) {
                   setupTree(schemaName, k.left(k.lastIndexOf(":")), schemaKeys, parents, schema);
               } else {
                    schemaKeys << schemaKeyStr;
                    QList<QVariant> data;
                    data <<  schemaKeyStr;
                    data << (schemaHelper->required?"Y":"");;
                    addTypeList(schemaHelper->types, data);
                    addValue(schemaHelper->defaultValue, data);
                    addValueList(schemaHelper->allowedValues, data);
                    addValue(schemaHelper->min, data);
                    data << QVariant(schemaKeys);
                    parents.last()->appendChild(new SchemaDefinitionItem(schemaName, data, parents.last()));
                    schemaKeys.removeLast();
               }
            }
            parents.pop_back();
        }
        schemaKeys.removeLast();
    }
    parents.pop_back();
}

} // namespace connect
} // namespace studio
} // namespace gams
