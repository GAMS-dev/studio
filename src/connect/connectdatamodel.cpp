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
#include <QStringListModel>

#include "connectdatamodel.h"
#include "theme.h"
namespace gams {
namespace studio {
namespace connect {

ConnectDataModel::ConnectDataModel(const QString& filename,  Connect* c, QObject *parent)
    : QAbstractItemModel{parent},
      mLocation(filename),
      mConnect(c)
{
    mConnectData = mConnect->loadDataFromFile(mLocation);
    qDebug() << mConnectData->str().c_str();

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
        if (index.column()==(int)DataItemColumn::SchemaType || index.column()==(int)DataItemColumn::AllowedValue)
           return  QVariant(item->data(index.column()).toStringList());
//           return  QVariant(item->data(index.column()).toStringList().join(","));
        else
           return item->data(index.column());
    }
    case Qt::ForegroundRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        switch(item->data( (int)DataItemColumn::CheckState ).toInt()) {
           case 0: return  QVariant::fromValue(Theme::color(Theme::Syntax_declaration));
           case 1: return  QVariant::fromValue(Theme::color(Theme::Syntax_embedded));
           case 2: return  QVariant::fromValue(Theme::color(Theme::Syntax_declaration));
           default: return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        }
    }
    case Qt::BackgroundRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        ConnectDataItem *parentItem = item->parentItem();
        if (parentItem == mRootItem) {
                return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Background));
        } else {
            if (item->data( (int)DataItemColumn::CheckState ).toInt() <= (int)DataCheckState::ListItem )
                return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Window));
            else if (item->data( (int)DataItemColumn::CheckState ).toInt() == (int)DataCheckState::ListAppend ||
                     item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::MapAppend       )
                    return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Window));
            else
                return QVariant::fromValue(QApplication::palette().color(QPalette::Base));
        }
    }
    case Qt::ToolTipRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        ConnectDataItem *parentItem = item->parentItem();
        if (parentItem != mRootItem) { // TODO
            QModelIndex data_index = index.sibling(index.row(),(int)DataItemColumn::Key );
            if (index.column()==(int)DataItemColumn::Delete) {
                return QVariant( QString("delete \"%1\" and all children if there is any").arg( data_index.data(Qt::DisplayRole).toString()) );
            } else if (index.column()==(int)DataItemColumn::MoveDown) {
                      if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ListItem &&
                          item->data(0).toInt() < parentItem->childCount()-1)
                          return QVariant( QString("move \"%1\" and all it children down").arg( data_index.data(Qt::DisplayRole).toString()) );
            } else if (index.column()==(int)DataItemColumn::MoveUp) {
                      if (item->data(0).toInt() > 0 && item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ListItem)
                          return QVariant( QString("move \"%1\" and all it children up").arg( data_index.data(Qt::DisplayRole).toString()) );
            } else if (index.column()==(int)DataItemColumn::Expand) {
                       break; // TODO
            } else if (index.column()==(int)DataItemColumn::Key) {
                      if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::SchemaName)
                          return QVariant( QString("show help for \"%1\" schema").arg( data_index.data(Qt::DisplayRole).toString()) );
                      else if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ListAppend ||
                               item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::MapAppend     )
                               return QVariant( "add element" );

            }
        } else {
            if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ListAppend ||
                item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::MapAppend     )
               return QVariant( "add schema data" );

        }
    }
//    case Qt::UserRole: { }
    case Qt::DecorationRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        ConnectDataItem *parentItem = item->parentItem();
        QModelIndex checkstate_index = index.sibling(index.row(),(int)DataItemColumn::CheckState );
        if (parentItem == mRootItem && index.column()==0) {
            if (checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::Root) {
                return QVariant::fromValue(Theme::icon(":/img/file-edit"));
            } else if (checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::ListAppend ||
                       checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::MapAppend     ) {
                    return QVariant::fromValue(Theme::icon(":/solid/plus"));
            }
        } else { // TODO
            if (index.column()==(int)DataItemColumn::Key) {
                if (checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::SchemaName) {
                   return QVariant::fromValue(Theme::icon(":/solid/question"));
                } else if (checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::ListAppend ||
                           checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::MapAppend     ) {
                    return QVariant::fromValue(Theme::icon(":/solid/plus"));
                }
            } else {
                if (index.column()==(int)DataItemColumn::Delete) {
                    if (item->data(index.column()).toBool())
                        return QVariant::fromValue(Theme::icon(":/solid/delete-all"));
                } else if (index.column()==(int)DataItemColumn::MoveDown) {
                          if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ListItem &&
                              item->data(0).toInt() < parentItem->childCount()-1)
                              return QVariant::fromValue(Theme::icon(":/solid/move-down"));
                } else if (index.column()==(int)DataItemColumn::MoveUp) {
                          if (item->data(0).toInt() > 0 && item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ListItem)
                              return QVariant::fromValue(Theme::icon(":/solid/move-up"));
                } else if (index.column()==(int)DataItemColumn::Expand) {
                           break;
                }
            }
        }
        break;
    }

    default:
         break;
    }
    return QVariant();

}

