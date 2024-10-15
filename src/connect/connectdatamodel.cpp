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
#include <QColor>
#include <QApplication>
#include <QPalette>
#include <QStringListModel>
#include <QMimeData>
#include <QDir>
#include <QRegularExpression>

#include "connectdatamodel.h"
#include "theme.h"
#include "commonpaths.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace connect {

const QString ConnectDataModel::TooltipStrHeader = QString("<html><head/><body>");
const QString ConnectDataModel::TooltipStrFooter = QString("</body></html>");
const QString ConnectDataModel::TooltipOpenedBoldStr = QString("<span style=' font-weight:600;'>");
const QString ConnectDataModel::TooltipClosedBoldStr = QString("</span>");
QRegularExpression ConnectDataModel::mRexWhitechar("\\s+");
QRegularExpression ConnectDataModel::mRexNumber("^\\[\\d\\d?\\]$");

ConnectDataModel::ConnectDataModel(const QString& filename,  Connect* c, QObject *parent)
    : QAbstractItemModel{parent},
      mOnlyRequriedAttributesAdded(false),
      mItemIDCount(0),
      mLocation(filename),
      mConnect(c)
{
    try {
        mConnectData = mConnect->loadDataFromFile(mLocation);
    } catch (std::exception &e) {
        EXCEPT() << e.what();
    }

    setupTreeItemModelData();
}

ConnectDataModel::~ConnectDataModel()
{
    if (mRootItem)
        delete mRootItem;
    if (mConnectData)
        delete mConnectData;
}

QVariant ConnectDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::EditRole: {

    }
    case Qt::DisplayRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        if (index.column()==(int)DataItemColumn::SchemaType || index.column()==(int)DataItemColumn::AllowedValue) {
           return  QVariant(item->data(index.column()).toStringList());
        } else if (index.column()==(int)DataItemColumn::Key) {
                  QModelIndex checkstate_index = index.sibling(index.row(),(int)DataItemColumn::CheckState );
                  if (checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::ListItem) {
                     return QVariant(index.row());
                  } else if (checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::ListAppend) {
                      QString key = item->data(Qt::DisplayRole).toString();
                      if (key.contains("["))
                          return QVariant(key.left(item->data(Qt::DisplayRole).toString().indexOf("[")));
                  }
                  return item->data(index.column());
        } else if (index.column()==(int)DataItemColumn::ElementID) {
                 return QVariant(item->id());
        } else if (index.column()==(int)DataItemColumn::SchemaKey) {
                 return QVariant(item->data(index.column()).toStringList().join(":"));
        } else if (index.column()==(int)DataItemColumn::Undefined) {
                  return QVariant(item->data(index.column()));
        } else {
            return item->data(index.column());
        }
    }
    case Qt::ForegroundRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        if (item->data((int)DataItemColumn::Undefined).toBool()) {
            if (index.column() == (int)DataItemColumn::Key &&
               !item->parentItem()->data((int)DataItemColumn::Undefined).toBool())
                return  QVariant::fromValue(Theme::color(Theme::Normal_Red));
        }

        int state = item->data( (int)DataItemColumn::CheckState ).toInt();
        bool invalid = (item->data((int)DataItemColumn::InvalidValue).toInt()>0);
        if (state==(int)DataCheckState::Root) {
            return  QVariant::fromValue(Theme::color(Theme::Syntax_declaration));
        } else if (state==(int)DataCheckState::SchemaName) {
                  if (invalid  && (index.column()==(int)DataItemColumn::Key || (state==(int)DataCheckState::KeyItem)) )
                      return  QVariant::fromValue(Theme::color(Theme::Normal_Red));
                   else
                  return  QVariant::fromValue(Theme::color(Theme::Normal_Green));
        } else if (state==(int)DataCheckState::ListItem || state==(int)DataCheckState::MapAppend
                                                        || state==(int)DataCheckState::MapSchemaAppend
                                                        || state==(int)DataCheckState::ListAppend) {
                 return  QVariant::fromValue(Theme::color(Theme::Disable_Gray));
        } else if (state==(int)DataCheckState::SchemaAppend) {
                  return  QVariant::fromValue(Theme::color(Theme::Active_Gray));
        } else if (state==(int)DataCheckState::ElementValue) {
                  if (invalid) {
                     if (index.column()==(int)DataItemColumn::Key) {
                        return QVariant::fromValue(Theme::color(Theme::Normal_Red));
                     }
                     if (index.column()==(int)DataItemColumn::Value) {
                        bool undefined = item->data((int)DataItemColumn::Undefined).toBool();
                        return (invalid && !undefined ? QVariant::fromValue(Theme::color(Theme::Normal_Red))
                                                      : QVariant::fromValue(Theme::color(Theme::Syntax_keyword)) );
                     }
                  } else {
                      bool deletable = item->data((int)DataItemColumn::Delete).toBool();
                      QString str = item->data((int)DataItemColumn::Value).toString();
                      if (!deletable) {
                          if (index.column()==(int)DataItemColumn::Value && QString::compare(str,"[value]",Qt::CaseInsensitive)==0) {
                              return  QVariant::fromValue(Theme::color(Theme::Normal_Blue));
                          }
                      }
                      return QVariant::fromValue(Theme::color(Theme::Syntax_keyword));
                  }
                  return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        } else if (state==(int)DataCheckState::ElementMap) {
            if (invalid) {
                if (index.column()==(int)DataItemColumn::Value) {
                    return  QVariant::fromValue(Theme::color(Theme::Normal_Red));
                }
            } else {
                bool deletable = (item->parentItem() ? item->parentItem()->data((int)DataItemColumn::Delete).toBool() : true);
                if (!deletable) {
                    if (index.column()==(int)DataItemColumn::Key && QString::compare(item->data((int)DataItemColumn::Key).toString(), "[key]", Qt::CaseInsensitive)==0)
                        return  QVariant::fromValue(Theme::color(Theme::Normal_Blue));
                    if (index.column()==(int)DataItemColumn::Value && QString::compare(item->data((int)DataItemColumn::Value).toString(), "[value]", Qt::CaseInsensitive)==0)
                        return  QVariant::fromValue(Theme::color(Theme::Normal_Blue));
                }
                return QVariant::fromValue(Theme::color(Theme::Syntax_keyword));
            }
            return  QVariant::fromValue(Theme::color(Theme::Syntax_keyword));
        } else if (state==(int)DataCheckState::ElementKey) {
                  if (invalid) {
                     return  QVariant::fromValue(Theme::color(Theme::Normal_Red));
                  } else {
                      bool deletable = true;
                      if (item->parentItem()) {
                          if (item->parentItem()->data((int)DataItemColumn::SchemaKey).toString().endsWith("-")) {
                              if (item->parentItem()->parentItem()) {
                                  deletable = item->parentItem()->parentItem()->data((int)DataItemColumn::Delete).toBool();
                              }
                          }
                      }
                      if (!deletable) {
                          if (index.column()==(int)DataItemColumn::Key && QString::compare(item->data((int)DataItemColumn::Key).toString(), "[value]", Qt::CaseInsensitive)==0)
                              return  QVariant::fromValue(Theme::color(Theme::Normal_Red));
                      }
                  }
                  return  QVariant::fromValue(Theme::color(Theme::Syntax_keyword));
        } else if (state==(int)DataCheckState::KeyItem) {
                  if (invalid) {
                      return  QVariant::fromValue(Theme::color(Theme::Normal_Red));
                  }
                  return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        } else if (index.column()==(int)DataItemColumn::Value) {
                   bool deletable = item->data((int)DataItemColumn::Delete).toBool();
                   if (!deletable) {
                       QString str = item->data(Qt::DisplayRole).toString().trimmed();
                       if (!deletable && QString::compare(item->data((int)DataItemColumn::Value).toString(),"[value]",Qt::CaseInsensitive)==0)
                           return  QVariant::fromValue(Theme::color(Theme::Normal_Red));
                   }
                   return  QVariant::fromValue(Theme::color(Theme::Syntax_keyword));
        } else {
            return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        }
    }
    case Qt::BackgroundRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        ConnectDataItem *parentItem = item->parentItem();
        if (parentItem == mRootItem ) {
            return QVariant::fromValue(QApplication::palette().color(QPalette::Window));
        } else {
            if (item->data( (int)DataItemColumn::CheckState ).toInt() <= (int)DataCheckState::ListItem   ||
                item->data( (int)DataItemColumn::CheckState ).toInt() == (int)DataCheckState::ListAppend ||
                item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::MapAppend    ||
                item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::MapSchemaAppend  )
                return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Window));
            else
                return QVariant::fromValue(QApplication::palette().color(QPalette::Base));
        }
    }
    case Qt::UserRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        return QVariant(item->id());
    }
    case Qt::ToolTipRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
        if (item->data(index.column()).toBool()) {
            ConnectDataItem *parentItem = item->parentItem();
            QModelIndex data_index = index.sibling(index.row(),(int)DataItemColumn::Key );
            if (parentItem != mRootItem) {
                if (index.column()==(int)DataItemColumn::Delete) {
                    return QVariant( QString("%1 Delete %2%3%4 and all children, if there is any%5")
                                     .arg( TooltipStrHeader,TooltipOpenedBoldStr,data_index.data(Qt::DisplayRole).toString(),TooltipClosedBoldStr,TooltipStrFooter ) );
                } else if (index.column()==(int)DataItemColumn::MoveDown) {
                              return QVariant( QString("%1 Move %2%3%4 and all its children down %5")
                                               .arg( TooltipStrHeader,TooltipOpenedBoldStr,data_index.data(Qt::DisplayRole).toString(),TooltipClosedBoldStr,TooltipStrFooter ) );
                } else if (index.column()==(int)DataItemColumn::MoveUp) {
                              return QVariant( QString("%1 Move %2%3%4 and all its children up %5")
                                               .arg( TooltipStrHeader,TooltipOpenedBoldStr,data_index.data(Qt::DisplayRole).toString(),TooltipClosedBoldStr,TooltipStrFooter ) );
                } else  if (index.column()==(int)DataItemColumn::Key) {
                           QVariant data = index.data(index.column());
                           if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::SchemaName) {
                               if (!item->data( (int)DataItemColumn::Undefined ).toBool() ) {
                                   if (item->data( (int)DataItemColumn::InvalidValue).toInt()>0)
                                       return QVariant( QString("%1%2%3%4 may contain an invalid or unknown attribute or miss a required attribute.<br/>Check schema definition for valid attributes.<br/>Note that name is case-sensitive.%5")
                                                             .arg( TooltipStrHeader,TooltipOpenedBoldStr,data_index.data(Qt::DisplayRole).toString(),TooltipClosedBoldStr,TooltipStrFooter ));
                                   else
                                         return QVariant( QString("%1 Show help for %2%3%4 schema %5")
                                                         .arg( TooltipStrHeader,TooltipOpenedBoldStr,data_index.data(Qt::DisplayRole).toString(),TooltipClosedBoldStr,TooltipStrFooter) );
                               } else {
                                   return QVariant( QString("%1 Unknown schema name %2%3%4.<br/>Note that name is case-sensitive.<br/>See %2%5%4 location for known schema definition.%6")
                                                    .arg( TooltipStrHeader,TooltipOpenedBoldStr,data.toString(),TooltipClosedBoldStr, CommonPaths::gamsConnectSchemaDir(),TooltipStrFooter ) );
                               }
                           } else if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ListAppend) {
                                      if (item->parentItem() && item->parentItem()->data((int)DataItemColumn::Undefined ).toBool())
                                          return QVariant( QString("%1 Unable to add element to the list of the unknown %2%3%4<br/>Check schema definition for valid attribute name or name of its parent.<br/>Note that name is case-sensitive %5")
                                                           .arg( TooltipStrHeader,TooltipOpenedBoldStr,data.toString(),TooltipClosedBoldStr,TooltipStrFooter) );
                                     return QVariant( QString("%1 Add element to the list of %2%3%4 %5")
                                                      .arg( TooltipStrHeader,TooltipOpenedBoldStr,data.toString(),TooltipClosedBoldStr,TooltipStrFooter ) );
                           } else if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::MapAppend) {
                               return QVariant( QString("%1 Add element to %2%3%4 dict %5")
                                                   .arg( TooltipStrHeader,TooltipOpenedBoldStr,data.toString(),TooltipClosedBoldStr,TooltipStrFooter ) );
                           } else if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::MapSchemaAppend) {
                                      return QVariant( QString("%1 Fill dict schema elements into %2%3%4 (only one time) %5")
                                                       .arg( TooltipStrHeader,TooltipOpenedBoldStr,data.toString(),TooltipClosedBoldStr,TooltipStrFooter ) );
                           } else if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::SchemaAppend) {
                                      QVariant parentdata = index.parent().data(index.column());
                                      return QVariant( QString("%1 Select which %2oneof%4 to add element to the list %2%3%4 %5")
                                                       .arg( TooltipStrHeader,TooltipOpenedBoldStr,parentdata.toString(),TooltipClosedBoldStr,TooltipStrFooter ) );
                           } else if (item->data( (int)DataItemColumn::Undefined).toBool()) {
                                     return QVariant( QString("%1 %2%3%4 attribute is unknown.<br/>Check schema definition for valid attribute name or name of its parent.<br/>Note that name is case-sensitive.%5")
                                                           .arg( TooltipStrHeader,TooltipOpenedBoldStr,data.toString(),TooltipClosedBoldStr,TooltipStrFooter ));
                           } else if (item->data((int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::KeyItem || item->data((int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ElementValue) {
                                     QStringList schemakey = item->data((int)DataItemColumn::SchemaKey).toStringList();
                                     if (!schemakey.isEmpty()) {
                                        ConnectSchema* schema = mConnect->getSchema(schemakey.first());
                                        if (schema) {
                                            schemakey.removeFirst();
                                            QString schemastr = schemakey.join(":");
                                            QString keystr = schemakey.last().left(schemakey.last().lastIndexOf("["));
                                            QString key = schemastr.left(schemastr.lastIndexOf("["));
                                            bool oneof = (schema->isOneOfDefined(key));
                                            if (oneof) {
                                                int number = schema->getNumberOfOneOfDefined(key);
                                                QVariant type = index.siblingAtColumn((int)DataItemColumn::SchemaType).data(Qt::DisplayRole);
                                                QVariant allowedValue = index.siblingAtColumn((int)DataItemColumn::AllowedValue).data(Qt::DisplayRole);
                                                return QVariant( QString("%1%2%3%4 is oneof %2%5%4 schema with type %2%6.%4 %7 %8")
                                                                    .arg( TooltipStrHeader,TooltipOpenedBoldStr,schemakey.last(),TooltipClosedBoldStr,QString::number(number),type.toStringList().join(","),
                                                                          (allowedValue.toString().isEmpty()?"" : "<br/>Allowed values are "+TooltipOpenedBoldStr+allowedValue.toString()+TooltipClosedBoldStr+"."),
                                                                          TooltipStrFooter));

                                            }
                                            if (item->data( (int)DataItemColumn::InvalidValue).toInt()>0) {
                                                    return QVariant( QString("%1%2%3%4 may be invalid attribute or be excluded from (an)other attribute or contains an invalid value.<br/>Check schema definition for valid and excluded attribute.%5")
                                                                        .arg( TooltipStrHeader,TooltipOpenedBoldStr,data_index.data(Qt::DisplayRole).toString(),TooltipClosedBoldStr,TooltipStrFooter ));
                                            }
                                        }
                                     }
                           } else if (item->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementMap) {
                                      QString str = item->data((int)DataItemColumn::Key).toString();
                                      if (QString::compare(str,"[key]",Qt::CaseInsensitive)==0)
                                          return  QVariant( QString("%1 The key %2%3%4 is created for the dict %2%5%4. <br/>Edit this to change to a wanted or desired dict key. %6")
                                                                .arg(TooltipStrHeader, TooltipOpenedBoldStr, "[key]", TooltipClosedBoldStr, item->parentItem()->data((int)DataItemColumn::Key).toString(), TooltipStrFooter) );

                           }
                } else if (index.column()==(int)DataItemColumn::Value) {
                          bool deletable = item->data((int)DataItemColumn::Delete).toBool();
                          QString str = item->data((int)DataItemColumn::Value).toString();
                          if (!deletable) {
                              if (item->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementValue) {
                                  if (QString::compare(str,"[value]",Qt::CaseInsensitive)==0)
                                      return  QVariant( QString("%1 %2%3%4 is created for the required attribute %2%5%4. <br/>Edit this value to change to a wanted or desired value. %6")
                                                      .arg(TooltipStrHeader, TooltipOpenedBoldStr, "[value]", TooltipClosedBoldStr, item->data((int)DataItemColumn::Key).toString(), TooltipStrFooter) );
                              }
                          }
                          if (item->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementMap) {
                              if (QString::compare(str,"[value]",Qt::CaseInsensitive)==0)
                                  return  QVariant( QString("%1 The value %2%3%4 is created for the dict %2%5%4. <br/>Edit this value to change to a wanted or desired dict value. %6")
                                                  .arg(TooltipStrHeader, TooltipOpenedBoldStr, "[value]", TooltipClosedBoldStr, item->parentItem()->data((int)DataItemColumn::Key).toString(), TooltipStrFooter) );

                          }
                }
            }
        }
        if (item->data( (int)DataItemColumn::InvalidValue).toInt()>0 && !item->data( (int)DataItemColumn::Undefined ).toBool()) {
            QVariant key  = index.siblingAtColumn((int)DataItemColumn::Key).data(Qt::DisplayRole);
            QVariant type = index.siblingAtColumn((int)DataItemColumn::SchemaType).data(Qt::DisplayRole);
            QVariant allowedValue = index.siblingAtColumn((int)DataItemColumn::AllowedValue).data(Qt::DisplayRole);

            QStringList schema = item->data((int)DataItemColumn::SchemaKey).toStringList();
            if (!schema.isEmpty()) {
                 if ( (index.column()==(int)DataItemColumn::Key && item->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementKey) ||
                    (index.column()==(int)DataItemColumn::Value && item->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementValue)  ) {
                     QString schemaname = schema.first();
                     schema.removeFirst();
                     ValueWrapper minvalue = mConnect->getSchema(schemaname)->getMin(schema.join(":"));
                     ValueWrapper maxvalue = mConnect->getSchema(schemaname)->getMax(schema.join(":"));
                     return QVariant( QString("%1 Invalid value.<br/>%2%3%4 must be of type %2%5%4.%6 %7 %8 %9")
                                      .arg( TooltipStrHeader, TooltipOpenedBoldStr, key.toString(), TooltipClosedBoldStr, type.toString(),
                                            (allowedValue.toString().isEmpty()?"" : "<br/>Allowed values are "+TooltipOpenedBoldStr+allowedValue.toString()+TooltipClosedBoldStr+"."),
                                            (minvalue.type!=SchemaValueType::Integer ? "" : "<br/>Min value is "+TooltipOpenedBoldStr+QString::number(minvalue.value.intval)+TooltipClosedBoldStr+"." ),
                                            (maxvalue.type!=SchemaValueType::Integer ? "" : "<br/>Max value is "+TooltipOpenedBoldStr+QString::number(maxvalue.value.intval)+TooltipClosedBoldStr+"." ),
                                            TooltipStrFooter ) );
                 }
            }
        }
        break;
    }
    case Qt::DecorationRole: {
        ConnectDataItem* item = static_cast<ConnectDataItem*>(index.internalPointer());
       if (index.column()==(int)DataItemColumn::Key) {
            QModelIndex checkstate_index = index.sibling(index.row(),(int)DataItemColumn::CheckState );
            QModelIndex unknown_index = index.sibling(index.row(),(int)DataItemColumn::Undefined );
            int checkstate = checkstate_index.data(Qt::DisplayRole).toInt();
            if (checkstate==(int)DataCheckState::SchemaName) {
               if (unknown_index.data(Qt::DisplayRole).toBool())
                  return QVariant::fromValue(Theme::icon(":/solid/kill"));
               else
                  return QVariant::fromValue(Theme::icon(":/solid/question"));
            } else if (checkstate==(int)DataCheckState::ListAppend ||
                       checkstate==(int)DataCheckState::SchemaAppend ||
                       checkstate==(int)DataCheckState::MapAppend    ||
                       checkstate==(int)DataCheckState::MapSchemaAppend  ) {
               return QVariant::fromValue(Theme::icon(":/solid/plus"));
            }
       } else if (index.column()==(int)DataItemColumn::Delete) {
                 if (item->data(index.column()).toBool())
                     return QVariant::fromValue(Theme::icon(":/solid/delete-all"));
       } else if (index.column()==(int)DataItemColumn::MoveDown) {
                  if (item->data(index.column()).toBool())
                      return QVariant::fromValue(Theme::icon(":/solid/move-down"));
       } else if (index.column()==(int)DataItemColumn::MoveUp) {
                  if (item->data(index.column()).toBool())
                      return QVariant::fromValue(Theme::icon(":/solid/move-up"));
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
               if (item->data((int)DataItemColumn::CheckState).toInt()== (int)DataCheckState::ElementMap ||
                   item->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementKey    )
                   return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
               else if (item->data( (int)DataItemColumn::CheckState ).toInt()>=(int)DataCheckState::SchemaAppend)
                       return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
               else if (item->data( (int)DataItemColumn::CheckState ).toInt()>=(int)DataCheckState::MapSchemaAppend)
                   return   Qt::NoItemFlags;
               else
                   return Qt::ItemIsDropEnabled | Qt::NoItemFlags;
    } else if (index.column()==(int)DataItemColumn::Value) {
              if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ElementKey)
                  return Qt::NoItemFlags;
              else if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ElementMap)
                       return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
              else if (item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ElementValue)
                      return Qt::ItemIsEditable | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
              else if (item->data( (int)DataItemColumn::CheckState ).toInt()<=(int)DataCheckState::ElementKey ||
                       item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::ListAppend ||
                       item->data( (int)DataItemColumn::CheckState ).toInt()==(int)DataCheckState::MapSchemaAppend )
                     return   Qt::NoItemFlags;
              else
                    return QAbstractItemModel::flags(index);
    } else if (index.column()>(int)DataItemColumn::CheckState) {
               return Qt::NoItemFlags;
    } else {
        return QAbstractItemModel::flags(index);
    }
}

