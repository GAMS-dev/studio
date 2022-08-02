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

#include "connectdatamodel.h"
#include "theme.h"
namespace gams {
namespace studio {
namespace connect {

ConnectDataModel::ConnectDataModel(const QString& filename, ConnectData* data, QObject *parent)
    : QAbstractItemModel{parent},
      mLocation(filename),
      mConnectData(data)
{
    setupTreeItemModelData();
}

ConnectDataModel::~ConnectDataModel()
{
    if (mRootItem)
        delete mRootItem;
}

QVariant ConnectDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        return item->data(index.column());
    }
    case Qt::ForegroundRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        switch(item->data(2).toInt()) {
           case 0: return  QVariant::fromValue(Theme::color(Theme::Syntax_declaration));
           case 1: return  QVariant::fromValue(Theme::color(Theme::Syntax_declaration));
           case 2: return  QVariant::fromValue(Theme::color(Theme::Syntax_embedded));
           default: return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        }
    }
    case Qt::BackgroundRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        ConnectDataItem *parentItem = item->parentItem();
        if (parentItem == mRootItem) {
                return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Background));
        } else {
            if (item->data(2).toInt() <=1 )
                return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Window));
            else
                return QVariant::fromValue(QApplication::palette().color(QPalette::Base));
        }
    }
    case Qt::DecorationRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        ConnectDataItem *parentItem = item->parentItem();
        if (parentItem == mRootItem) {
            return QVariant::fromValue(Theme::icon(":/img/file-edit"));
        } else
            if (item->data(2).toInt()==1 && index.column()==0) {
                return QVariant::fromValue(Theme::icon(":/solid/new"));
            }
    }

    default:
         break;
    }
    return QVariant();

}

Qt::ItemFlags ConnectDataModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::NoItemFlags;
    else
        return Qt::ItemIsDragEnabled | defaultFlags;  // ToDo
}

bool ConnectDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QVector<int> roles;
    switch (role) {
    case Qt::DisplayRole: {
        roles = { Qt::EditRole };
        ConnectDataItem *item = getItem(index);
        bool result = item->setData(index.column(), value);
        if (result)
            emit dataChanged(index, index, roles);
        return result;
    }
//    case Qt::CheckStateRole: {  }
    default:
        break;
    }
    return false;
}

QVariant ConnectDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return mRootItem->data(section);

    return QVariant();
}

QModelIndex ConnectDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ConnectDataItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<ConnectDataItem*>(parent.internalPointer());

    ConnectDataItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ConnectDataModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ConnectDataItem *childItem = static_cast<ConnectDataItem*>(index.internalPointer());
    ConnectDataItem *parentItem = childItem->parentItem();

    if (parentItem == mRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);

}

int ConnectDataModel::rowCount(const QModelIndex &parent) const
{
    ConnectDataItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<ConnectDataItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int ConnectDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ConnectDataItem*>(parent.internalPointer())->columnCount();
    else
        return mRootItem->columnCount();
}

ConnectDataItem *ConnectDataModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return mRootItem;
}

void ConnectDataModel::insertItem(int position, ConnectDataItem *item, const QModelIndex &parent)
{
    ConnectDataItem* parentItem = getItem(parent);

    beginInsertRows(parent, position, position);
    parentItem->insertChild(position, item);
    endInsertRows();
}

bool ConnectDataModel::removeItem(const QModelIndex &index)
{
    ConnectDataItem* treeItem = getItem(index);
    if (treeItem)
        return removeRows(treeItem->row(), 1, parent(index));
    else
        return false;
}

bool ConnectDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    ConnectDataItem* parentItem = getItem(parent);
    bool success = false;
    if (count > 0) {
        beginRemoveRows(parent, row, row + count - 1);
        success = parentItem->removeChildren(row, count);
        endRemoveRows();
    }
    return success;
}

void ConnectDataModel::addFromSchema(ConnectData* data)
{
    qDebug() << data->str().c_str();

    beginResetModel();
    endResetModel();

    mConnectData = data;
    setupTreeItemModelData();
}