Qt::ItemFlags ConnectDataModel::flags(const QModelIndex &index) const
{
    ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else if (index.column()==(int)DataItemColumn::Key) {
               if (item->data( Qt::DisplayRole ).toInt()==(int)DataCheckState::ElementMap ||
                   item->data( Qt::DisplayRole ).toInt()==(int)DataCheckState::ElementKey ||
                   item->data( Qt::DisplayRole ).toInt()==(int)DataCheckState::ElementValue  )
                   return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
               else
                   return Qt::NoItemFlags;
    } else if (index.column()==(int)DataItemColumn::Value) {
              if (item->data( Qt::DisplayRole ).toInt()==(int)DataCheckState::ElementKey   ||
                  item->data( Qt::DisplayRole ).toInt()==(int)DataCheckState::ElementValue ||
                  item->data( Qt::DisplayRole ).toInt()==(int)DataCheckState::ElementMap      )
                  return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
              else if (item->data( (int)DataItemColumn::CheckState ).toInt()<=(int)DataCheckState::ElementKey ||
                  item->data( (int)DataItemColumn::CheckState ).toInt()>=(int)DataCheckState::ListAppend)
                     return  Qt::NoItemFlags;
              else
                    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    } else if (index.column()>(int)DataItemColumn::CheckState) {
               return  Qt::NoItemFlags;
    } else {
        return QAbstractItemModel::flags(index);
    }
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

    Q_ASSERT(data->getRootNode().Type()==YAML::NodeType::Sequence);
    YAML::Node root = mConnectData->getRootNode();
    if (root.IsNull()) {
        mConnectData = data;
    } else {
        int size = root.size();
        for (size_t i= 0; i<data->getRootNode().size(); i++) {
             root[size+i] = data->getRootNode()[i];
         }
    }
    setupTreeItemModelData();

    endResetModel();


    emit dataChanged(index(0, (int)DataItemColumn::MoveDown),
                     index(rowCount() - 1, (int)DataItemColumn::Expand),
                     QVector<int> { Qt::DisplayRole, Qt::ToolTipRole, Qt::DecorationRole} );
}