bool ConnectDataModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    QVector<int> roles;
    switch (role) {
    case Qt::EditRole: {
        roles = { Qt::EditRole };
        ConnectDataItem *item = getItem(idx);
        int preValidValue = item->data((int)DataItemColumn::InvalidValue).toInt();
        bool result = item->setData(idx.column(), value);
        if (result) {
            editDataChanged(idx, preValidValue<=0);
        }
        return result;
    }
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

    ConnectDataItem *parentItem = getItem(parent);
    ConnectDataItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ConnectDataModel::indexForTreeItem(ConnectDataItem *item)
{
    return createIndex(item->childNumber(), 0, item);
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

    informDataChanged( parent );
}

bool ConnectDataModel::removeItem(const QModelIndex &index)
{
    ConnectDataItem* treeItem = getItem(index);
    bool removed = false;
    if (treeItem) {
        if (!treeItem->isLastChild()) {
            if ((int)DataCheckState::KeyItem==treeItem->data((int)DataItemColumn::CheckState).toInt()) {
                QModelIndex sibling = index.sibling(index.row()+1, 0);
                int state = sibling.data((int)DataItemColumn::CheckState).toInt();
                if ((int)DataCheckState::ListAppend==state || (int)DataCheckState::MapAppend==state || (int)DataCheckState::MapSchemaAppend==state) {
                    removeRows(index.row()+1, 1, parent(sibling));
                }
            }
        }
        QStringList schemaKeyList = treeItem->data((int)DataItemColumn::SchemaKey).toStringList();
        if (!schemaKeyList.isEmpty()) {
              if (treeItem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ListItem) {
                  ConnectDataItem* parentitem = treeItem->parentItem();
                  while(parentitem && parentitem!=mRootItem) {
                      int itemvalue = treeItem->data((int)DataItemColumn::InvalidValue).toInt();
                      int parentvalue = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
                      if (parentvalue >= itemvalue) {
                          parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue <=0 ? parentvalue : parentvalue-itemvalue));
                      }
                      parentitem = parentitem->parentItem();
                  }
              } else {
//                  bool removeValidItem = isIndexValueValid(index.column(), treeItem);
                  if (! treeItem->data((int)DataItemColumn::ExcludedKeys).toStringList().isEmpty()) {  // there is excluded
                      int sibling = numberOfExcludedSibling(treeItem);
                      if (sibling > 0) { // there is an excluded sibling
                          bool found = false;
                          ConnectDataItem* parent = treeItem->parentItem();
                          for(int i=0; i<parent->childCount(); i++) {
                              ConnectDataItem* c = parent->child(i);
                              if (QString::compare(treeItem->data((int)DataItemColumn::Key).toString(), c->data((int)DataItemColumn::Key).toString())==0) {
                                   continue;
                              }
                              if (found) {
                                   break;
                              }
                              for(const QString& k : treeItem->data((int)DataItemColumn::ExcludedKeys).toString().split(",")) {
                                  if (found)
                                      break;
                                  if (QString::compare(k, c->data((int)DataItemColumn::Key).toString())==0) {
                                      found = true;
                                      if (sibling == 1) {
                                          int value = c->data((int)DataItemColumn::InvalidValue).toInt();
                                          if (value > 0)
                                              c->setData((int)DataItemColumn::InvalidValue, QVariant(value-1));
                                      }
                                  }
                              }
                          }
                          if (found) {
                              ConnectDataItem* schemaitem = getSchemaParentItem(treeItem);
                              ConnectDataItem* parentitem = treeItem->parentItem();
                              int itemvalue = treeItem->data((int)DataItemColumn::InvalidValue).toInt();
                              while(parentitem && parentitem!=schemaitem) {
                                  int parentvalue = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
                                  if (sibling == 1) {
                                      if (parentvalue > 0)
                                          parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue <=0 ? parentvalue : parentvalue-2));
                                  } else { // sibling > 1
                                      parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue <= itemvalue ? parentvalue :  parentvalue-itemvalue));
                                  }
                                  parentitem = parentitem->parentItem();
                              }
                              if (schemaitem!=treeItem &&
                                  treeItem->data((int)DataItemColumn::Delete).toBool() &&
                                  !treeItem->data((int)DataItemColumn::ExcludedKeys).toStringList().isEmpty()) {
                                      int value = schemaitem->data((int)DataItemColumn::InvalidValue).toInt();
                                      if (sibling == 1) {
                                          if (value > itemvalue) {
                                              schemaitem->setData((int)DataItemColumn::InvalidValue, QVariant(value-itemvalue-1));
                                          }
                                      } else {
                                          if (value >= itemvalue) {
                                              schemaitem->setData((int)DataItemColumn::InvalidValue, QVariant(value-itemvalue));
                                          }
                                      }
                              }
                          }
                      } else { // there is no excluded sibling
//                          if (removeValidItem) {
                              ConnectDataItem* parentitem = treeItem->parentItem();
                              while(parentitem && parentitem!=mRootItem) {
                                  int itemvalue = treeItem->data((int)DataItemColumn::InvalidValue).toInt();
                                  int parentvalue = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
                                  if (parentvalue > 0)
                                      parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue-itemvalue)); //itemvalue > 0 ? parentvalue-itemvalue : parentvalue-1));
                                  parentitem = parentitem->parentItem();
                              }
                              ConnectDataItem* schemaitem = getSchemaParentItem(treeItem);
                              if (treeItem->data((int)DataItemColumn::Delete).toBool()) { // delete what is required
                                  int value = schemaitem->data((int)DataItemColumn::InvalidValue).toInt();
                                  schemaitem->setData((int)DataItemColumn::InvalidValue, QVariant(value+1));
                              }
//                          }
                      }
                  } else {
//                      if (!removeValidItem) {
                          ConnectDataItem* parentitem = treeItem->parentItem();
                          while(parentitem && parentitem!=mRootItem) {
                              int itemvalue = treeItem->data((int)DataItemColumn::InvalidValue).toInt();
                              int parentvalue = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
                              if (parentvalue >= itemvalue)
                                  parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue-itemvalue)); // itemvalue > 0 ? parentvalue-itemvalue : parentvalue-1));
                              parentitem = parentitem->parentItem();
                          }
                          ConnectDataItem* schemaitem = getSchemaParentItem(treeItem);

                          if (schemaitem!=treeItem &&
                              treeItem->data((int)DataItemColumn::Delete).toBool() &&
                              !treeItem->data((int)DataItemColumn::ExcludedKeys).toStringList().isEmpty()) { // delete what is required
                              int value = schemaitem->data((int)DataItemColumn::InvalidValue).toInt();
                              schemaitem->setData((int)DataItemColumn::InvalidValue, QVariant(value+1));
                          }
//                      }
                  }
              }
        }
        removed = removeRows(treeItem->row(), 1, parent(index));
    }
    return removed;
}

bool ConnectDataModel::insertRows(int row, int count, const QModelIndex &parent)
{
    ConnectDataItem* parentItem = getItem(parent);
    bool success = false;
    if (count > 0) {
        beginInsertRows(parent, row, row + count - 1);
        for (int i=0; i<count; ++i) {
            parentItem->insertChild(row+i, mRootItem);
        }
        endInsertRows();
        informDataChanged(parent);
        success = true;
    }
    return success;
}

bool ConnectDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    ConnectDataItem* parentItem = getItem(parent);
    bool success = false;
    if (count > 0) {
        beginRemoveRows(parent, row, row + count - 1);
        success = parentItem->removeChildren(row, count);
        endRemoveRows();
        informDataChanged(parent);
//        success = true;
    }
    return success;
}

bool ConnectDataModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    ConnectDataItem* destParentItem = getItem(destinationParent);
    ConnectDataItem* sourceParentItem = getItem(sourceParent);
    if (destParentItem != sourceParentItem)
        return false;
    if (destinationChild > sourceRow) { // move down
         beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild);
         for(int i=0; i<count; ++i) {
             sourceParentItem->moveChildren(sourceRow+i, destinationChild-1);
         }
         endMoveRows();
    } else { // move up
        beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild);
        for(int i=0; i<count; ++i) {
            sourceParentItem->moveChildren(sourceRow+i, destinationChild);
        }
        endMoveRows();
    }
    informDataChanged(destinationParent);

    return true;
}

QStringList ConnectDataModel::mimeTypes() const
{
    QStringList types;
    types <<  "application/vnd.gams-connect.text";
    return types;
}

Qt::DropActions ConnectDataModel::supportedDropActions() const
{
    return Qt::CopyAction ;
}

