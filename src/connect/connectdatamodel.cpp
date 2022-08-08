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

ConnectDataModel::ConnectDataModel(const QString& filename,  Connect* connect, QObject *parent)
    : QAbstractItemModel{parent},
      mLocation(filename),
      mConnect(connect)
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
        return item->data(index.column());
    }
    case Qt::ForegroundRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        switch(item->data( (int)DataItemColumn::CHECK_STATE ).toInt()) {
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
            if (item->data( (int)DataItemColumn::CHECK_STATE ).toInt() <=1 )
                return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Window));
            else if (item->data( (int)DataItemColumn::CHECK_STATE ).toInt() ==4 )
                    return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Window));
            else
                return QVariant::fromValue(QApplication::palette().color(QPalette::Base));
        }
    }
    case Qt::ToolTipRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        ConnectDataItem *parentItem = item->parentItem();
        if (parentItem != mRootItem) { // TODO
            QModelIndex data_index = index.sibling(index.row(),(int)DataItemColumn::KEY );
            if (index.column()==(int)DataItemColumn::DELETE) {
                return QVariant( QString("delete \"%1\" and all children if there is any").arg( data_index.data(Qt::DisplayRole).toString()) );
            } else if (index.column()==(int)DataItemColumn::MOVE_DOWN) {
                      if (item->data( (int)DataItemColumn::CHECK_STATE ).toInt()==1 &&
                          item->data(0).toInt() < parentItem->childCount()-1)
                          return QVariant( QString("move \"%1\" and all it children down").arg( data_index.data(Qt::DisplayRole).toString()) );
            } else if (index.column()==(int)DataItemColumn::MOVE_UP) {
                      if (item->data(0).toInt() > 0 && item->data( (int)DataItemColumn::CHECK_STATE ).toInt()==1)
                          return QVariant( QString("move \"%1\" and all it children up").arg( data_index.data(Qt::DisplayRole).toString()) );
            } else if (index.column()==(int)DataItemColumn::EXPAND) {
                       break; // TODO
            } else if (index.column()==(int)DataItemColumn::KEY) {
                      if (item->data( (int)DataItemColumn::CHECK_STATE ).toInt()==2)
                          return QVariant( QString("show help for \"%1\" schema").arg( data_index.data(Qt::DisplayRole).toString()) );
                      else if (item->data( (int)DataItemColumn::CHECK_STATE ).toInt()==4)
                               return QVariant( "add another element" );

            }
        } else {
            if (item->data( (int)DataItemColumn::CHECK_STATE ).toInt()==4)
               return QVariant( "add schema data" );

        }
    }