void ConnectDataModel::setupTreeItemModelData()
{
    QList<QVariant> rootData;
    rootData << "Key" << "Value"
             << "State" << "Type" << "AllowedValue"
             << ""  << "" << "" << "";
//             << "A0"  << "A2" << "A3" << "A4";

    mRootItem = new ConnectDataItem(rootData);

    QList<ConnectDataItem*> parents;
    parents << mRootItem;

    QList<QVariant> connectData;
    connectData << mLocation;
    connectData << "";
    connectData << "0";
    connectData << QVariant(QStringList());
    connectData << QVariant(QStringList());
    connectData << " "; // A0
    connectData << " "; // A2
    connectData << " "; // A3
    connectData << " "; // A4
    parents.last()->appendChild(new ConnectDataItem(connectData, parents.last()));

    QString currentSchemaName;
    for(size_t i = 0; i<mConnectData->getRootNode().size(); i++) {
        parents << parents.last()->child(parents.last()->childCount()-1);

        Q_ASSERT(mConnectData->getRootNode()[i].Type()==YAML::NodeType::Map); // TODO
        YAML::Node rootnode = mConnectData->getRootNode()[i];

        QList<QVariant> schemaindexData;
        schemaindexData << QVariant::fromValue(i);
        schemaindexData << "";
        schemaindexData << QVariant((int)DataCheckState::ListItem);
        schemaindexData << QVariant(QStringList());
        schemaindexData << QVariant(QStringList());
        schemaindexData << QVariant(true);  // A0
        schemaindexData << "";  // A2
        schemaindexData << "";  // A3
        schemaindexData << "";  // A4
        parents.last()->appendChild(new ConnectDataItem(schemaindexData, parents.last()));

        for (YAML::const_iterator it = rootnode.begin(); it != rootnode.end(); ++it) {
             parents << parents.last()->child(parents.last()->childCount()-1);
             currentSchemaName = QString::fromStdString(it->first.as<std::string>());
             ConnectSchema* schema = mConnect->getSchema(currentSchemaName);
             QStringList dataKeys;
             QList<QVariant> listData;
             listData << currentSchemaName;
             listData << "";
             listData << QVariant((int)DataCheckState::SchemaName);
             listData << QVariant(QStringList());
             listData << QVariant(QStringList());
             listData << QVariant(false); // A0
             listData << ""; // A2
             listData << ""; // A3
             listData << ""; // A4
             parents.last()->appendChild(new ConnectDataItem(listData, parents.last()));

             parents << parents.last()->child(parents.last()->childCount()-1);
             Q_ASSERT(it->second.Type()==YAML::NodeType::Map); // TODO
             for (YAML::const_iterator mit = it->second.begin(); mit != it->second.end(); ++mit) {

                 bool isMapToSequence = false;
                 if (mit->second.Type()==YAML::NodeType::Scalar) {
                     QString key = QString::fromStdString(mit->first.as<std::string>());
                     dataKeys << key;
                     QList<QVariant> itemData;
                     itemData << key;
                     itemData << QVariant(mit->second.as<std::string>().c_str()); // TODO
                     itemData << QVariant((int)DataCheckState::ElementValue);
                     itemData << QVariant(schema->getTypeAsStringList(key));
                     itemData << QVariant(schema->getAllowedValueAsStringList(key));
                     itemData << QVariant(!schema->isRequired(key)); // A0
                     itemData << QVariant(false); // A2
                     itemData << ""; // A3
                     itemData << ""; // A4
                     parents.last()->appendChild(new ConnectDataItem(itemData, parents.last()));

                     dataKeys.removeLast();
                 } else if (mit->second.Type()==YAML::NodeType::Map) {
                     qDebug() << "not implemented " << QString::fromStdString(mit->first.as<std::string>());
                     QString key = QString::fromStdString(mit->first.as<std::string>());
                     dataKeys << key;
                     QList<QVariant> itemData;
                     itemData << key;
                     itemData << ""; // TODO
                     itemData << QVariant((int)DataCheckState::KeyItem);
                     itemData << QVariant(schema->getTypeAsStringList(key));
                     itemData << QVariant(QStringList());
                     itemData << QVariant(!schema->isRequired(key)); // A0
                     itemData << QVariant(false); // A2
                     itemData << ""; // A3
                     itemData << ""; // A4
                     parents.last()->appendChild(new ConnectDataItem(itemData, parents.last()));
                     parents << parents.last()->child(parents.last()->childCount()-1);
                     int k = 0;
                     for (YAML::const_iterator dmit = mit->second.begin(); dmit != mit->second.end(); ++dmit) {
                         QString mapkey = QString::fromStdString(dmit->first.as<std::string>());
                         dataKeys << mapkey;
                         QList<QVariant> mapitemData;
                         mapitemData << mapkey;
                         Q_ASSERT(dmit->second.Type()==YAML::NodeType::Scalar); // TODO
                         mapitemData << QVariant(dmit->second.as<std::string>().c_str()); // TODO
                         mapitemData << QVariant((int)DataCheckState::ElementMap);
                         mapitemData << QVariant(schema->getTypeAsStringList(key));
                         mapitemData << QVariant(QStringList());
                         mapitemData << QVariant(!schema->isRequired(key)); // A0
                         mapitemData << ""; // A2
                         mapitemData << ""; // A3
                         mapitemData << ""; // A4
                         parents.last()->appendChild(new ConnectDataItem(mapitemData, parents.last()));
                         dataKeys.removeLast();
                         k++;
                     }
                     parents.pop_back();
                     dataKeys.removeLast();

                     QList<QVariant> sequenceDummyData;
                     sequenceDummyData << "";
                     sequenceDummyData << "";
                     sequenceDummyData << QVariant((int)DataCheckState::MapAppend);
                     sequenceDummyData << QVariant(QStringList());
                     sequenceDummyData << QVariant(QStringList());
                     sequenceDummyData << QVariant(false); // A0
                     sequenceDummyData << QVariant(false); // A2
                     sequenceDummyData << QVariant(false); // A3
                     sequenceDummyData << QVariant(false); // A4
                     parents.last()->appendChild(new ConnectDataItem(sequenceDummyData, parents.last()));
                 } else if (mit->second.Type()==YAML::NodeType::Sequence) {
                            isMapToSequence = true;
                            QString key = QString::fromStdString(mit->first.as<std::string>());
                            dataKeys << key;
                            qDebug() << key << ">>" << dataKeys;
                            QList<QVariant> itemData;
                            itemData << key;
                            itemData << "";
                            itemData << QVariant((int)DataCheckState::KeyItem);
                            itemData << QVariant(schema->getTypeAsStringList(key));
                            itemData << QVariant(QStringList());
                            itemData << QVariant(!schema->isRequired(key)); // A0
                            itemData << ""; // A2
                            itemData << ""; // A3
                            itemData << ""; // A4
                            parents.last()->appendChild(new ConnectDataItem(itemData, parents.last()));
                            parents << parents.last()->child(parents.last()->childCount()-1);
                            dataKeys << "-";
                            for(size_t k = 0; k<mit->second.size(); k++) {
                                QList<QVariant> indexData;
                                indexData << QVariant::fromValue(k);
                                indexData << QVariant(QStringList());
                                indexData << QVariant((int)DataCheckState::ListItem);
                                indexData << QVariant(QStringList());
                                indexData << QVariant(QStringList());
                                indexData << QVariant(!schema->isRequired(dataKeys.join(":"))); // A0
                                indexData << ""; // A2
                                indexData << ""; // A3
                                indexData << ""; // A4
                                parents.last()->appendChild(new ConnectDataItem(indexData, parents.last()));

                               if (mit->second[k].Type()==YAML::NodeType::Map) {
                                  parents << parents.last()->child(parents.last()->childCount()-1);
                                  const YAML::Node mapnode = mit->second[k];
                                  for (YAML::const_iterator mmit = mapnode.begin(); mmit != mapnode.end(); ++mmit) {
                                      key =  QString::fromStdString( mmit->first.as<std::string>() );
                                      dataKeys << key;

                                     if (mmit->second.Type()==YAML::NodeType::Sequence) {

                                         QList<QVariant> seqSeqData;
                                         seqSeqData << key;
                                         seqSeqData << "";
                                         seqSeqData << QVariant((int)DataCheckState::KeyItem);
                                         seqSeqData << QVariant(schema->getTypeAsStringList(key));
                                         seqSeqData << QVariant(QStringList());
                                         seqSeqData << QVariant(!schema->isRequired(key)); // A0
                                         seqSeqData << ""; // A2
                                         seqSeqData << ""; // A3
                                         seqSeqData << ""; // A4
                                         parents.last()->appendChild(new ConnectDataItem(seqSeqData, parents.last()));

                                         dataKeys << "-";
                                         parents << parents.last()->child(parents.last()->childCount()-1);
                                         for(size_t kk = 0; kk<mmit->second.size(); kk++) {
                                             QList<QVariant> indexSeqData;
                                             indexSeqData << QVariant::fromValue(kk);
                                             indexSeqData << QVariant(QStringList());
                                             indexSeqData << QVariant((int)DataCheckState::ListItem);
                                             indexSeqData << QVariant(QStringList());
                                             indexSeqData << QVariant(QStringList());
                                             indexSeqData << QVariant(!schema->isRequired(dataKeys.join(":"))); // A0
                                             indexSeqData << ""; // A2
                                             indexSeqData << ""; // A3
                                             indexSeqData << ""; // A4
                                             parents.last()->appendChild(new ConnectDataItem(indexSeqData, parents.last()));

                                            if (mmit->second[kk].Type()==YAML::NodeType::Scalar) {
                                                parents << parents.last()->child(parents.last()->childCount()-1);
                                                 QList<QVariant> indexScalarData;
                                                indexScalarData << mmit->second[kk].as<std::string>().c_str();
                                                indexScalarData << ""; // TODO
                                                indexScalarData << QVariant((int)DataCheckState::ElementValue);
                                                indexScalarData << QVariant(QStringList());
                                                indexScalarData << QVariant(QStringList());
                                                indexScalarData << "";  // A0
                                                indexScalarData << "";  // A2
                                                indexScalarData << "";  // A3
                                                indexScalarData << "";  // A4
                                                parents.last()->appendChild(new ConnectDataItem(indexScalarData, parents.last()));
                                                parents.pop_back();
                                             } // TODO: else

                                             QList<QVariant> indexSeqDummyData;
                                             indexSeqDummyData << "";
                                             indexSeqDummyData << "";
                                             indexSeqDummyData << QVariant((int)DataCheckState::ListAppend);
                                             indexSeqDummyData << QVariant(QStringList());
                                             indexSeqDummyData << QVariant(QStringList());
                                             indexSeqDummyData << QVariant(false);  // A0
                                             indexSeqDummyData << QVariant(false);  // A2
                                             indexSeqDummyData << QVariant(false);  // A3
                                             indexSeqDummyData << QVariant(false);  // A4
                                             parents.last()->appendChild(new ConnectDataItem(indexSeqDummyData, parents.last()));

                                         }
                                         parents.pop_back();
                                         dataKeys.removeLast();

                                     } else if (mmit->second.Type()==YAML::NodeType::Map) {
                                                QList<QVariant> mapData;
                                                mapData << key;
                                                mapData << "";
                                                mapData << QVariant((int)DataCheckState::KeyItem);
                                                mapData << QVariant(schema->getTypeAsStringList(dataKeys.join(":")));
                                                mapData << QVariant(QStringList());
                                                mapData << QVariant(!schema->isRequired(dataKeys.join(":"))); // A0
                                                mapData << QVariant(false); // A2
                                                mapData << ""; // A3
                                                mapData << ""; // A4
                                                parents.last()->appendChild(new ConnectDataItem(mapData, parents.last()));

                                                parents << parents.last()->child(parents.last()->childCount()-1);
                                                const YAML::Node mapmapnode = mmit->second;
                                                for (YAML::const_iterator mmmit = mapmapnode.begin(); mmmit != mapmapnode.end(); ++mmmit) {
                                                     QList<QVariant> mapSeqData;
                                                     mapSeqData << mmmit->first.as<std::string>().c_str();
                                                     mapSeqData << mmmit->second.as<std::string>().c_str();  // can be int/bool/double
                                                     mapSeqData << QVariant((int)DataCheckState::ElementMap);
                                                     mapSeqData << QVariant(schema->getTypeAsStringList(dataKeys.join(":")));
                                                     mapSeqData << QVariant(QVariant(schema->getAllowedValueAsStringList(dataKeys.join(":"))));
                                                     mapSeqData << QVariant(!schema->isRequired(dataKeys.join(":")));  // A0
                                                     mapSeqData << "";  // A2
                                                     mapSeqData << "";  // A3
                                                     mapSeqData << "";  // A4
                                                     parents.last()->appendChild(new ConnectDataItem(mapSeqData, parents.last()));
                                                     dataKeys.removeLast();
                                                }
                                                parents.pop_back();

                                                QList<QVariant> indexSeqDummyData;
                                                indexSeqDummyData << "";
                                                indexSeqDummyData << "";
                                                indexSeqDummyData << QVariant((int)DataCheckState::MapAppend);
                                                indexSeqDummyData << QVariant(QStringList());
                                                indexSeqDummyData << QVariant(QStringList());
                                                indexSeqDummyData << QVariant(false);  // A0
                                                indexSeqDummyData << QVariant(false);  // A2
                                                indexSeqDummyData << QVariant(false);  // A3
                                                indexSeqDummyData << QVariant(false);  // A4
                                                parents.last()->appendChild(new ConnectDataItem(indexSeqDummyData, parents.last()));

                                     } else if (mmit->second.Type()==YAML::NodeType::Scalar) {
                                         QList<QVariant> mapSeqData;
                                         mapSeqData << key;
                                         mapSeqData << mmit->second.as<std::string>().c_str(); // TODO
                                         mapSeqData << QVariant((int)DataCheckState::ElementValue);
                                         mapSeqData << QVariant(schema->getTypeAsStringList(dataKeys.join(":")));
                                         mapSeqData << QVariant(QVariant(schema->getAllowedValueAsStringList(dataKeys.join(":"))));
                                         mapSeqData << QVariant(!schema->isRequired(dataKeys.join(":")));  // A0
                                         mapSeqData << "";  // A2
                                         mapSeqData << "";  // A3
                                         mapSeqData << "";  // A4
                                         parents.last()->appendChild(new ConnectDataItem(mapSeqData, parents.last()));
                                         dataKeys.removeLast();
                                     }
                                  }
                                  parents.pop_back();
                               } else if (mit->second[k].Type()==YAML::NodeType::Scalar) {
                                   qDebug() << "not yet implemented ";
//                                   parents << parents.last()->child(parents.last()->childCount()-1);
                                   QList<QVariant> mapSeqData;
                                   mapSeqData << "";
                                   mapSeqData << ""; // TODO
                                   mapSeqData << QVariant((int)DataCheckState::ElementValue);
                                   mapSeqData << QVariant(QStringList());
                                   mapSeqData << QVariant(QStringList());
                                   mapSeqData << "";  // A0
                                   mapSeqData << "";  // A2
                                   mapSeqData << "";  // A3
                                   mapSeqData << "";  // A4
                                   parents.last()->appendChild(new ConnectDataItem(mapSeqData, parents.last()));
//                                   parents.pop_back();
                               }
                            }
                            dataKeys.removeLast();
                            parents.pop_back();

                            // update data in MOVE_DOWN and MOVE_UP column
                            for(int row = 0; row<parents.last()->childCount(); row++) {
                                ConnectDataItem* item = parents.last()->child(row);
                                if (item->data((int)DataItemColumn::CheckState).toInt() == 1) {
                                    if (item->data((int)DataItemColumn::Key).toInt() < parents.last()-> childCount()-1)
                                        item->setData((int)DataItemColumn::MoveDown, QVariant(true));
                                    else if (item->data((int)DataItemColumn::Key).toInt() > 0)
                                             item->setData((int)DataItemColumn::MoveUp, QVariant(true));
                                }
                            }

                 }
                 if (isMapToSequence) {
                     QList<QVariant> sequenceDummyData;
                     sequenceDummyData << "";
                     sequenceDummyData << "";
                     sequenceDummyData << QVariant((int)DataCheckState::MapAppend);
                     sequenceDummyData << QVariant(QStringList());
                     sequenceDummyData << QVariant(QStringList());
                     sequenceDummyData << QVariant(false); // A0
                     sequenceDummyData << QVariant(false); // A2
                     sequenceDummyData << QVariant(false); // A3
                     sequenceDummyData << QVariant(false); // A4
                     parents.last()->appendChild(new ConnectDataItem(sequenceDummyData, parents.last()));
                 }

             }
             parents.pop_back();
             parents.pop_back();
        }
        parents.pop_back();
    }
/*
    ConnectDataItem* item = parents.last();
    ConnectDataItem* parent = item->parent();
    if (item->data((int)DataItemColumn::CHECK_STATE).toInt() == 1) {
        if (item->data((int)DataItemColumn::KEY).toInt() < parent-> childCount()-1)
            item->setData((int)DataItemColumn::MOVE_DOWN, QVariant(true));
        else if (item->data((int)DataItemColumn::KEY).toInt() > 0)
                 item->setData((int)DataItemColumn::MOVE_UP, QVariant(true));
    }
*/
    QList<QVariant> sequenceDummyData;
    sequenceDummyData << "";
    sequenceDummyData << "";
    sequenceDummyData << QVariant((int)DataCheckState::ListAppend);
    sequenceDummyData << QVariant(QStringList());
    sequenceDummyData << QVariant(QStringList());
    sequenceDummyData << QVariant(false);  // A0
    sequenceDummyData << QVariant(false);  // A2
    sequenceDummyData << QVariant(false);  // A3
    sequenceDummyData << QVariant(false);  // A4
    parents.last()->appendChild(new ConnectDataItem(sequenceDummyData, parents.last()));

    parents.pop_back();
}

} // namespace connect
} // namespace studio
} // namespace gams