bool ConnectDataModel::canDropMimeData(const QMimeData *mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(row)
    Q_UNUSED(parent)
    if (action != Qt::CopyAction)
        return false;

    if (!mimedata->hasFormat("application/vnd.gams-connect.text"))
        return false;

    QStringList newItems;
    QByteArray encodedData = mimedata->data("application/vnd.gams-connect.text");

    QDataStream stream(&encodedData, QFile::ReadOnly);
//    int rows = stream.atEnd()?-1:0;
    while (!stream.atEnd()) {
       QString text;
       stream >> text;
       newItems << text;
//       ++rows;
    }

    QStringList schemastrlist = newItems[0].split("=");
    if (schemastrlist.size() <= 1)
        return false;

    if (!parent.isValid())
        return false;

    QStringList tobeinsertSchemaKey = schemastrlist[1].split(":");
    QModelIndex schemaIndex = (row < 0 || column < 0 ? parent.siblingAtColumn((int)DataItemColumn::SchemaKey)
                                                     : (row==rowCount(parent)) ? index(rowCount(parent)-1, (int)DataItemColumn::SchemaKey, parent)
                                                                               : index(row, (int)DataItemColumn::SchemaKey, parent)
                               );
    QStringList schemaKey =  schemaIndex.data(Qt::DisplayRole).toString().split(":");
    int state = schemaIndex.siblingAtColumn((int)DataItemColumn::CheckState).data(Qt::DisplayRole).toInt();
    if (mOnlyRequriedAttributesAdded && !isRequired(tobeinsertSchemaKey, schemaKey))
        return false;
    if (row < 0 || column < 0) { // drop on to a row or end of the list
        if (schemaKey.size()>tobeinsertSchemaKey.size()) // same size
            return false;
        if (state==(int)DataCheckState::SchemaName) {
           if (schemaKey.size()>tobeinsertSchemaKey.size()) // not immediate attribute of schemaname
               return false;
           if (tobeinsertSchemaKey.size()-1 > schemaKey.size()) // not immediate level
               return false;
           if (!hasSameParent(tobeinsertSchemaKey, schemaKey, schemaKey.size()==tobeinsertSchemaKey.size())) // not immediate attribute of schemaname
              return false;
           if (existsUnderSameParent(schemastrlist[1],  parent)) // attribute already exists
              return false;
        } else {
            if (schemaKey.size()==1 && schemaKey.first().isEmpty() &&
               tobeinsertSchemaKey.size()==1 && !tobeinsertSchemaKey.isEmpty()) // special case; drop
                return true;
            if (mOnlyRequriedAttributesAdded && !isRequired(tobeinsertSchemaKey, schemaKey))
                return false;
            if (state!=(int)DataCheckState::ListItem && state!=(int)DataCheckState::KeyItem) // not a ListItem
                return false;
            if (schemaKey.size()==tobeinsertSchemaKey.size()) // same size
                return false;
            if (tobeinsertSchemaKey.size()!=schemaKey.size()+1) // not immediate attribute to be inserted
                return false;
            if (!hasSameParent(tobeinsertSchemaKey, schemaKey, schemaKey.size()==tobeinsertSchemaKey.size())) // not immediate attribute of schemaname
                return false;
            if (schemaKey.last().compare("-")==0) {
                if (mRexNumber.match(tobeinsertSchemaKey.last()).hasMatch())
                    return true;
            }
            if (existsUnderSameParent(schemastrlist[1],  parent)) // exists under the same parent
                return false;
        }
    } else { // drop between row
        if (schemaKey.size() > 0 && schemaKey.at(0).isEmpty())
            return false;
        if (!hasSameParent(tobeinsertSchemaKey, schemaKey,true)) // does not have the same parent
            return false;
        if (tobeinsertSchemaKey.size()>1) {  // when insert attribute
           // check if tobeinsertSchemaKey exists under the same parent
           if (tobeinsertSchemaKey.size()!=schemaKey.size() ||
               existsUnderSameParent(schemastrlist[1],  parent)) {
               return false;
           }
        }
    }

    if (schemaKey.at(0).isEmpty())
       return false;

    return true;
}

bool ConnectDataModel::dropMimeData(const QMimeData *mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    QByteArray encodedData = mimedata->data("application/vnd.gams-connect.text");
    QDataStream stream(&encodedData, QFile::ReadOnly);
    QStringList newItems;
//    int rows = stream.atEnd()?-1:0;
    while (!stream.atEnd()) {
       QString text;
       stream >> text;
       newItems << text;
//       ++rows;
    }
    QStringList schemastrlist = newItems[0].split("=");
    if (!parent.isValid()) {
        return false;
    } else {
        QStringList tobeinsertSchemaKey = schemastrlist[1].split(":");
        QModelIndex schemaIndex = (row < 0 || column < 0 ? parent.siblingAtColumn((int)DataItemColumn::SchemaKey)
                                                         : (row==rowCount(parent)) ? index(rowCount(parent)-1, (int)DataItemColumn::SchemaKey, parent)
                                                                                   : index(row, (int)DataItemColumn::SchemaKey, parent)
                                   );

        QString schemaname = tobeinsertSchemaKey.first();
        if (tobeinsertSchemaKey.size() == 1) {
            addFromSchema(tobeinsertSchemaKey.first(), row < 0 ? (parent==index(0,0, QModelIndex()) ? rowCount(parent) : parent.row()+1)
                                                               : row);
        } else {
            ConnectData* data = mConnect->createDataHolderFromSchema(tobeinsertSchemaKey, mOnlyRequriedAttributesAdded);
            if (!tobeinsertSchemaKey.isEmpty())
                tobeinsertSchemaKey.removeFirst();
            if (data->getRootNode().Type()==YAML::NodeType::Map) {
                if (!tobeinsertSchemaKey.isEmpty())
                    tobeinsertSchemaKey.removeLast();
                appendMapElement(schemaname, tobeinsertSchemaKey, data,  row, parent);
            } else if (data->getRootNode().Type()==YAML::NodeType::Sequence) {
                     insertLastListElement(schemaname, tobeinsertSchemaKey, data, schemaIndex);
            }
        }
        emit modificationChanged(true);
        return true;
    }

    return false;
}

void ConnectDataModel::addFromSchema(const QString& schemaname, int position)
{
    QStringList strlist;
    strlist << schemaname;
    ConnectData* data = mConnect->createDataHolder( strlist, mOnlyRequriedAttributesAdded );

    Q_ASSERT(data->getRootNode().Type() ==YAML::NodeType::Sequence);

    YAML::Node node = data->getRootNode();
    Q_ASSERT(node.Type()==YAML::NodeType::Sequence);

    QList<ConnectDataItem*> parents;
    parents << mRootItem << mRootItem->child(0);

    beginInsertRows(indexForTreeItem(parents.last()), position, position);
    for(size_t i = 0; i<node.size(); i++) {
        for (YAML::const_iterator it = node[i].begin(); it != node[i].end(); ++it) {
            QString schemaName = QString::fromStdString(it->first.as<std::string>());
            QStringList dataKeys;
            QList<QVariant> listData;
            listData << schemaName;
            listData << "";
            listData << QVariant((int)DataCheckState::SchemaName);
            listData << QVariant(QString());
            listData << QVariant(QString());
            listData << QVariant(true);
            listData << "";
            listData << "";
            listData << "";
            listData << QVariant(QStringList(schemaName));
            listData << (mConnect->getSchema(schemaName) ? QVariant(false):QVariant(true));
            listData << QVariant(0);
            listData << QVariant();
            listData << QVariant();
            if (position>=parents.last()->childCount() || position < 0) {
                parents.last()->appendChild(new ConnectDataItem(listData, mItemIDCount++, parents.last()));
                parents << parents.last()->child(parents.last()->childCount()-1);
            } else {
                parents.last()->insertChild(position, new ConnectDataItem(listData, mItemIDCount++, parents.last()));
                parents << parents.last()->child(position);
            }

            insertSchemaData(schemaName, dataKeys, new ConnectData(it->second), -1,  parents);

            parents.pop_back();
        }
        position++;
    }

    endInsertRows();
    if (data) {
        delete data;
        data = NULL;
    }
    informDataChanged( index(0,0).parent() );
    QModelIndex parent = indexForTreeItem( mRootItem->child(0) );
    if (position <= 0)
        emit indexExpandedAndResized(index(getItem(parent)->childCount()-1, (int)DataItemColumn::Key, parent));
    else
        emit indexExpandedAndResized(index(position-1, (int)DataItemColumn::Key, parent));

}

void ConnectDataModel::appendMapElement(const QModelIndex &index)
{
    QStringList schemakeys = index.sibling(index.row(), (int)DataItemColumn::SchemaKey).data(Qt::DisplayRole).toString().split(":");
    QList<QVariant> mapSeqData;
    mapSeqData << "[key]";
    mapSeqData << "[value]";
    mapSeqData << QVariant((int)DataCheckState::ElementMap);
    mapSeqData << QVariant("dict");
    mapSeqData << QVariant();
    mapSeqData << QVariant(true);
    mapSeqData << QVariant();
    mapSeqData << QVariant();
    mapSeqData << QVariant();
    mapSeqData << QVariant(QStringList());
    mapSeqData << QVariant(0);
    mapSeqData << QVariant();
    mapSeqData << QVariant();
    ConnectDataItem *item = new ConnectDataItem( mapSeqData, mItemIDCount++, getItem(index.parent()) );
    insertItem(index.row(), item, index.parent());
}

void ConnectDataModel::appendMapElement(const QString& schemaname, QStringList& keys, ConnectData* data, int position, const QModelIndex& parentIndex)
{
    if (data->getRootNode().IsNull()) {
        delete data;
        data = NULL;
        return;
    }
    ConnectDataItem* parentItem = getItem(parentIndex);

    QList<ConnectDataItem*> parents;
    parents << parentItem;

    QStringList schemaKeys;
    schemaKeys << schemaname << keys;

    YAML::Node node = data->getRootNode();
    beginInsertRows(parentIndex, position, position);
    insertSchemaData(schemaname, keys, data, position, parents);
    endInsertRows();
    informDataChanged(parentIndex);
    if (position < 0)
        emit indexExpandedAndResized(index(getItem(parentIndex)->childCount()-1, (int)DataItemColumn::Key, parentIndex));
    else
        emit indexExpandedAndResized(index(position, (int)DataItemColumn::Key, parentIndex));
}

void ConnectDataModel::appendMapSchemaElement(const QString &schemaname, QStringList &keys, ConnectData *data, const QModelIndex &parentIndex)
{
    if (data->getRootNode().IsNull()) {
        delete data;
        data = NULL;
        return;
    }
    ConnectDataItem* parentItem = getItem(parentIndex);

    QList<ConnectDataItem*> parents;
    parents << parentItem;

    QStringList schemaKeys;
    schemaKeys << schemaname << keys;

    YAML::Node node = data->getRootNode();
    int i = 0;
    if (node.Type()==YAML::NodeType::Map) {
        int position = 0;
        for (YAML::const_iterator mit = node.begin(); mit !=node.end(); ++mit) {
            YAML::Node subnode;
            subnode[ mit->first ] = mit->second;
            beginInsertRows(parentIndex, position, position);
            insertSchemaData(schemaname, keys, new ConnectData(subnode), position+i, parents);
            endInsertRows();
            ++position;
        }
    }
    informDataChanged(parentIndex);
    emit indexExpandedAndResized(index(getItem(parentIndex)->childCount()-1, (int)DataItemColumn::Key, parentIndex));

}

void ConnectDataModel::appendListElement(const QString& schemaname,  QStringList& keys, ConnectData* data, const QModelIndex &idx)
{
    QModelIndex parentIndex = idx.parent();
    ConnectDataItem* parentItem = getItem(parentIndex);

    QList<ConnectDataItem*> parents;
    parents << parentItem;

    QStringList schemaKeys;
    schemaKeys << schemaname << keys;
    beginInsertRows(parentIndex, idx.row(), idx.row());

    ConnectSchema* schema = mConnect->getSchema(schemaname);
    YAML::Node node = data->getRootNode();
    for(size_t i = 0; i<node.size(); i++) {
        QList<QVariant> mapSeqData;
//        QString listkey = schemaKeys.last();
        mapSeqData << QVariant::fromValue(idx.row()+i);
        mapSeqData << "";
        mapSeqData << QVariant((int)DataCheckState::ListItem);
        mapSeqData << QVariant(QString());
        mapSeqData << QVariant(QString());
        mapSeqData << QVariant(true);
        mapSeqData << QVariant();
        mapSeqData << QVariant();
        mapSeqData << QVariant();
        schemaKeys << "-";
        keys       << "-";
        if (node[i].Type()==YAML::NodeType::Map) {
            if (schema && schema->getNumberOfOneOfDefined(keys.join(":"))) {
                int n = whichOneOfSchema(data->getRootNode(), schema, keys, keys.first());
                QString oneofstrindex = "[" + QString::number(n) + "]";
                keys   << oneofstrindex;
                schemaKeys << oneofstrindex;
            }
            mapSeqData << QVariant(schemaKeys);
            mapSeqData << (schema ?  QVariant(false) : QVariant(true));
            mapSeqData << QVariant(0);
            mapSeqData << QVariant();
            mapSeqData << QVariant();
            ConnectDataItem* listitem = new ConnectDataItem( mapSeqData, mItemIDCount++, getItem(idx.parent()));
            parents.last()->insertChild(idx.row(), listitem );
            parents << listitem;

            insertSchemaData(schemaname, keys, new ConnectData(node[i]), -1, parents);
        } else if (node[i].Type()==YAML::NodeType::Scalar) {
            mapSeqData << QVariant(schemaKeys);
            mapSeqData << (schema ?  QVariant(false) : QVariant(true));
            mapSeqData << QVariant(0);
            mapSeqData << QVariant();
            mapSeqData << QVariant();
            ConnectDataItem* listitem = new ConnectDataItem( mapSeqData, mItemIDCount++, getItem(idx.parent()));
            parents.last()->insertChild(idx.row(), listitem );
            parents << listitem;

            QString key = QString::fromStdString(node[i].as<std::string>());
            QList<QVariant> itemData;
            itemData << key;
            itemData << "";
            itemData << QVariant((int)DataCheckState::ElementKey);
            QStringList dataKeysforTypes(keys);
            if (QString::compare(dataKeysforTypes.last(), "-")!=0)
                dataKeysforTypes.removeLast();
            itemData << (schema ? QVariant(schema->getTypeAsStringList(dataKeysforTypes.join(":")).join(",")) : QVariant());
            itemData << (schema ? QVariant(schema->getAllowedValueAsStringList(dataKeysforTypes.join(":")).join(",")) : QVariant());
            itemData << QVariant(false);
            itemData << QVariant(false);
            itemData << QVariant(false);
            itemData << QVariant();
            itemData << (schema ? QVariant(schemaKeys) : QVariant());
            itemData << (schema ? (schema->contains(dataKeysforTypes.join(":")) ? QVariant(false) : QVariant(true))
                                                                                : QVariant(true));
            itemData << QVariant(0);
            QStringList excluded = schema->getExcludedKeys(dataKeysforTypes.join(":"));
            itemData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                : QVariant());
            itemData << (schema ? (schema->getDefaultValue(dataKeysforTypes.join(":")))
                                : QVariant());
            ConnectDataItem* item = new ConnectDataItem(itemData, mItemIDCount++, parents.last());
            updateInvaldItem((int)DataItemColumn::Key, item);
            /*
            updateInvaldItem((int)DataItemColumn::Value, item);
            if (!isIndexValueValid((int)DataItemColumn::Key, item)) {
                item->setData((int)DataItemColumn::InvalidValue, QVariant(1));
                ConnectDataItem* parentitem = getSchemaParentItem(item);
                if (parentitem!=item) {
                    int value = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
                    parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(value+1));
                }
            }*/
            parents.last()->appendChild(item);
        }
    }
    ConnectDataItem* item = parents.last();
    updateInvaldItem((int)DataItemColumn::Key, item);

    endInsertRows();

    if (data) {
        delete data;
        data = NULL;
    }
    informDataChanged(parentIndex);
    emit indexExpandedAndResized(index(idx.row(), (int)DataItemColumn::Key, idx.parent()));
}