//    case Qt::UserRole: { }
    case Qt::DecorationRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        ConnectDataItem *parentItem = item->parentItem();
        QModelIndex checkstate_index = index.sibling(index.row(),(int)DataItemColumn::CHECK_STATE );
        if (parentItem == mRootItem && index.column()==0) {
            if (checkstate_index.data(Qt::DisplayRole).toInt()==0) {
                return QVariant::fromValue(Theme::icon(":/img/file-edit"));
            } else if (checkstate_index.data(Qt::DisplayRole).toInt()==4) {
                    return QVariant::fromValue(Theme::icon(":/solid/plus"));
            }
        } else { // TODO
            if (index.column()==(int)DataItemColumn::KEY) {
                if (checkstate_index.data(Qt::DisplayRole).toInt()==2) {
                   return QVariant::fromValue(Theme::icon(":/solid/question"));
                } else if (checkstate_index.data(Qt::DisplayRole).toInt()==4) {
                    return QVariant::fromValue(Theme::icon(":/solid/plus"));
                }
            } else {
                if (index.column()==(int)DataItemColumn::DELETE) {
                    if (item->data(index.column()).toBool())
                        return QVariant::fromValue(Theme::icon(":/solid/delete-all"));
                } else if (index.column()==(int)DataItemColumn::MOVE_DOWN) {
                          if (item->data( (int)DataItemColumn::CHECK_STATE ).toInt()==1 &&
                              item->data(0).toInt() < parentItem->childCount()-1)
                              return QVariant::fromValue(Theme::icon(":/solid/move-down"));
                } else if (index.column()==(int)DataItemColumn::MOVE_UP) {
                          if (item->data(0).toInt() > 0 && item->data( (int)DataItemColumn::CHECK_STATE ).toInt()==1)
                              return QVariant::fromValue(Theme::icon(":/solid/move-up"));
                } else if (index.column()==(int)DataItemColumn::EXPAND) {
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
    } else if (index.column()==(int)DataItemColumn::KEY) {
        return Qt::NoItemFlags;
    } else if (index.column()==(int)DataItemColumn::VALUE) {
            if (item->data( (int)DataItemColumn::CHECK_STATE ).toInt()<=2 ||
                item->data( (int)DataItemColumn::CHECK_STATE ).toInt()==4)
               return  Qt::NoItemFlags;
            else
                return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    } else if (index.column()>(int)DataItemColumn::CHECK_STATE) {
               return  Qt::NoItemFlags;
//              return  Qt::ItemIsEnabled | Qt::ItemIsSelectable | QAbstractItemModel::flags(index);
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

    mConnectData = data;
    setupTreeItemModelData();

    endResetModel();

    emit dataChanged(index(0, (int)DataItemColumn::MOVE_DOWN),
                     index(rowCount() - 1, (int)DataItemColumn::EXPAND),
                     QVector<int> { Qt::DisplayRole, Qt::ToolTipRole, Qt::DecorationRole} );
}

void ConnectDataModel::setupTreeItemModelData()
{
    QList<QVariant> rootData;
    rootData << "Key" << "Value"
             << "State"
             << "A0"  << "A2" << "A3" << "A4";

    mRootItem = new ConnectDataItem(rootData);

    QList<ConnectDataItem*> parents;
    parents << mRootItem;

    QList<QVariant> connectData;
    connectData << mLocation;
    connectData << "";
    connectData << "0";
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
        schemaindexData << "1";
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
             listData << "2";
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
                     itemData << "3";
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
                     itemData << "3";
                     itemData << QVariant(!schema->isRequired(key)); // A0
                     itemData << QVariant(false); // A2
                     itemData << ""; // A3
                     itemData << ""; // A4
                     parents.last()->appendChild(new ConnectDataItem(itemData, parents.last()));
                     parents << parents.last()->child(parents.last()->childCount()-1);
                     for (YAML::const_iterator dmit = mit->second.begin(); dmit != mit->second.end(); ++dmit) {
                         QString mapkey = QString::fromStdString(dmit->first.as<std::string>());
                         dataKeys << mapkey;
                         QList<QVariant> mapitemData;
                         mapitemData << mapkey;
                         Q_ASSERT(dmit->second.Type()==YAML::NodeType::Scalar); // TODO
                         mapitemData << QVariant(dmit->second.as<std::string>().c_str()); // TODO
                         mapitemData << "5";
                         mapitemData << QVariant(!schema->isRequired(key)); // A0
                         mapitemData << ""; // A2
                         mapitemData << ""; // A3
                         mapitemData << ""; // A4
                         parents.last()->appendChild(new ConnectDataItem(mapitemData, parents.last()));
                         dataKeys.removeLast();
                     }
                     parents.pop_back();
                     dataKeys.removeLast();
                 } else if (mit->second.Type()==YAML::NodeType::Sequence) {
                            isMapToSequence = true;
                            QString key = QString::fromStdString(mit->first.as<std::string>());
                            dataKeys << key;
                            QList<QVariant> itemData;
                            itemData << key;
                            itemData << "";
                            itemData << "3";
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
                                indexData << "";
                                indexData << "1";
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
                                      QList<QVariant> mapSeqData;
                                      mapSeqData << key;
                                      mapSeqData << mmit->second.as<std::string>().c_str(); // TODO
                                      mapSeqData << "3";
                                      mapSeqData << QVariant(!schema->isRequired(dataKeys.join(":")));  // A0
                                      mapSeqData << "";  // A2
                                      mapSeqData << "";  // A3
                                      mapSeqData << "";  // A4
                                      parents.last()->appendChild(new ConnectDataItem(mapSeqData, parents.last()));
                                      dataKeys.removeLast();
                                  }
                                  parents.pop_back();
                               } else if (mit->second[k].Type()==YAML::NodeType::Scalar) {
                                   qDebug() << "not yet implemented ";
//                                   parents << parents.last()->child(parents.last()->childCount()-1);
                                   QList<QVariant> mapSeqData;
                                   mapSeqData << "";
                                   mapSeqData << ""; // TODO
                                   mapSeqData << "3";
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
                                if (item->data((int)DataItemColumn::CHECK_STATE).toInt() == 1) {
                                    if (item->data((int)DataItemColumn::KEY).toInt() < parents.last()-> childCount()-1)
                                        item->setData((int)DataItemColumn::MOVE_DOWN, QVariant(true));
                                    else if (item->data((int)DataItemColumn::KEY).toInt() > 0)
                                             item->setData((int)DataItemColumn::MOVE_UP, QVariant(true));
                                }
                            }

                 }
                 if (isMapToSequence) {
                     QList<QVariant> sequenceDummyData;
                     sequenceDummyData << "";
                     sequenceDummyData << "";
                     sequenceDummyData << "4";
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
    sequenceDummyData << "4";
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