void ConnectDataModel::setupTreeItemModelData()
{
    QList<QVariant> rootData;
    rootData << "Key" << "Value"  << "Status";

    mRootItem = new ConnectDataItem(rootData);

    QList<ConnectDataItem*> parents;
    parents << mRootItem;

    QList<QVariant> connectData;
    connectData << mLocation;
    connectData << "";
    connectData << "0";
    parents.last()->appendChild(new ConnectDataItem(connectData, parents.last()));

    Q_ASSERT(mConnectData->getRootNode().Type()==YAML::NodeType::Sequence); // TODO
    for(size_t i = 0; i<mConnectData->getRootNode().size(); i++) {
        parents << parents.last()->child(parents.last()->childCount()-1);

        Q_ASSERT(mConnectData->getRootNode()[i].Type()==YAML::NodeType::Map); // TODO
        YAML::Node rootnode = mConnectData->getRootNode()[i];

        QList<QVariant> schemaindexData;
        schemaindexData << QVariant::fromValue(i);
        schemaindexData << "";
        schemaindexData << "1";
        parents.last()->appendChild(new ConnectDataItem(schemaindexData, parents.last()));

        for (YAML::const_iterator it = rootnode.begin(); it != rootnode.end(); ++it) {
             parents << parents.last()->child(parents.last()->childCount()-1);
             QList<QVariant> listData;
             listData << it->first.as<std::string>().c_str();
             listData << "";
             listData << "2";
             parents.last()->appendChild(new ConnectDataItem(listData, parents.last()));

             parents << parents.last()->child(parents.last()->childCount()-1);
             Q_ASSERT(it->second.Type()==YAML::NodeType::Map); // TODO
             for (YAML::const_iterator mit = it->second.begin(); mit != it->second.end(); ++mit) {

                 if (mit->second.Type()==YAML::NodeType::Scalar) {
                     QList<QVariant> itemData;
                     itemData << mit->first.as<std::string>().c_str();
                     itemData << QVariant(mit->second.as<std::string>().c_str()); // TODO
                     itemData << "3";
                     parents.last()->appendChild(new ConnectDataItem(itemData, parents.last()));
                 } else if (mit->second.Type()==YAML::NodeType::Map) {
                     qDebug() << "not implemented";
                 } else if (mit->second.Type()==YAML::NodeType::Sequence) {
                            QList<QVariant> itemData;
                            itemData << mit->first.as<std::string>().c_str();
                            itemData << "";
                            itemData << "3";
                            parents.last()->appendChild(new ConnectDataItem(itemData, parents.last()));
                            parents << parents.last()->child(parents.last()->childCount()-1);
                            for(size_t k = 0; k<mit->second.size(); k++) {
                               QList<QVariant> indexData;
                               indexData << QVariant::fromValue(k);
                               indexData << "";
                               indexData << "1";
                               parents.last()->appendChild(new ConnectDataItem(indexData, parents.last()));

                               if (mit->second[k].Type()==YAML::NodeType::Map) {
                                  parents << parents.last()->child(parents.last()->childCount()-1);
                                  const YAML::Node mapnode = mit->second[k];
                                  for (YAML::const_iterator mmit = mapnode.begin(); mmit != mapnode.end(); ++mmit) {
                                       QList<QVariant> mapSeqData;
                                       mapSeqData << mmit->first.as<std::string>().c_str();
                                       mapSeqData << mmit->second.as<std::string>().c_str(); // TODO
                                       mapSeqData << "3";
                                       parents.last()->appendChild(new ConnectDataItem(mapSeqData, parents.last()));
                                  }
                                  parents.pop_back();
                               } else if (mit->second[k].Type()==YAML::NodeType::Scalar) {
                                   qDebug() << "not yet implemented";
                               }
                           }
                           parents.pop_back();
                 }
             }
             parents.pop_back();
             parents.pop_back();
        }
        parents.pop_back();
    }
    parents.pop_back();
}

} // namespace connect
} // namespace studio
} // namespace gams