void ConnectDataModel::insertLastListElement(const QString &schemaname, QStringList &keys, ConnectData *data, const QModelIndex &parentIndex)
{
    ConnectDataItem* parentItem = getItem(parentIndex);

    QList<ConnectDataItem*> parents;
    parents << parentItem;

    QStringList schemaKeys;
    schemaKeys << schemaname << keys;
    QModelIndex parentKeyIndex = parentIndex.siblingAtColumn((int)DataItemColumn::Key);
    QModelIndex idx = index(rowCount(parentKeyIndex)-1, (int)DataItemColumn::Key, parentKeyIndex);

    beginInsertRows(parentKeyIndex, idx.row(), idx.row());

    ConnectSchema* schema = mConnect->getSchema(schemaname);
    YAML::Node node = data->getRootNode();

    for(size_t i = 0; i<node.size(); i++) {
        QList<QVariant> mapSeqData;
//        QString listkey = schemaKeys.last();
        mapSeqData << QVariant::fromValue(idx.row()+i);
        mapSeqData << "";
        mapSeqData << QVariant((int)DataCheckState::ListItem);
        mapSeqData << QVariant(QString());
        mapSeqData << QVariant(QString());
        mapSeqData << QVariant(true);
        mapSeqData << QVariant();
        mapSeqData << QVariant();
        mapSeqData << QVariant();
        if (node[i].Type()==YAML::NodeType::Map) {
            ConnectSchema* schema = mConnect->getSchema(schemaname);
            if (schema && schema->getNumberOfOneOfDefined(keys.join(":"))) {
                int n = whichOneOfSchema(data->getRootNode(), schema, keys, keys.first());
                QString oneofstrindex = "[" + QString::number(n) + "]";
                keys   << oneofstrindex;
                schemaKeys << oneofstrindex;
            }
            mapSeqData << QVariant(schemaKeys);
            mapSeqData << (schema ?  QVariant(false) : QVariant(true));
            mapSeqData << QVariant(0);
            QStringList excluded = schema->getExcludedKeys(keys.join(":"));
            mapSeqData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                  : QVariant());
            mapSeqData << (schema ? (schema->getDefaultValue(keys.join(":")))
                                : QVariant());
            ConnectDataItem* listitem = new ConnectDataItem( mapSeqData, mItemIDCount++, getItem(idx.parent()));
            parents.last()->insertChild(idx.row(), listitem );
            parents << listitem;

            insertSchemaData(schemaname, keys, new ConnectData(node[i]), -1, parents);
        } else if (node[i].Type()==YAML::NodeType::Scalar) {
            mapSeqData << QVariant(schemaKeys);
            mapSeqData << (schema ?  QVariant(false) : QVariant(true));
            mapSeqData << QVariant(0);
            QStringList excluded = schema->getExcludedKeys(keys.join(":"));
            mapSeqData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                  : QVariant());
            mapSeqData << (schema ? (schema->getDefaultValue(keys.join(":")))
                                  : QVariant());
            ConnectDataItem* listitem = new ConnectDataItem( mapSeqData, mItemIDCount++, getItem(idx.parent()));
            parents.last()->insertChild(idx.row(), listitem );
            parents << listitem;

            QString key = QString::fromStdString(node[i].as<std::string>());
            QList<QVariant> itemData;
            itemData << key;
            itemData << "";
            itemData << QVariant((int)DataCheckState::ElementKey);
            QStringList dataKeysforTypes(keys);
            dataKeysforTypes.removeLast();
            itemData << (schema ? QVariant(schema->getTypeAsStringList(dataKeysforTypes.join(":")).join(",")) : QVariant());
            itemData << (schema ? QVariant(schema->getAllowedValueAsStringList(dataKeysforTypes.join(":")).join(",")) : QVariant());
            itemData << QVariant(false);
            itemData << QVariant(false);
            itemData << QVariant();
            itemData << QVariant();
            itemData << (schema ? (schema->contains(dataKeysforTypes.join(":")) ? QVariant(false) : QVariant(true))
                                                                                : QVariant(true));
            itemData << QVariant(0);
            itemData << QVariant();
            itemData << (schema ? (schema->getDefaultValue(dataKeysforTypes.join(":"))) : QVariant());
            ConnectDataItem* item = new ConnectDataItem(itemData, mItemIDCount++, parents.last());
            if (!isIndexValueValid((int)DataItemColumn::Key, item)) {
                item->setData((int)DataItemColumn::InvalidValue, QVariant(1));
                ConnectDataItem* parentitem = getSchemaParentItem(item);
                if (parentitem!=item) {
                    int value = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
                    parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(value+1));
                }
            }
            parents.last()->appendChild(item);
        } /* else if (node[i].Type()==YAML::NodeType::Sequence) {
            mapSeqData << QVariant(schemaKeys);
            mapSeqData << (schema ?  QVariant(false) : QVariant(true));
            mapSeqData << QVariant(0);
            QStringList excluded = schema->getExcludedKeys(keys.join(":"));
            mapSeqData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                  : QVariant());
            mapSeqData << (schema ? (schema->getDefaultValue(keys.join(":")))
                                  : QVariant());
            ConnectDataItem* listitem = new ConnectDataItem( mapSeqData, mItemIDCount++, getItem(idx.parent()));
            parents.last()->insertChild(idx.row(), listitem );
            parents << listitem;
        }*/
    }
    endInsertRows();

    if (data) {
        delete data;
        data = NULL;
    }
    informDataChanged(parentIndex);
    emit indexExpandedAndResized(index(idx.row(), (int)DataItemColumn::Key, idx.parent()));
}

void ConnectDataModel::onlyRequriedAttributedChanged(int state)
{
     mOnlyRequriedAttributesAdded = (state==Qt::Checked);
}

void ConnectDataModel::reloadConnectDataModel()
{
    try {
        ConnectData* data = mConnect->loadDataFromFile(mLocation);
        delete mConnectData;
        mConnectData = data;
    } catch (std::exception &e) {
        EXCEPT() << e.what();
    }

    beginResetModel();
    if (mRootItem)
        delete mRootItem;

    mItemIDCount = 0;

    setupTreeItemModelData();
    endResetModel();
}

void ConnectDataModel:: onEditDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
    QModelIndex idx = topLeft;
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)

    ConnectDataItem* item = getItem(idx);
    if (idx.column() == (int)DataItemColumn::Key)
       editDataChanged(topLeft, isIndexValueValid((int)DataItemColumn::Key, item));
    else
        editDataChanged(topLeft, isIndexValueValid((int)DataItemColumn::Value, item));
}

void ConnectDataModel::editDataChanged(const QModelIndex &index, bool preValidValue)
{
    if (index.column() != (int)DataItemColumn::Key && index.column() != (int)DataItemColumn::Value)
         return;
    ConnectDataItem *item = getItem(index);
    int value = item->data((int)DataItemColumn::InvalidValue).toInt();
    if (isIndexValueValid(index.column(), item)) {
        if (!preValidValue) {
            int reducedvalue = 1;
            item->setData((int)DataItemColumn::InvalidValue, QVariant(value-reducedvalue));
            ConnectDataItem* parentitem = item->parentItem();
            while(parentitem && parentitem != mRootItem) {
                int parentvalue = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
                if (parentvalue >0) {
                    parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue-reducedvalue));
                }
                parentitem = parentitem->parentItem();
             }
        }
    } else {
        if (preValidValue) {
            int addedvalue = 1;
            item->setData((int)DataItemColumn::InvalidValue, QVariant(value+addedvalue));
            ConnectDataItem* parentitem = item->parentItem();
            while(parentitem && parentitem!=mRootItem) {
                int parentvalue = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
                parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue+addedvalue));
                parentitem = parentitem->parentItem();
            }
        }
    }
}

bool ConnectDataModel::isIndexValueValid(int column, ConnectDataItem *item)
{
    QStringList types = item->data((int)DataItemColumn::SchemaType).toString().split(",");
    QString values = item->data((int)DataItemColumn::AllowedValue).toString();
    QStringList allowedValues = (values.isEmpty() ? QStringList() : values.split(","));
    if (column==(int)DataItemColumn::Key) {
        if (item->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ListItem)
            return true;
        if (!allowedValues.isEmpty()) {
            return (allowedValues.contains(item->data((int)DataItemColumn::Key).toString()));
        }
    } else if (column==(int)DataItemColumn::Value) {
              if (QString::compare(item->data((int)DataItemColumn::DefaultValue).toString(),
                                   item->data((int)DataItemColumn::Value).toString(), Qt::CaseInsensitive)==0) {
                  return true;
              }
    }
    int n = 0;
    QStringList schemakeys = item->data((int)DataItemColumn::SchemaKey).toStringList();
    ConnectSchema* schema = schemakeys.isEmpty() ? nullptr : mConnect->getSchema(schemakeys.first());
    QString schemaname;
    if (schema) {
        schemaname = schemakeys.first();
        QString pattern = "^[A-Za-z]+.\\[\\d\\d?\\]";
        QRegularExpression rx(QRegularExpression::anchoredPattern(pattern));
        if (rx.match(schemakeys.last()).hasMatch()) {
            QString key = schemakeys.last();
            schemakeys.removeFirst();
            n = schema->getNumberOfOneOfDefined(schemakeys.join(":"));
            if (n <= 0)
                n = schema->getNumberOfAnyOfDefined(schemakeys.join(":"));
        } else if (QString::compare(schemakeys.last(),"-")==0) {
                   QString key = schemakeys.last();
        }
    } else {
        return true;
    }

    if (schema->isNullDefaultAllowed(schemakeys.join(":"))) {
        if (item->data((int)DataItemColumn::Value).toString().compare("null", Qt::CaseInsensitive)==0) {
            return true;
        }
    }
    int i = 0;
    bool valid = false;
    while (!valid && i<=n ) {
        QString slist = schemakeys.join(":");
        if (schema && n>0) { // if (schema defined) and (anyof defined)
            slist = QString("%1[%2]").arg(schemakeys.join(":"), QString::number(i));
            types = schema->getTypeAsStringList( slist );
            allowedValues = schema->getAllowedValueAsStringList( slist );
        }
        for (const QString& t : std::as_const(types) ) {
            if (t.compare("string")==0 || t.compare("dict")==0 || t.compare("list")==0) {
                if (allowedValues.isEmpty()) {
                    valid = true;
                    break;
                } else {
                      for (const QString& v : std::as_const(allowedValues)) {
                          if (v.compare(item->data((int)DataItemColumn::Value).toString())==0) {
                              valid = true;
                              break;
                          }
                      }
                      if (valid)
                          break;
               }
            } else if (t.compare("boolean")==0) {
                      QStringList booleanlist({"true", "false"});
                      if (booleanlist.contains(item->data((int)DataItemColumn::Value).toString())) {
                          valid = true;
                          break;
                      }
            } else if (t.compare("integer")==0) {
                      QString newdata = (column==(int)DataItemColumn::Key
                                             ? item->data((int)DataItemColumn::Key).toString()
                                             : item->data((int)DataItemColumn::Value).toString());
                      if (allowedValues.isEmpty()) {
                          bool flag = false;
                          if (newdata.contains(mRexWhitechar)) {
                              valid = false;
                              break;
                          }
                          int i = newdata.trimmed().toInt(&flag);
                          if (flag) {
                               if (!schema)
                                   schema = mConnect->getSchema(schemaname);
                              ValueWrapper minvalue = schema->getMin(slist);
                              if (minvalue.type==SchemaValueType::NoValue) {
                                  valid = true;
                                  break;
                              } else if (minvalue.type==SchemaValueType::Integer) {
                                         if (i >= minvalue.value.intval) {
                                             ValueWrapper maxvalue = schema->getMax(slist);
                                             if (maxvalue.type==SchemaValueType::NoValue) {
                                                 valid = true;
                                             } else if (maxvalue.type==SchemaValueType::Integer) {
                                                       if (i <= maxvalue.value.intval) {
                                                           valid=true;
                                                       }
                                             }
                                             break;
                                         }
                              }
                          }
                      } else {
                          for (const QString& v : std::as_const(allowedValues)) {
                              if (v.compare(item->data((int)DataItemColumn::Value).toString())==0) {
                                  valid = true;
                                  break;
                              }
                          }
                          if (valid)
                              break;
                      }
            } // else if (t.compare("float")==0) { }
        }
        ++i;
    }
    return valid;
}

ConnectData *ConnectDataModel::getConnectData()
{
     YAML::Node root = YAML::Node(YAML::NodeType::Sequence);
     Q_ASSERT(mRootItem->childCount()==1);
     for(int i=0; i<mRootItem->childCount(); ++i) {
         ConnectDataItem* rootitem = mRootItem->child(i);
         for(int j=0; j<rootitem->childCount(); ++j) {
            ConnectDataItem* item = rootitem->child(j);
            YAML::Node node = YAML::Node(YAML::NodeType::Map);
            std::string key = item->data((int)DataItemColumn::Key).toString().toStdString();
            YAML::Node mapnode = YAML::Node(YAML::NodeType::Map);
            getData( item, mapnode );
            node[key] = mapnode;
            root[j] = node;
         }
     }
     return new ConnectData(root);
}

bool ConnectDataModel::hasSameParent(const QStringList& tobeinsertSchema, const QStringList& schemaKey, bool samelevel) const
{
    if (samelevel) {
        if (tobeinsertSchema.size()!=schemaKey.size())
            return false;
        for(int i = 0; i<schemaKey.size()-1; ++i) {
            if (tobeinsertSchema.at(i).compare(schemaKey.at(i), Qt::CaseSensitive)!=0) {
                return false;
            }
        }
    } else {
        if (tobeinsertSchema.size()<schemaKey.size())
            return false;
        for(int i = 0; i<schemaKey.size(); ++i) {
            if (tobeinsertSchema.at(i).compare(schemaKey.at(i), Qt::CaseSensitive)!=0) {
                return false;
            }
        }
    }
    return true;
}

bool ConnectDataModel::existsUnderSameParent(const QString& tobeinsertSchema, const QModelIndex &parent, bool samelevel) const
{
    QStringList insertlist = tobeinsertSchema.split(":");
    int level = insertlist.size();
    for (int i =0; i<rowCount(parent); ++i) {
        QString schemaKey = (index(i, (int)DataItemColumn::SchemaKey,parent).data(Qt::DisplayRole).toString());
        QStringList schemakeylist = schemaKey.split(":");
        const QString& tobeinsertedname = insertlist.join(":");
        QString keyname = (schemaKey.endsWith(":-") ? schemaKey.left(schemaKey.lastIndexOf(":-"))
                                                    :schemaKey);
        if (tobeinsertedname.compare(keyname)==0)
            return true;
        if (schemaKey.endsWith("]"))
            if (tobeinsertedname.startsWith( schemaKey.left(schemaKey.indexOf("[")) ))
               return true;
        int keylevel = schemakeylist.size();
        if (!samelevel && qAbs(level-keylevel) != 1 )
            return false;
    }
    return false;
}

bool ConnectDataModel::isRequired(const QStringList &tobeinsertSchema, const QStringList &schemaKey) const
{
    if (tobeinsertSchema.size()==1)
        return true;
    QStringList schemakeylist(tobeinsertSchema);
    schemakeylist.removeFirst();
    ConnectSchema* s = mConnect->getSchema(schemaKey[0]);
    if (s) {
        if (s->isRequired(schemakeylist.join(":"))) {
            return true;
         } else {
             if (s->getAllRequiredKeyList().contains(schemakeylist.join(":") ))
                return true;
         }
    }
    return false;
}

int ConnectDataModel::numberOfExcludedSibling(ConnectDataItem *item)
{
    int sibling = 0;
    if (item->data((int)DataItemColumn::SchemaKey).toStringList().isEmpty())
        return sibling;
    ConnectSchema* s = mConnect->getSchema(item->data((int)DataItemColumn::SchemaKey).toStringList().first());
    if (!s)
        return sibling;
    ConnectDataItem* parent = item->parentItem();
    for(int i=0; i<parent->childCount(); i++) {
        ConnectDataItem* c = parent->child(i);
        if (QString::compare(c->data((int)DataItemColumn::Key).toString(), item->data((int)DataItemColumn::Key).toString())==0)
            continue;
        QStringList excludes = item->data((int)DataItemColumn::ExcludedKeys).toString().split(",");
        if (excludes.contains( c->data((int)DataItemColumn::Key).toString() )) {
            ++sibling;
        }
    }
    return sibling;
}

int ConnectDataModel::numberOfExcludedChildren(ConnectDataItem *item)
{
    int children = 0;
    if (item->childCount() > 1) {
        for (int i=0; i<item->childCount(); ++i) {
            ConnectDataItem *citem = item->child(i);
            QStringList levelExcludedKeyList = citem->data((int)DataItemColumn::ExcludedKeys).toString().split(",");
            if (!levelExcludedKeyList.isEmpty())
                ++children;
        }
    }
    return children;
}

QModelIndex ConnectDataModel::getSchemaParentIndex(const QModelIndex &idx)
{
    QModelIndex parentidx = idx;
    ConnectDataItem *item = getItem(idx);
    if (item) {
        parentidx = parentidx.parent();
        while(parentidx.parent().isValid()) {
            if (parentidx.siblingAtColumn((int)DataItemColumn::SchemaKey).data().toString().split(":").size()>1)
                parentidx = parentidx.parent();
            else
                break;
        }
       return parentidx;
    } else {
        return QModelIndex();
    }

}

ConnectDataItem *ConnectDataModel::getSchemaParentItem(ConnectDataItem *item)
{
    ConnectDataItem* parentitem = item;
    if (parentitem) {
        parentitem = parentitem->parentItem();
        while(parentitem) {
            if (parentitem->data((int)DataItemColumn::SchemaKey).toStringList().size() != 1)
                parentitem = parentitem->parentItem();
            else
                break;
        }
       return parentitem;
    }
    return nullptr;
}

int ConnectDataModel::whichAnyOfSchema(const YAML::Node &data, ConnectSchema *schema, const QStringList &keylist, const QString &key)
{
    QStringList keystrlist(keylist);
    keystrlist << key;
    for(int i =0; i<schema->getNumberOfAnyOfDefined( keystrlist.join(":")); ++i) {
        QString schemastr = QString("%1[%2]").arg( keystrlist.join(":"), QString::number(i));
        bool validType = false;
        const QList<SchemaType> typeList = schema->getType(schemastr);
        for (SchemaType type : typeList) {
           try {
               if (type==SchemaType::Integer) {
                   if (data.Type()==YAML::NodeType::Scalar) {
                      data.as<int>();
                      validType = true;
                      break;
                   } else
                       continue;
               } else if (type==SchemaType::Float) {
                         if (data.Type()==YAML::NodeType::Scalar) {
                             data.as<float>();
                             validType = true;
                         } else
                            continue;
               } else if (type==SchemaType::Boolean) {
                          if (data.Type()==YAML::NodeType::Scalar) {
                              data.as<bool>();
                              validType = true;
                          } else
                              continue;
               } else if (type==SchemaType::String) {
                         if (data.Type()==YAML::NodeType::Scalar) {
                             data.as<std::string>();
                             validType = true;
                         } else
                             continue;
               } else if (type==SchemaType::List) {
                         continue; // TODO
               } else if (type==SchemaType::Dict) {
                         if (data.Type()==YAML::NodeType::Map) {
                             validType = true;
                         } else {
                             continue; // TODO
                        }
               }
               break;
           } catch (const YAML::BadConversion &) {
               continue;
           }
        }
        if (validType)
            return i;
    }
    return 0;
}

// oneof
int ConnectDataModel::whichOneOfSchema(const YAML::Node &data, ConnectSchema *schema, const QStringList &keylist, const QString &key)
{
    QStringList keystrlist(keylist);
    keystrlist << key;
    for(int i =0; i<schema->getNumberOfOneOfDefined( keystrlist.join(":")); ++i) {
        QString schemastr = QString("%1[%2]").arg( keystrlist.join(":"), QString::number(i));
        bool validType = false;
        const QList<SchemaType> typeList = schema->getType(schemastr);
        for (SchemaType type : typeList) {
            try {
                if (type==SchemaType::Integer) {
                    if (data.Type()==YAML::NodeType::Scalar) {
                        data.as<int>();
                        validType = true;
                        break;
                    } else
                        continue;
                } else if (type==SchemaType::Float) {
                    if (data.Type()==YAML::NodeType::Scalar) {
                        data.as<float>();
                        validType = true;
                    } else
                        continue;
                } else if (type==SchemaType::Boolean) {
                    if (data.Type()==YAML::NodeType::Scalar) {
                        data.as<bool>();
                        validType = true;
                    } else
                        continue;
                } else if (type==SchemaType::String) {
                    if (data.Type()==YAML::NodeType::Scalar) {
                        std::string str = data.as<std::string>();
                        if (str.compare("false")==0 || str.compare("true")==0)
                            continue;
                        validType = true;
                    } else
                        continue;
                } else if (type==SchemaType::List) {
                          if (data.Type()==YAML::NodeType::Sequence) {
                             validType = true;
                          } else
                              continue; // TODO
                } else if (type==SchemaType::Dict) {
                    if (data.Type()==YAML::NodeType::Map) {
                        validType = true;
                    } else {
                        continue; // TODO
                    }
                }
                break;
            } catch (const YAML::BadConversion &) {
                continue;
            }
        }
        if (validType)
            return i;
    }
    return 0;
}

// oneof_schema
int ConnectDataModel::whichOneOfSchema(const YAML::Node &data, ConnectSchema *schema, const QString &key)
{
    int n = 0;
    QStringList oneofkeystr;
    getSchemaListFromData(data, oneofkeystr);
    const QStringList oneofschemakeys = schema->getAllOneOfSchemaKeys(key);
    for(const QString& schemastr : oneofschemakeys) {
        QStringList sstr(schemastr);
        bool found = false;
        for(const QString& kstr : oneofkeystr) {
            QString str = kstr;
            if (schema->isOneOfDefined(sstr.join(":") + ":" + kstr))
                str += "[0]";
            if (schema->getSchema( sstr.join(":") + ":" + str )) {
                found = true;
            } else {
                found = false;
                break;
            }
        }
        if (found)
            break;
        ++n;
    }
    return n;
}

void ConnectDataModel::getSchemaListFromData(const YAML::Node &data, QStringList& schemaList)
{
    if (data.Type() == YAML::NodeType::Sequence) {
        for(size_t k = 0; k<data.size(); k++) {
            getSchemaListFromData(data[k], schemaList);
        }
    } else if (data.Type()==YAML::NodeType::Map) {
              for (YAML::const_iterator mit = data.begin(); mit !=data.end(); ++mit) {
                  schemaList << QString::fromStdString(mit->first.as<std::string>());
                  getSchemaListFromData(mit->second, schemaList);
              }
    }
}

void ConnectDataModel::getData(ConnectDataItem *item, YAML::Node& node)
{
    for(int i=0; i<item->childCount(); ++i) {
        ConnectDataItem* childitem = item->child(i);
        if (childitem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::SchemaName) {
            std::string key = childitem->data((int)DataItemColumn::Key).toString().toStdString();
            YAML::Node mapnode = YAML::Node(YAML::NodeType::Map);
            getData( childitem, mapnode );
            node[key] = mapnode;
        } else if (childitem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::KeyItem) {
            std::string key = childitem->data((int)DataItemColumn::Key).toString().toStdString();
            YAML::Node mapnode;
            getData( childitem, mapnode );
            node[key] = mapnode;
        } else if (childitem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ListItem) {
                   YAML::Node mapnode;
                   for(int j=0; j<childitem->childCount(); ++j) {  // skip row with DataCheckState::ListItem
                       ConnectDataItem* seqchilditem = childitem->child(j);
                       std::string seqmapkey = seqchilditem->data((int)DataItemColumn::Key).toString().toStdString();
                       if (seqchilditem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementValue ||
                           seqchilditem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementMap      ) {
                           YAML::Node mapseqnode;
                           getData(seqchilditem, mapseqnode);
                           if (seqchilditem->data((int)DataItemColumn::Value).toString().toLower().compare("null")==0)
                               mapnode[seqmapkey] = YAML::Node(YAML::NodeType::Null);
                           else
                               mapnode[seqmapkey] = seqchilditem->data((int)DataItemColumn::Value).toString().toStdString();;
                       } else if (seqchilditem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementKey ) {
                           mapnode = seqmapkey;
                       } else {
                           YAML::Node mapseqnode;
                           getData(seqchilditem, mapseqnode);
                           mapnode[seqmapkey] = mapseqnode;
                       }
                   }
                   node[i] = mapnode;
        } else if (childitem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementValue ||
                   childitem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementMap      ) {
                   std::string key = childitem->data((int)DataItemColumn::Key).toString().toStdString();
                   if (childitem->data((int)DataItemColumn::Value).toString().toLower().compare("null")==0)
                       node[key] = YAML::Node(YAML::NodeType::Null);
                   else
                       node[key] = childitem->data((int)DataItemColumn::Value).toString().toStdString();
        }  else if (childitem->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ElementKey) {
                  YAML::Node node = YAML::Node(YAML::NodeType::Scalar);
                  node = YAML::Node(childitem->data((int)DataItemColumn::Key).toString().toStdString());
        }
    }
}

void ConnectDataModel::informDataChanged(const QModelIndex& parent)
{
    QModelIndex checkstate_index = parent.sibling(parent.row(), (int)DataItemColumn::CheckState);
    int state = checkstate_index.data(Qt::DisplayRole).toInt();
    int childcount = getItem(parent)->childCount();
    if ((int)DataCheckState::KeyItem==state) {
        if (childcount <= 2) {
            for (int i=0; i<childcount; ++i) {
                ConnectDataItem* item = getItem(index(i, 0, parent));
                item->setData((int)DataItemColumn::MoveDown, QVariant( false ));
                item->setData((int)DataItemColumn::MoveUp, QVariant( false ));
                informDataChanged( index(i, 0, parent) );
            }
        } else { // childcount > 2
            int i = 0;
            while (i<childcount-1) {
                ConnectDataItem* item = getItem(index(i, 0, parent));
                if (item->data((int)DataItemColumn::CheckState).toUInt()==(int)DataCheckState::ListItem) {
                    bool firstChild = (i==0 && i!=(childcount-2));
                    bool lastChild  = (i!=0 && i==(childcount-2));
                    item->setData((int)DataItemColumn::MoveDown, QVariant( !lastChild ));
                    item->setData((int)DataItemColumn::MoveUp, QVariant( !firstChild ));
                } else if (item->data((int)DataItemColumn::CheckState).toUInt()==(int)DataCheckState::ElementMap) {
                           if (item->data((int)DataItemColumn::SchemaKey).toStringList().isEmpty()) {
                                bool firstChild = (i==0 && i!=(childcount-2));
                                bool lastChild  = (i!=0 && i==(childcount-2));
                                item->setData((int)DataItemColumn::MoveDown, QVariant( !lastChild ));
                                item->setData((int)DataItemColumn::MoveUp, QVariant( !firstChild ));
                           } else {
                                item->setData((int)DataItemColumn::MoveDown, QVariant( false ));
                                item->setData((int)DataItemColumn::MoveUp, QVariant( false ));
                           }
                } else {
                    item->setData((int)DataItemColumn::MoveDown, QVariant( false ));
                    item->setData((int)DataItemColumn::MoveUp, QVariant( false ));
                }
                informDataChanged( index(i, 0, parent) );
                ++i;
            }
            ConnectDataItem* item = getItem(index(i, 0, parent));
            item->setData((int)DataItemColumn::MoveDown, QVariant( false ));
            item->setData((int)DataItemColumn::MoveUp, QVariant( false ));
        }
    } else {
        for (int i=0; i<childcount; ++i) {
            ConnectDataItem* item = getItem(index(i, 0, parent));
            if (item->data((int)DataItemColumn::CheckState).toUInt()==(int)DataCheckState::ListItem  ||
                item->data((int)DataItemColumn::CheckState).toUInt()==(int)DataCheckState::SchemaName   ) {
                item->setData((int)DataItemColumn::MoveDown, QVariant( !item->isLastChild()) );
                item->setData((int)DataItemColumn::MoveUp, QVariant( !item->isFirstChild()) );
            } else {
                item->setData((int)DataItemColumn::MoveDown, QVariant( false ));
                item->setData((int)DataItemColumn::MoveUp, QVariant( false ));
            }
            informDataChanged( index(i, 0, parent) );
        }
    }

    emit dataChanged(index(0, (int)DataItemColumn::MoveDown, parent),
                     index(getItem(parent)->childCount()-1, (int)DataItemColumn::ElementID, parent),
                     QVector<int> { Qt::DisplayRole, Qt::ToolTipRole, Qt::DecorationRole} );

}

void ConnectDataModel::setupTreeItemModelData()
{
    QList<QVariant> rootData;
    rootData << "Key"               << "Value"         << "" /*State*/    << "" /*Type*/
             << "" /*AllowedValue*/ << "" /*Required*/ << "" /*MoveDown*/ << "" /*MoveUp*/
             << "" /*ID*/           << "" /*Schema*/   << "" /*UnDefined*/<< "" /*InvalidValue*/
             << "" /*ExcludesKeys*/ << "" /*DefaultValue*/;

    mRootItem = new ConnectDataItem(rootData, mItemIDCount++);

    QList<ConnectDataItem*> parents;
    parents << mRootItem;

    QList<QVariant> rootListData;
    rootListData << QVariant();
    rootListData << QVariant();
    rootListData << QVariant((int)DataCheckState::Root);
    rootListData << QVariant();
    rootListData << QVariant();
    rootListData << QVariant();
    rootListData << QVariant();
    rootListData << QVariant();
    rootListData << QVariant();
    rootListData << QVariant();
    rootListData << QVariant();
    rootListData << QVariant(0);
    rootListData << QVariant();
    rootListData << QVariant();
    parents.last()->appendChild(new ConnectDataItem(rootListData, mItemIDCount++, parents.last()));
    parents << parents.last()->child(parents.last()->childCount()-1);

    if (!mConnectData->getRootNode().IsNull()) {
        YAML::Node node = mConnectData->getRootNode();
        if (node.Type()!=YAML::NodeType::Sequence)
            EXCEPT() << "Error: The file content might be corrupted or incorrectly overwritten";

        int position = 0;
        for(size_t i = 0; i<node.size(); i++) {
            for (YAML::const_iterator it = node[i].begin(); it != node[i].end(); ++it) {
                QString schemaName = QString::fromStdString(it->first.as<std::string>());
                ConnectSchema* schema = mConnect->getSchema(schemaName);
                QStringList dataKeys;
                QList<QVariant> listData;
                listData << schemaName;
                listData << "";
                listData << QVariant((int)DataCheckState::SchemaName);
                listData << QVariant(QString());
                listData << QVariant(QString());
                listData << QVariant(true);
                listData << QVariant();
                listData << QVariant();
                listData << QVariant();
                listData << QVariant(schemaName);
                listData << (schema ? QVariant(false) : QVariant(true));
                listData << QVariant(0);
                listData << QVariant();
                listData << QVariant();
                if (position>=parents.last()->childCount() || position < 0) {
                    parents.last()->appendChild(new ConnectDataItem(listData, mItemIDCount++, parents.last()));
                    parents << parents.last()->child(parents.last()->childCount()-1);
                } else {
                    parents.last()->insertChild(position, new ConnectDataItem(listData, mItemIDCount++, parents.last()));
                    parents << parents.last()->child(position);
                }

                insertSchemaData(schemaName, dataKeys, new ConnectData(it->second), -1, parents);

                parents.pop_back();
            }
            position++;
        }
        informDataChanged( index(0,0).parent() );
    }
}

void ConnectDataModel::insertSchemaData(const QString& schemaName, const QStringList& keys, ConnectData* data, int position, QList<ConnectDataItem*>& parents)
{
    ConnectSchema* schema = mConnect->getSchema(schemaName);
    QStringList dataKeys(keys);
    QStringList schemaKeys;
    schemaKeys << schemaName;
    if (!dataKeys.isEmpty())
        schemaKeys << dataKeys;
    for (YAML::const_iterator mit = data->getRootNode().begin(); mit != data->getRootNode().end(); ++mit) {
         QString mapToSequenceKey = "";
         if (mit->second.Type()==YAML::NodeType::Scalar || mit->second.Type()==YAML::NodeType::Null) {
             QString key = QString::fromStdString(mit->first.as<std::string>());
             QStringList keyslist(dataKeys);
             keyslist << key;
             if (schema) {
                 if (schema->isOneOfDefined(keyslist.join(":"))) {
                     int n = whichOneOfSchema(mit->second, schema, dataKeys, key);
                     key += QString("[%1]").arg(n);
                 } else if (schema->isAnyOfDefined(keyslist.join(":"))) {
                     int n = whichAnyOfSchema(mit->second, schema, dataKeys, key);
                     key += QString("[%1]").arg(n);
                 }
             }
             dataKeys << key;
             schemaKeys << key;
             QStringList typelist;
             QVariant defvalue;
             QStringList excluded;
             if (schema) {
                 typelist = schema->getTypeAsStringList(dataKeys.join(":"));
                 defvalue = schema->getDefaultValue(dataKeys.join(":"));
                 excluded = schema->getExcludedKeys(dataKeys.join(":"));
             }
             int column = (int)DataItemColumn::Key;
             QList<QVariant> itemData;
             itemData << (key.contains("[") ? key.left(key.lastIndexOf("[")) : key);
             if (typelist.contains("dict") || (typelist.contains("list"))) {
                 itemData << QVariant();
                 itemData << QVariant( (int)DataCheckState::KeyItem);
                 column = (int)DataItemColumn::Key;
             } else {
                 itemData << (mit->second.Type()==YAML::NodeType::Scalar ? QVariant( QString::fromStdString(mit->second.as<std::string>()) )
                                                                         : QVariant( "null" ) );
                 itemData << QVariant((int)DataCheckState::ElementValue);
                 column = (int)DataItemColumn::Value;
             }
             itemData << (schema ? QVariant(typelist.join(",")) : QVariant());
             itemData << (schema ? QVariant(schema->getAllowedValueAsStringList(dataKeys.join(":")).join(",")) : QVariant());
             if (!typelist.contains("dict") && !typelist.contains("list") && key.endsWith("]")) { // take rquire flag from parent schema
                 QStringList skeylist(dataKeys);
                 skeylist.removeLast();
                 skeylist << key.left(key.lastIndexOf("["));
                 itemData << (schema ? QVariant(!schema->isRequired(skeylist.join(":"))) : QVariant());
             } else {
                itemData << (schema ? QVariant(!schema->isRequired(dataKeys.join(":"))) : QVariant());
             }
             itemData << QVariant();
             itemData << QVariant();
             itemData << QVariant();
             itemData << QVariant(schemaKeys);
             itemData << (schema ? (schema->contains(dataKeys.join(":")) ? QVariant(false) : QVariant(true))
                                 : QVariant(true));
             itemData << QVariant(0);
             itemData << (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")));
             itemData << defvalue;
             ConnectDataItem* item = new ConnectDataItem(itemData, mItemIDCount++, parents.last());
             if (!isIndexValueValid(column, item)) {
                 item->setData((int)DataItemColumn::InvalidValue, 1);
                 updateInvaldItem(column, item);
             }
             if (position>=parents.last()->childCount() || position < 0) {
                 parents.last()->appendChild(item);
                 if (typelist.contains("dict") || typelist.contains("list"))
                     parents << parents.last()->child(parents.last()->childCount()-1);
             } else {
                 parents.last()->insertChild(position, item);
                 if (typelist.contains("dict") || typelist.contains("list"))
                     parents << parents.last()->child(position);
             }
             updateInvalidExcludedItem(item);
             if (typelist.contains("dict") || typelist.contains("list")) {
                 QList<QVariant> sequenceDummyData;
                 sequenceDummyData << (key.contains("[") ? key.left(key.lastIndexOf("[")) : key);
                 sequenceDummyData << "";
                 sequenceDummyData << (typelist.contains("dict") ? (schema->isSchemaDefined(dataKeys.join(":")) ? QVariant((int)DataCheckState::MapSchemaAppend)
                                                                                                                : QVariant((int)DataCheckState::MapAppend))
                                                                 : QVariant((int)DataCheckState::ListAppend));
                 sequenceDummyData << QVariant(QString());
                 sequenceDummyData << QVariant(QString());
                 sequenceDummyData << QVariant(false);
                 sequenceDummyData << QVariant();
                 sequenceDummyData << QVariant();
                 sequenceDummyData << QVariant();
                 QStringList keys(dataKeys);
                 keys.insert(0,schemaName);
                 sequenceDummyData << QVariant(keys);
                 sequenceDummyData << (schema ? QVariant(false) : QVariant(true));
                 sequenceDummyData << QVariant(0);
                 sequenceDummyData << QVariant();
                 sequenceDummyData << QVariant();
                 parents.last()->appendChild(new ConnectDataItem(sequenceDummyData, mItemIDCount++, parents.last()));
                 parents.pop_back();
             }
             dataKeys.removeLast();
             schemaKeys.removeLast();
         } else if (mit->second.Type()==YAML::NodeType::Map) {
                   QString key = QString::fromStdString(mit->first.as<std::string>());
                   QStringList keyslist(dataKeys);
                   keyslist << key;
                   if (schema) {
                       if (schema->isOneOfDefined(keyslist.join(":"))) {
                           int n = whichOneOfSchema(mit->second, schema, dataKeys, key);
                           key += QString("[%1]").arg(n);
                       } else if (schema->isAnyOfDefined(keyslist.join(":"))) {
                           int n = whichAnyOfSchema(mit->second, schema, dataKeys, key);
                           key += QString("[%1]").arg(n);
                       }
                   }
                   dataKeys << key;
                   schemaKeys << key;
                   QList<QVariant> itemData;
                   itemData << (key.contains("[") ? key.left(key.lastIndexOf("[")) : key);
                   itemData << ""; // TODO
                   itemData << QVariant((int)DataCheckState::KeyItem);
                   itemData << (schema ? QVariant(schema->getTypeAsStringList(dataKeys.join(":")).join(",")) : QVariant());
                   itemData << (schema ? QVariant(schema->getAllowedValueAsStringList(dataKeys.join(":")).join(",")) : QVariant());
                   itemData << (schema ? QVariant(!schema->isRequired(dataKeys.join(":"))) : QVariant());
                   itemData << QVariant();
                   itemData << QVariant();
                   itemData << QVariant();
                   itemData << QVariant(schemaKeys);
                   itemData << (schema ? (schema->contains(dataKeys.join(":")) ? QVariant(false) : QVariant(true))
                                       : QVariant(true));
                   itemData << QVariant(0);
                   QStringList excluded = schema->getExcludedKeys(dataKeys.join(":"));
                   itemData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                       : QVariant());
                   itemData << (schema ? schema->getDefaultValue(dataKeys.join(":"))
                                       : QVariant());
                   ConnectDataItem* item = new ConnectDataItem(itemData, mItemIDCount++, parents.last());
                   if (!updateInvaldItem((int)DataItemColumn::Key, item))
                       updateInvaldItem((int)DataItemColumn::Value, item);
                   if (position>=parents.last()->childCount() || position < 0) {
                       parents.last()->appendChild(item);
                       parents << parents.last()->child(parents.last()->childCount()-1);
                   } else {
                       parents.last()->insertChild(position, item);
                       parents << parents.last()->child(position);
                   }
                   updateInvalidExcludedItem(item);
                   int k = 0;
                   for (YAML::const_iterator dmit = mit->second.begin(); dmit != mit->second.end(); ++dmit) {
                       QString mapkey = QString::fromStdString(dmit->first.as<std::string>());
//                       dataKeys << mapkey;
                       QStringList sKeys(schemaKeys);
                       sKeys << mapkey;
                       QStringList checkKeys(dataKeys);
                       checkKeys << mapkey;
                       if (dmit->second.Type()==YAML::NodeType::Scalar) {
                           QList<QVariant> mapitemData;
                           mapitemData << mapkey;
                           mapitemData << QVariant(dmit->second.as<std::string>().c_str());
                           mapitemData << QVariant((int)DataCheckState::ElementMap);
                           mapitemData << (schema ? QVariant(schema->getTypeAsStringList(checkKeys.join(":")).join(",")) : QVariant());
                           mapitemData << (schema ? QVariant(schema->getAllowedValueAsStringList(checkKeys.join(":")).join(",")) : QVariant());
                           mapitemData << (schema ? QVariant(!schema->isRequired(checkKeys.join(":"))) : QVariant());
                           mapitemData << QVariant();
                           mapitemData << QVariant();
                           mapitemData << QVariant();
                           mapitemData << (schema ? (schema->contains(checkKeys.join(":")) ? sKeys : QVariant(QStringList()))
                                                  : QVariant(QStringList()));
                           mapitemData << QVariant(false);
                           mapitemData << QVariant(0);
                           QStringList excluded = schema->getExcludedKeys(checkKeys.join(":"));
                           mapitemData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                                  : QVariant());
                           mapitemData << (schema ? (schema->getDefaultValue(checkKeys.join(":")))
                                                  : QVariant());
                           ConnectDataItem* item = new ConnectDataItem(mapitemData, mItemIDCount++, parents.last());
                           if (!updateInvaldItem((int)DataItemColumn::Key, item))
                               updateInvaldItem((int)DataItemColumn::Value, item);
                           parents.last()->appendChild(item);
                           k++;
                       } else if (dmit->second.Type()==YAML::NodeType::Null) {
                                  QList<QVariant> mapitemData;
                                  mapitemData << mapkey;
                                  mapitemData << "null";
                                  mapitemData << QVariant((int)DataCheckState::ElementMap);
                                  mapitemData << (schema ? QVariant(schema->getTypeAsStringList(checkKeys.join(":")).join(",")) : QVariant());
                                  mapitemData << (schema ? QVariant(schema->getAllowedValueAsStringList(checkKeys.join(":")).join(",")) : QVariant());
                                  mapitemData << (schema ? QVariant(!schema->isRequired(checkKeys.join(":"))) : QVariant());
                                  mapitemData << QVariant();
                                  mapitemData << QVariant();
                                  mapitemData << QVariant();
                                  mapitemData << (schema ? (schema->contains(checkKeys.join(":")) ? sKeys : QVariant(QStringList()))
                                                         : QVariant(QStringList()));
                                  mapitemData << (schema ? (schema->contains(checkKeys.join(":")) ? QVariant(false) : QVariant(true))
                                                         : QVariant(true));
                                  mapitemData << QVariant(0);
                                  QStringList excluded = schema->getExcludedKeys(checkKeys.join(":"));
                                  mapitemData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                                         : QVariant());
                                  mapitemData << (schema ? (schema->getDefaultValue(checkKeys.join(":")))
                                                         : QVariant());
                                  ConnectDataItem* item = new ConnectDataItem(mapitemData, mItemIDCount++, parents.last());
                                  if (!updateInvaldItem((int)DataItemColumn::Key, item))
                                      updateInvaldItem((int)DataItemColumn::Value, item);
                                  parents.last()->appendChild(item);
                                  k++;
                       } else if (dmit->second.Type()==YAML::NodeType::Sequence) {
                                 QString key = QString::fromStdString(dmit->first.as<std::string>());
                                 dataKeys   << key;
                                 schemaKeys << key;
                                 QList<QVariant> seqSeqData;
                                 seqSeqData << key;
                                 seqSeqData << "";
                                 seqSeqData << QVariant((int)DataCheckState::KeyItem);
                                 seqSeqData << (schema ? QVariant(schema->getTypeAsStringList(dataKeys.join(":")).join(",")) : QVariant());
                                 seqSeqData << (schema ? QVariant(schema->getAllowedValueAsStringList(dataKeys.join(":")).join(",")) : QVariant());
                                 seqSeqData << (schema ? QVariant(!schema->isRequired(dataKeys.join(":"))) : QVariant());
                                 seqSeqData << QVariant();
                                 seqSeqData << QVariant();
                                 seqSeqData << QVariant();
                                 seqSeqData << QVariant(schemaKeys);
                                 seqSeqData << (schema ? (schema->contains(dataKeys.join(":")) ? QVariant(false) : QVariant(true))
                                                       : QVariant(true));
                                 seqSeqData << QVariant(0);
                                 QStringList excluded = schema->getExcludedKeys(dataKeys.join(":"));
                                 seqSeqData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                                       : QVariant());
                                 seqSeqData << (schema ? (schema->getDefaultValue(dataKeys.join(":")))
                                                       : QVariant());
                                 ConnectDataItem* item = new ConnectDataItem(seqSeqData, mItemIDCount++, parents.last());
                                 if (!updateInvaldItem((int)DataItemColumn::Key, item))
                                     updateInvaldItem((int)DataItemColumn::Value, item);
                                 parents.last()->appendChild(item);
                                 dataKeys   << "-";
                                 schemaKeys << "-";
                                 parents << parents.last()->child(parents.last()->childCount()-1);
                                 for(size_t kk = 0; kk<dmit->second.size(); kk++) {
                                     QList<QVariant> indexSeqData;
                                     indexSeqData << QVariant::fromValue(kk);
                                     indexSeqData << QVariant(QStringList());
                                     indexSeqData << QVariant((int)DataCheckState::ListItem);

                                     indexSeqData << QVariant(QString());
                                     indexSeqData << QVariant(QString());
                                     QStringList Keys(dataKeys);
                                     if (Keys.last().compare("-")==0)
                                         Keys.removeLast();
                                     indexSeqData << QVariant(true);
                                     indexSeqData << QVariant();
                                     indexSeqData << QVariant();
                                     indexSeqData << QVariant();
                                     indexSeqData << QVariant(schemaKeys);
                                     indexSeqData << QVariant(false);
                                     indexSeqData << QVariant(0);
                                     indexSeqData << QVariant();
                                     indexSeqData << (schema ? (schema->getDefaultValue(Keys.join(":"))) : QVariant());
                                     parents.last()->appendChild(new ConnectDataItem(indexSeqData, mItemIDCount++, parents.last()));
                                }
                                updateInvalidExcludedItem(item);
                                parents.pop_back();
                                dataKeys.removeLast();
                                schemaKeys.removeLast();

                       } else {
                           Q_ASSERT(dmit->second.Type()==YAML::NodeType::Scalar || dmit->second.Type()==YAML::NodeType::Sequence );
                       }
//                       dataKeys.removeLast();
                   }
////                   updateInvalidExcludedItem(item);
                   if (!schema->isSchemaDefined(key)) {
                      QList<QVariant> sequenceDummyData;
                      sequenceDummyData << (key.contains("[") ? key.left(key.lastIndexOf("[")) : key);
                      sequenceDummyData << "";
                      sequenceDummyData << QVariant((int)DataCheckState::MapAppend);
                      sequenceDummyData << QVariant(QString());
                      sequenceDummyData << QVariant(QString());
                      sequenceDummyData << QVariant(false);
                      sequenceDummyData << QVariant();
                      sequenceDummyData << QVariant();
                      sequenceDummyData << QVariant();
                      QStringList keys(dataKeys);
                      keys.insert(0,schemaName);
                      sequenceDummyData << QVariant(keys);
                      sequenceDummyData << (schema ? QVariant(false) : QVariant(true));
                      sequenceDummyData << QVariant(0);
                      sequenceDummyData << QVariant();
                      sequenceDummyData << QVariant();
                      parents.last()->appendChild(new ConnectDataItem(sequenceDummyData, mItemIDCount++, parents.last()));
                   }
                   parents.pop_back();
                   dataKeys.removeLast();
                   schemaKeys.removeLast();
         } else if (mit->second.Type()==YAML::NodeType::Sequence) {
             QString key = QString::fromStdString(mit->first.as<std::string>());
             bool isAnyofDefined = schema ? schema->isAnyOfDefined(key) : false;
             bool isOneofDefined = schema ? schema->isOneOfDefined(key) : false;
             if (isOneofDefined) {
                 isOneofDefined=(schema ? schema->getNumberOfOneOfDefined(key) > 0 : false);
                 int n = whichOneOfSchema(mit->second, schema, dataKeys, key);
                 key += QString("[%1]").arg(n);
             } else if (isAnyofDefined) {
                    int n = whichAnyOfSchema(mit->second, schema, dataKeys, key);
                    key += QString("[%1]").arg(n);
             }
             mapToSequenceKey = key;
             dataKeys   << key;
             schemaKeys << key;
             QStringList typelist = schema->getTypeAsStringList(dataKeys.join(":"));
             QVariant defvalue = schema->getDefaultValue(dataKeys.join(":"));
             QList<QVariant> itemData;
             itemData << (key.contains("[") ? key.left(key.lastIndexOf("[")) : key);
             itemData << "";
             itemData << QVariant((int)DataCheckState::KeyItem);
             itemData << (schema ? QVariant(schema->getTypeAsStringList(dataKeys.join(":")).join(",")) : QVariant());
             itemData << (schema ? QVariant(schema->getAllowedValueAsStringList(dataKeys.join(":")).join(",")) : QVariant());
             itemData << (schema ? QVariant(!schema->isRequired(dataKeys.join(":"))) : QVariant());
             itemData << QVariant();
             itemData << QVariant();
             itemData << QVariant();
             itemData << QVariant(schemaKeys);
             itemData << (schema ? (schema->contains(dataKeys.join(":")) ? QVariant(false) : QVariant(true))
                                 : QVariant(true));
             itemData << QVariant(0);
             QStringList excluded = schema->getExcludedKeys(dataKeys.join(":"));
             itemData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                 : QVariant());
             itemData << (schema ? (schema->getDefaultValue(dataKeys.join(":")))
                                 : QVariant());
             ConnectDataItem* item = new ConnectDataItem(itemData, mItemIDCount++, parents.last());
             updateInvaldItem((int)DataItemColumn::Key, item);
             if (position>=parents.last()->childCount() || position < 0) {
                 parents.last()->appendChild(item);
                 parents << parents.last()->child(parents.last()->childCount()-1);
             } else {
                 parents.last()->insertChild(position, item);
                 parents << parents.last()->child(position);
             }
             dataKeys   << "-";
//             if (!isOneofDefined)
             schemaKeys << "-";
             QString keystr;
             QStringList schemakeyslist;
             QStringList datakeyslist;
             for(size_t k = 0; k<mit->second.size(); k++) {
                 keystr = key;
                 schemakeyslist = schemaKeys;
                 datakeyslist   = dataKeys;
                 QList<QVariant> indexData;
                 indexData << QVariant::fromValue(k);
                 indexData << QVariant(QStringList());
                 indexData << QVariant((int)DataCheckState::ListItem);
                 indexData << QVariant(QString());
                 indexData << QVariant(QString());
                 QStringList Keys(datakeyslist);
//                 if (Keys.last().compare("-")==0)
//                     Keys.removeLast();
                 indexData << QVariant(true);
                 indexData << QVariant();
                 indexData << QVariant();
                 indexData << QVariant();
                 indexData << QVariant(schemakeyslist);
                 indexData << QVariant(false);
                 indexData << QVariant(0);
                 indexData << QVariant();
                 indexData << QVariant();
                 parents.last()->appendChild(new ConnectDataItem(indexData, mItemIDCount++, parents.last()));
                 if (mit->second[k].Type()==YAML::NodeType::Map) {
                           parents << parents.last()->child(parents.last()->childCount()-1);
                           const YAML::Node mapnode = mit->second[k];
                           for (YAML::const_iterator mmit = mapnode.begin(); mmit != mapnode.end(); ++mmit) {
                               keystr =  QString::fromStdString( mmit->first.as<std::string>() );
                               QStringList mapkeyslist(datakeyslist);
                               mapkeyslist << keystr;
                               if (schema) {
                                   if (schema->isOneOfDefined(mapkeyslist.join(":"))) {
                                       int n = whichOneOfSchema(mmit->second, schema, datakeyslist, keystr);
                                       keystr += QString("[%1]").arg(n);
                                   } else if (schema->isAnyOfDefined(mapkeyslist.join(":"))) {
                                            int n = whichAnyOfSchema(mmit->second, schema, datakeyslist, keystr);
                                            keystr += QString("[%1]").arg(n);
                                   }
                               }
                               int before = datakeyslist.size();
                               datakeyslist   << keystr;
                               schemakeyslist << keystr;
                               if (mmit->second.Type()==YAML::NodeType::Sequence) {
                                   QStringList typelist = schema->getTypeAsStringList(datakeyslist.join(":"));
                                   QVariant defvalue = schema->getDefaultValue(datakeyslist.join(":"));
                                   QList<QVariant> seqSeqData;
                                   seqSeqData << (keystr.contains("[") ? keystr.left(keystr.lastIndexOf("[")) : keystr);
                                   seqSeqData << "";
                                   seqSeqData << QVariant((int)DataCheckState::KeyItem);
                                   seqSeqData << (schema ? QVariant(schema->getTypeAsStringList(datakeyslist.join(":")).join(",")) : QVariant());
                                   seqSeqData << (schema ? QVariant(schema->getAllowedValueAsStringList(datakeyslist.join(":")).join(",")) : QVariant());
                                   seqSeqData << (schema ? QVariant(!schema->isRequired(datakeyslist.join(":"))) : QVariant(false));
                                   seqSeqData << QVariant();
                                   seqSeqData << QVariant();
                                   seqSeqData << QVariant();
                                   seqSeqData << QVariant(schemakeyslist);
                                   seqSeqData << (schema ? (schema->contains(datakeyslist.join(":")) ? QVariant(false) : QVariant(true))
                                                         : QVariant(true));
                                   seqSeqData << QVariant(0);
                                   QStringList excluded = schema->getExcludedKeys(datakeyslist.join(":"));
                                   seqSeqData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                                         : QVariant());
                                   seqSeqData << (schema ? (schema->getDefaultValue(datakeyslist.join(":")))
                                                          : QVariant());
                                   ConnectDataItem* item = new ConnectDataItem(seqSeqData, mItemIDCount++, parents.last());
                                   if (!updateInvaldItem((int)DataItemColumn::Key, item))
                                       updateInvaldItem((int)DataItemColumn::Value, item);
                                   parents.last()->appendChild(item);

                                   datakeyslist   << "-";
                                   schemakeyslist << "-";
                                   parents << parents.last()->child(parents.last()->childCount()-1);
                                   for(size_t kk = 0; kk<mmit->second.size(); kk++) {
                                       QList<QVariant> indexSeqData;
                                       indexSeqData << QVariant::fromValue(kk);
                                       indexSeqData << QVariant(QStringList());
                                       indexSeqData << QVariant((int)DataCheckState::ListItem);
                                       indexSeqData << QVariant(QString());
                                       indexSeqData << QVariant(QString());
                                       QStringList Keys(datakeyslist);
                                       if (Keys.last().compare("-")==0)
                                           Keys.removeLast();
                                       indexSeqData << QVariant(true);
                                       indexSeqData << QVariant();
                                       indexSeqData << QVariant();
                                       indexSeqData << QVariant();
                                       QStringList keystrlist(Keys);
                                       keystrlist.prepend(schemaName);
                                       if (!schemakeyslist.startsWith(schemaName) )
                                           schemakeyslist.prepend(schemaName);
                                       indexSeqData << QVariant(schemakeyslist);
                                       indexSeqData << QVariant(false);
                                       indexSeqData << QVariant(0);
                                       indexSeqData << QVariant();
                                       indexSeqData << QVariant();
                                       parents.last()->appendChild(new ConnectDataItem(indexSeqData, mItemIDCount++, parents.last()));

                                       if (mmit->second[kk].Type()==YAML::NodeType::Scalar || mmit->second[kk].Type()==YAML::NodeType::Null) {
                                           parents << parents.last()->child(parents.last()->childCount()-1);
                                            QList<QVariant> indexScalarData;
                                           indexScalarData << (mmit->second[kk].Type()==YAML::NodeType::Scalar
                                                               ? QVariant( QString::fromStdString(mmit->second[kk].as<std::string>()) )
                                                               : QVariant( "null" ) );
                                           indexScalarData << ""; // TODO
                                           indexScalarData << QVariant((int)DataCheckState::ElementKey);
//                                           QStringList dataKeysforTypes(datakeyslist);
//                                           dataKeysforTypes.removeLast();
                                           indexScalarData << (schema ? QVariant(schema->getTypeAsStringList(datakeyslist.join(":")).join(",")) : QVariant());
                                           indexScalarData << (schema ? QVariant(schema->getAllowedValueAsStringList(datakeyslist.join(":")).join(",")) : QVariant());
                                           indexScalarData << QVariant();
                                           indexScalarData << QVariant();
                                           indexScalarData << QVariant();
                                           indexScalarData << QVariant();
                                           indexScalarData << QVariant(schemakeyslist);
                                           indexScalarData << (schema ? (schema->contains(datakeyslist.join(":")) ? QVariant(false) : QVariant(true))
                                                                                                                      : QVariant(true));
                                           indexScalarData << QVariant(0);
                                           QStringList excluded = schema->getExcludedKeys(datakeyslist.join(":"));
                                           seqSeqData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                                                 : QVariant());
                                           indexScalarData << (schema ? (schema->getDefaultValue(datakeyslist.join(":")))
                                                                      : QVariant());
                                           ConnectDataItem* item = new ConnectDataItem(indexScalarData, mItemIDCount++, parents.last());
                                           updateInvaldItem((int)DataItemColumn::Key, item);

                                           parents.last()->appendChild(item);
//                                           updateInvalidExcludedItem(item);
                                           parents.pop_back();
                                        } // TODO: else

                                   }
                                   updateInvalidExcludedItem(item);

                                   QList<QVariant> indexSeqDummyData;
                                   indexSeqDummyData << keystr;
                                   indexSeqDummyData << "";
                                   indexSeqDummyData << QVariant((int)DataCheckState::ListAppend);
                                   QStringList keys(datakeyslist);
                                   keys.insert(0,schemaName);
                                   indexSeqDummyData << QVariant(QStringList());
                                   indexSeqDummyData << QVariant();
                                   indexSeqDummyData << QVariant();
                                   indexSeqDummyData << QVariant();
                                   indexSeqDummyData << QVariant();
                                   indexSeqDummyData << QVariant();
                                   indexSeqDummyData << QVariant(keys);
                                   indexSeqDummyData << (schema ? QVariant(false) : QVariant(true));
                                   indexSeqDummyData << QVariant(0);
                                   indexSeqDummyData << QVariant();
                                   indexSeqDummyData << QVariant();
                                   parents.last()->appendChild(new ConnectDataItem(indexSeqDummyData, mItemIDCount++, parents.last()));

                                   parents.pop_back();

                               } else if (mmit->second.Type()==YAML::NodeType::Map) {
                                   QList<QVariant> mapData;
                                   mapData << (keystr.contains("[") ? key.left(keystr.lastIndexOf("[")) : keystr);
                                   mapData << "";
                                   mapData << QVariant((int)DataCheckState::KeyItem);
                                   mapData << (schema ? QVariant(schema->getTypeAsStringList(datakeyslist.join(":")).join(",")) : QVariant());
                                   mapData << (schema ? QVariant(schema->getAllowedValueAsStringList(datakeyslist.join(":")).join(",")) : QVariant());
                                   mapData << (schema ? QVariant(!schema->isRequired(datakeyslist.join(":"))) : QVariant());
                                   mapData << QVariant();
                                   mapData << QVariant();
                                   mapData << QVariant();
                                   mapData << QVariant(schemakeyslist);
                                   mapData << (schema ? (schema->contains(datakeyslist.join(":")) ? QVariant(false) : QVariant(true))
                                                      : QVariant(true));
                                   mapData << QVariant(0);
                                   QStringList excluded = schema->getExcludedKeys(datakeyslist.join(":"));
                                   mapData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                                      : QVariant());
                                   mapData << (schema ? (schema->getDefaultValue(datakeyslist.join(":")))
                                                      : QVariant());
                                   parents.last()->appendChild(new ConnectDataItem(mapData, mItemIDCount++, parents.last()));

                                   parents << parents.last()->child(parents.last()->childCount()-1);
                                   const YAML::Node mapmapnode = mmit->second;
                                          for (YAML::const_iterator mmmit = mapmapnode.begin(); mmmit != mapmapnode.end(); ++mmmit) {
                                               QList<QVariant> mapSeqData;
                                               mapSeqData << mmmit->first.as<std::string>().c_str();
                                               mapSeqData << mmmit->second.as<std::string>().c_str();  // can be int/bool/double
                                               mapSeqData << QVariant((int)DataCheckState::ElementMap);
                                               mapSeqData << (schema ? QVariant(schema->getTypeAsStringList(dataKeys.join(":")).join(",")): QVariant());
                                               mapSeqData << (schema ? QVariant(schema->getAllowedValueAsStringList(dataKeys.join(":")).join(",")): QVariant());
                                               mapSeqData << (schema ? QVariant(!schema->isRequired(dataKeys.join(":"))): QVariant());
                                               mapSeqData << QVariant();
                                               mapSeqData << QVariant();
                                               mapSeqData << QVariant();
                                               mapSeqData << QVariant(QStringList());
                                               mapSeqData << (schema ? (schema->contains(dataKeys.join(":")) ? QVariant(false) : QVariant(true))
                                                                     : QVariant(true));
                                               mapSeqData << QVariant(0);
                                               QStringList excluded = schema->getExcludedKeys(dataKeys.join(":"));
                                               mapSeqData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                                                     : QVariant());
                                               mapSeqData << (schema ? (schema->getDefaultValue(dataKeys.join(":")))
                                                                     : QVariant());
                                               ConnectDataItem* item = new ConnectDataItem(mapSeqData, mItemIDCount++, parents.last());
                                               if (!updateInvaldItem((int)DataItemColumn::Key, item))
                                                   updateInvaldItem((int)DataItemColumn::Value, item);
                                               parents.last()->appendChild(new ConnectDataItem(mapSeqData, mItemIDCount++, parents.last()));
//                                                       dataKeys.removeLast();
                                          }
                                          updateInvalidExcludedItem(item);
                                          QList<QVariant> indexSeqDummyData;
                                          indexSeqDummyData << keystr;
                                          indexSeqDummyData << "";
                                          indexSeqDummyData << QVariant((int)DataCheckState::MapAppend);
                                          indexSeqDummyData << QVariant(QString());
                                          indexSeqDummyData << QVariant(QString());
                                          indexSeqDummyData << QVariant();
                                          indexSeqDummyData << QVariant();
                                          indexSeqDummyData << QVariant();
                                          indexSeqDummyData << QVariant();
                                          indexSeqDummyData << QVariant(QStringList());
                                          indexSeqDummyData << (schema ? QVariant(false) : QVariant(true));
                                          indexSeqDummyData << QVariant(0);
                                          indexSeqDummyData << QVariant();
                                          indexSeqDummyData << QVariant();
                                          parents.last()->appendChild(new ConnectDataItem(indexSeqDummyData, mItemIDCount++, parents.last()));

                                          parents.pop_back();

                               } else if (mmit->second.Type()==YAML::NodeType::Scalar || mmit->second.Type()==YAML::NodeType::Null) {
                                     QList<QVariant> mapSeqData;
                                     if (schema->isOneOfDefined(datakeyslist.join(":"))) {
                                         int n = whichOneOfSchema(mit->second, schema, datakeyslist, keystr);
                                         keystr += QString("[%1]").arg(n);
                                         datakeyslist.removeLast();
                                         datakeyslist << keystr;
                                     } else if (schema->isAnyOfDefined(datakeyslist.join(":"))) {
                                         int n = whichAnyOfSchema(mit->second, schema, datakeyslist, keystr);
                                         keystr += QString("[%1]").arg(n);
                                         datakeyslist.removeLast();
                                         datakeyslist << keystr;
                                     }
                                     int column = (int)DataItemColumn::Key;
                                     QStringList typelist = schema->getTypeAsStringList(datakeyslist.join(":"));
                                     QVariant defvalue = schema->getDefaultValue(datakeyslist.join(":"));
                                     schemaKeys << keystr;
                                     mapSeqData << (keystr.contains("[") ? keystr.left(keystr.lastIndexOf("[")) : keystr);
                                     if (typelist.contains("dict") || typelist.contains("list")) {
                                         mapSeqData << QVariant();
                                         mapSeqData << QVariant( (int)DataCheckState::KeyItem);
                                         column = (int)DataItemColumn::Key;
                                     } else {
                                         mapSeqData << ( mmit->second.Type()==YAML::NodeType::Scalar ? QVariant( QString::fromStdString(mmit->second.as<std::string>()) )
                                                                                                     : (mmit->second.Type()==YAML::NodeType::Null ? QVariant("null")
                                                                                                                                                  : (defvalue.toString().isEmpty() ? QVariant("[value]")
                                                                                                                                                                                   : QVariant(defvalue.toString())))
                                                        );
                                         mapSeqData << QVariant((int)DataCheckState::ElementValue);
                                         column = (int)DataItemColumn::Value;
                                     }
                                     mapSeqData << (schema ? QVariant(schema->getTypeAsStringList(datakeyslist.join(":")).join(",")): QVariant());
                                     mapSeqData << (schema ? QVariant(schema->getAllowedValueAsStringList(datakeyslist.join(":")).join(",")): QVariant());
                                     bool required =  schema->isRequired(datakeyslist.join(":"));
                                     if (key.endsWith("]")) {
                                         if (required) {
                                            mapSeqData << (!required); // delete able
                                         } else { // take rquire flag from parent schema
                                            QStringList skeylist(dataKeys);
                                            skeylist.removeLast();
                                            skeylist << key.left(key.lastIndexOf("["));
                                            mapSeqData << (schema ? QVariant(!schema->isRequired(skeylist.join(":"))) : QVariant());
                                         }
                                     } else {
                                         mapSeqData << (schema ? QVariant(!required) : QVariant(false));
                                     }
                                     mapSeqData << QVariant();
                                     mapSeqData << QVariant();
                                     mapSeqData << QVariant();
                                     mapSeqData << QVariant(schemaKeys);
                                     mapSeqData << (schema ? (schema->contains(datakeyslist.join(":")) ? QVariant(false) : QVariant(true))
                                                           : QVariant(true));
                                     mapSeqData << QVariant(0);
                                     QStringList excluded = schema->getExcludedKeys(datakeyslist.join(":"));
                                     mapSeqData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                                           : QVariant());
                                     mapSeqData << (schema ? (schema->getDefaultValue(datakeyslist.join(":")))
                                                           : QVariant());
                                     ConnectDataItem* item = new ConnectDataItem(mapSeqData, mItemIDCount++, parents.last());

                                     if (!isIndexValueValid(column, item)) {
                                         item->setData((int)DataItemColumn::InvalidValue, 1);
                                         updateInvaldItem(column, item);
                                     }
                                     if (position>=parents.last()->childCount() || position < 0) {
                                         parents.last()->appendChild(item);
                                         if (typelist.contains("dict") || typelist.contains("list"))
                                             parents << parents.last()->child(parents.last()->childCount()-1);
                                     } else {
                                         parents.last()->insertChild(position, item);
                                         if (typelist.contains("dict") || typelist.contains("list"))
                                             parents << parents.last()->child(position);
                                     }
                                     updateInvalidExcludedItem(item);
                                     if (typelist.contains("dict") || typelist.contains("list")) {
                                         QList<QVariant> sequenceDummyData;

                                         sequenceDummyData << (keystr.contains("[") ? keystr.left(keystr.lastIndexOf("[")) : keystr);
                                         sequenceDummyData << "";
                                         sequenceDummyData << QVariant((int)DataCheckState::MapAppend);
                                         sequenceDummyData << QVariant(QString());
                                         sequenceDummyData << QVariant(QString());
                                         sequenceDummyData << QVariant(false);
                                         sequenceDummyData << QVariant();
                                         sequenceDummyData << QVariant();
                                         sequenceDummyData << QVariant();
                                         QStringList keys(datakeyslist);
                                         keys.insert(0,schemaName);
                                         sequenceDummyData << QVariant(keys);
                                         sequenceDummyData << (schema ? QVariant(false) : QVariant(true));
                                         sequenceDummyData << QVariant(0);
                                         sequenceDummyData << QVariant();
                                         sequenceDummyData << QVariant();
                                         parents.last()->appendChild(new ConnectDataItem(sequenceDummyData, mItemIDCount++, parents.last()));
                                         parents.pop_back();
                                     }
                                     schemaKeys.removeLast();
                               }
                               updateInvalidExcludedItem(item);
                               datakeyslist.removeLast();
                               schemakeyslist.removeLast();
                               for (int i=datakeyslist.size(); i>before; i--) {
                                     datakeyslist.removeLast();
                                     schemakeyslist.removeLast();
                               }
                           }
                           updateInvaldItem((int)DataItemColumn::Key, item);
                           parents.pop_back();
                           if (isOneofDefined) {
                               datakeyslist.removeLast();
                               datakeyslist.removeLast();
                               schemakeyslist.removeLast();
                               schemakeyslist.removeLast();
                           }

                 } else if (mit->second[k].Type()==YAML::NodeType::Scalar) {
                                   parents << parents.last()->child(parents.last()->childCount()-1);
                                   QStringList dataKeysforTypes(datakeyslist);
//                                   if (dataKeysforTypes.size() > 1)
//                                       dataKeysforTypes.removeLast();
                                   QVariant defvalue = schema->getDefaultValue(dataKeysforTypes.join(":"));
                                   QList<QVariant> mapSeqData;
                                   mapSeqData << QVariant( QString::fromStdString(mit->second[k].as<std::string>()) );
                                   mapSeqData << "";
                                   mapSeqData << QVariant((int)DataCheckState::ElementKey);
                                   mapSeqData << (schema ? QVariant(schema->getTypeAsStringList(dataKeysforTypes.join(":")).join(",")) : QVariant());
                                   mapSeqData << (schema ? QVariant(schema->getAllowedValueAsStringList(dataKeysforTypes.join(":")).join(",")): QVariant());
                                   mapSeqData << QVariant();
                                   mapSeqData << QVariant();
                                   mapSeqData << QVariant();
                                   mapSeqData << QVariant();
                                   mapSeqData << QVariant(schemakeyslist);
                                   mapSeqData << (schema ? (schema->contains(dataKeysforTypes.join(":")) ? QVariant(false) : QVariant(true))
                                                                                                         : QVariant(true));
                                   mapSeqData << QVariant(0);
                                   QStringList excluded = schema->getExcludedKeys(dataKeysforTypes.join(":"));
                                   mapSeqData << (schema ? (excluded.isEmpty() ? QVariant() : QVariant(excluded.join(",")))
                                                         : QVariant());
                                   mapSeqData << (schema ? (defvalue.isValid() ? defvalue : QVariant())
                                                         : QVariant());
                                   ConnectDataItem* item = new ConnectDataItem(mapSeqData, mItemIDCount++, parents.last());
                                   updateInvaldItem((int)DataItemColumn::Key, item);
                                   parents.last()->appendChild(item);
                                   parents.pop_back();
                                   if (isOneofDefined) {
//                                       datakeyslist.removeLast();
                                       datakeyslist.removeLast();
                                       schemakeyslist.removeLast();
//                                       schemakeyslist.removeLast();
                                   }
                 }
             }
             updateInvalidExcludedItem(item);
             dataKeys.removeLast();
             schemaKeys.removeLast();
              QList<QVariant> sequenceDummyData;
              sequenceDummyData << /*(isOneofDefined ? QVariant(mapToSequenceKey + "[0]") : */ QVariant(mapToSequenceKey) /*)*/;
              sequenceDummyData << QVariant();
              sequenceDummyData << QVariant(/*isOneofDefined ? (int)DataCheckState::SchemaAppend : */(int)DataCheckState::ListAppend);
              QStringList keys(dataKeys);
              keys.insert(0,schemaName);
              sequenceDummyData << QVariant(QString());
              sequenceDummyData << QVariant();
              sequenceDummyData << QVariant();
              sequenceDummyData << QVariant();
              sequenceDummyData << QVariant();
              sequenceDummyData << QVariant();
              sequenceDummyData << QVariant(keys);
              sequenceDummyData << (schema ? QVariant(false) : QVariant(true));
              sequenceDummyData << QVariant(0);
              sequenceDummyData << QVariant();
              sequenceDummyData << QVariant();
              parents.last()->appendChild(new ConnectDataItem(sequenceDummyData, mItemIDCount++, parents.last()));
              parents.pop_back();
              if (!dataKeys.isEmpty())
                  dataKeys.removeLast();
              if (!schemaKeys.isEmpty())
                  schemaKeys.removeLast();
        }
         ConnectDataItem* item = parents.last();
         updateInvaldItem((int)DataItemColumn::Key, item);
    }
    if (data) {
        delete data;
        data = NULL;
    }
}

void ConnectDataModel::updateInvalidExcludedItem(ConnectDataItem *item)
{
    ConnectDataItem* schemaitem = getSchemaParentItem(item);
    ConnectDataItem *parentitem = (schemaitem==mRootItem ? item : item->parentItem());
    if (parentitem->childCount() > 1) {
        QStringList childrenNameList;
        for (int i=0; i<parentitem->childCount(); ++i) {
            if (parentitem->child(i)->data((int)DataItemColumn::ExcludedKeys).toStringList().isEmpty())
                continue;
            if ( parentitem->child(i)->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::ListAppend ||
                 parentitem->child(i)->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::MapAppend   ||
                 parentitem->child(i)->data((int)DataItemColumn::CheckState).toInt()==(int)DataCheckState::MapSchemaAppend )
                continue;
            childrenNameList << parentitem->child(i)->data((int)DataItemColumn::Key).toString();
        }
        int parentvalue = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
        int addedvalue = 0;
        for (int i=0; i<parentitem->childCount(); ++i) {
            ConnectDataItem *ditem = parentitem->child(i);
            QStringList levelExcludedKeyList = ditem->data((int)DataItemColumn::ExcludedKeys).toString().split(",");
            for (const QString &k : levelExcludedKeyList) {
                if (!childrenNameList.contains(k))
                    continue;
                int value = ditem->data((int)DataItemColumn::InvalidValue).toInt();
                if (value > 0) {
                    ditem->setData((int)DataItemColumn::InvalidValue, QVariant(value));
                } else {
                     ditem->setData((int)DataItemColumn::InvalidValue, QVariant(value+1));
                    ++addedvalue;
                }
                break;
            }
        }
        if (addedvalue > 0) {
            parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue+addedvalue));
            ConnectDataItem* pitem = parentitem->parentItem();
            while(pitem && pitem != schemaitem) {
                int value = pitem->data((int)DataItemColumn::InvalidValue).toInt();
                pitem->setData((int)DataItemColumn::InvalidValue, QVariant(addedvalue+value));
                parentvalue = pitem->data((int)DataItemColumn::InvalidValue).toInt();
                pitem = pitem->parentItem();
            }
            if (schemaitem==pitem) {
                int value = schemaitem->data((int)DataItemColumn::InvalidValue).toInt();
                schemaitem->setData((int)DataItemColumn::InvalidValue, QVariant(addedvalue+value));
            }
        }
    }
}

bool ConnectDataModel::updateInvaldItem(int column, ConnectDataItem *item)
{
    bool valid = isIndexValueValid(column, item);
    if (!valid) {
        int value = item->data((int)DataItemColumn::InvalidValue).toInt();
        ConnectDataItem* schemaitem = getSchemaParentItem(item);
        ConnectDataItem* parentitem = item->parentItem();
        while(parentitem && parentitem != schemaitem && parentitem != mRootItem) {
            int parentvalue = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
            parentitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue+value));
            parentitem = parentitem->parentItem();
        }
        if (schemaitem==parentitem) {
            int parentvalue = parentitem->data((int)DataItemColumn::InvalidValue).toInt();
            schemaitem->setData((int)DataItemColumn::InvalidValue, QVariant(parentvalue+value));
        }
    }
    return valid;
}

} // namespace connect
} // namespace studio
} // namespace gams
