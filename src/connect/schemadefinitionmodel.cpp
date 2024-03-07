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
#include <QMimeData>
#include <QFile>
#include <QRegularExpression>

#include "schemadefinitionmodel.h"
#include "theme.h"

namespace gams {
namespace studio {
namespace connect {

static const QRegularExpression cRex("^\\[\\d\\d?\\]$");

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
    case Qt::UserRole: {
        SchemaDefinitionItem* item = static_cast<SchemaDefinitionItem*>(index.internalPointer());
        if (index.column()==(int)SchemaItemColumn::SchemaKey)
            return QVariant(item->data(index.column()).toStringList().join(":"));
        else
            return QVariant();
    }
    case Qt::DisplayRole: {
        SchemaDefinitionItem* item = static_cast<SchemaDefinitionItem*>(index.internalPointer());
        if (index.column()==(int)SchemaItemColumn::SchemaKey)
            return QVariant(item->data(index.column()).toStringList().join(":"));
        if (index.column()<=item->columnCount()-1) {
            return item->data(index.column());
        } else {
            return QVariant();
        }
    }
    case Qt::ForegroundRole: {
        SchemaDefinitionItem* item = static_cast<SchemaDefinitionItem*>(index.internalPointer());
        if (index.column()==(int)SchemaItemColumn::Type &&
            item->data((int)SchemaItemColumn::Type).toString().compare("anyof")==0)
            return  QVariant::fromValue(Theme::color(Theme::Active_Gray));
        else if (index.column()==(int)SchemaItemColumn::Type &&
                 item->data((int)SchemaItemColumn::Type).toString().compare("oneof")==0)
                  return  QVariant::fromValue(Theme::color(Theme::Active_Gray));
        else if (index.column()==(int)SchemaItemColumn::Field &&
                 item->data(index.column()).toString().compare("schema")==0)
                  return  QVariant::fromValue(Theme::color(Theme::Disable_Gray));
        else if (index.column()==(int)SchemaItemColumn::Field                      &&
                 item->data((int)SchemaItemColumn::Field).toString().contains("[") &&
                 item->data((int)SchemaItemColumn::Field).toString().contains("]")    )
            return  QVariant::fromValue(Theme::color(Theme::Active_Gray));
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
        QModelIndex idx = index.sibling(index.row(), (int)SchemaItemColumn::Field);
        SchemaDefinitionItem* item = static_cast<SchemaDefinitionItem*>(idx.internalPointer());
        if (item->data((int)SchemaItemColumn::Field).toString().compare("schema", Qt::CaseInsensitive)!=0)
            return (
                QString("<html><head/><body>Drag and drop <span style=' font-weight:600;'>%1</span>  to insert the attribute from definition.</body></html>")
                       .arg(item->data((int)SchemaItemColumn::Field).toString())
            );
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
    else if (!index.sibling(index.row(), (int)SchemaItemColumn::DragEnabled).data(Qt::DisplayRole).toBool())
        return defaultFlags;
    else
        return Qt::ItemIsDragEnabled | defaultFlags;
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
    QDataStream stream(&encodedData, QFile::WriteOnly);

    for (const QModelIndex &index : indexes) {
        const QModelIndex sibling = index.sibling(index.row(), (int)SchemaItemColumn::SchemaKey);
        const QString text = QString("schema=%1").arg(sibling.data(Qt::UserRole).toString());
        stream << text;
        break;
    }
    mimeData->setData( "application/vnd.gams-connect.text", encodedData);
    return mimeData;
}

void SchemaDefinitionModel::loadSchemaFromName(const QString &name)
{
    if (!mRootItems.contains(name))
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
           data << QString("%1").arg(schemaTypeList.join(","));
    } else {
        data << "";
    }
}

void SchemaDefinitionModel::addValueList(QList<ValueWrapper> &valueList, QList<QVariant> &data)
{
    QStringList valueStrList;
    for(int i=0; i<valueList.size(); ++i) {
        if (valueList.at(i).type==SchemaValueType::NoValue) {
            valueStrList << "";
        } else if (valueList.at(i).type==SchemaValueType::Integer) {
            valueStrList << QString::number(valueList.at(i).value.intval);
        } else if (valueList.at(i).type==SchemaValueType::Float) {
            valueStrList << QString::number(valueList.at(i).value.doubleval);
        } else if (valueList.at(i).type==SchemaValueType::String) {
            valueStrList << QString(valueList.at(i).value.stringval);
        } else if (valueList.at(i).type==SchemaValueType::Boolean) {
            valueStrList << QString(valueList.at(i).value.boolval ? "1" : "0");
        } else  {
            valueStrList << "";
        }
    }
    if (valueList.size() > 0) {
        if (valueList.size() == 1)
           data << QString("%1").arg(valueStrList.at(0));
       else
           data << QString("%1").arg(valueStrList.join(","));
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
               if (strcmp(value.value.stringval, "null") == 0)
                  data << QString(value.value.stringval);
               else
                  data << QString(value.value.stringval);
    } else if (value.type==SchemaValueType::Boolean) {
        data << value.value.boolval;
    } else  {
        data << "";
    }
}

QString SchemaDefinitionModel::getValue(ValueWrapper &value)
{
    if (value.type==SchemaValueType::Integer || value.type==SchemaValueType::Float) {
        return QString::number(value.value.intval);
    } else if (value.type==SchemaValueType::String) {
              return QString::fromLatin1(value.value.stringval);
    } else if (value.type==SchemaValueType::Boolean) {
               return (value.value.boolval ? "true": "false");
    }
    return "";
}

QStringList SchemaDefinitionModel::gettAllAllowedValues(Schema *schemaHelper)
{
    QList<ValueWrapper> allvalues(schemaHelper->allowedValues);
    QStringList valueStrList;
    for(int i=0; i<allvalues.size(); ++i) {
        const QString value = getValue(allvalues[i]);
        if (!value.isEmpty())
            valueStrList << value;
    }
    const QString minvalue = getValue(schemaHelper->min);
    if (!minvalue.isEmpty()) {
        valueStrList << minvalue << "..";
    }
    const QString maxvalue = getValue(schemaHelper->max);
    if (!maxvalue.isEmpty()) {
        valueStrList << maxvalue;
    } else {
        if (schemaHelper->min.type==SchemaValueType::Integer)
           valueStrList << QString::number(std::numeric_limits<int>::max());
    }
    return valueStrList;
}

void SchemaDefinitionModel::setupTreeItemModelData()
{
    QList<QVariant> rootData;
    rootData << "Option" << "Required"  << "Type" << "Nullable" <<  "Default"
             << "Allowed Values"  << "min" /*<< "max"*/ << "SchemaKey" << "DragEnabled" << "Excludes" ;

    for (const QString& schemaName : mConnect->getSchemaNames()) {
        SchemaDefinitionItem* rootItem = new SchemaDefinitionItem(schemaName, rootData);
        mRootItems[schemaName] = rootItem;

        QList<SchemaDefinitionItem*> parents;
        parents << rootItem;

        ConnectSchema* schema = mConnect->getSchema(schemaName);
        QStringList schemaKeys;
        schemaKeys << schemaName;
        QStringList oneOfDefinedKeys, anyOfDefinedKeys;
        for (const QString& key : schema->getFirstLevelKeyList()) {
            if (schemaKeys.contains(key) || oneOfDefinedKeys.contains(key) || anyOfDefinedKeys.contains(key))
                continue;
            QList<QVariant> columnData;
            Schema* s = schema->getSchema(key);
            if (s) {
                if (s && (s->isOneOf || s->isAnyOf)) {
                    if (s->isOneOf && !oneOfDefinedKeys.contains(key))
                        oneOfDefinedKeys << key;
                    if (s->isAnyOf && !anyOfDefinedKeys.contains(key))
                        anyOfDefinedKeys << key;
                    schemaKeys << key;
                    columnData << key;
                    columnData << (s->required?"Y":"");
                    columnData << (s->isOneOf ? "oneof" : (s->isAnyOf ? "anyof" : "")) ;
                    columnData << (s->nullable?"Y":"");
                    addValue(s->defaultValue, columnData);
                    QStringList strlist = gettAllAllowedValues(s);
                    columnData << (strlist.isEmpty() ? "" : strlist.join(","));
                    columnData << "";
                    columnData << QVariant(schemaKeys);
                    columnData << (s->isOneOf || s->isAnyOf ? QVariant(false) : QVariant(true));
                    columnData << (s ? s->excludes : QStringList());
                    if (s && !s->excludes.isEmpty())
                        columnData << s->excludes.join(",");
                    else
                        columnData << "";
                    columnData << (s ? QVariant::fromValue(s->defaultValue.value.stringval) : QVariant() );
                    SchemaDefinitionItem* item = new SchemaDefinitionItem(schemaName, columnData, parents.last());
                    parents.last()->appendChild(item);
                    if (s->isOneOf || s->isAnyOf) {
                        for (int i =0; i<schema->getNumberOfOneOfDefined(key); ++i) {
                            QString keystr = QString("%1[%2]").arg(key).arg(i);
                            schemaKeys.removeLast();
                            schemaKeys << keystr;
                            setupOneofAnyofSchemaTree(schemaName, keystr, schemaKeys, parents, schema);
                        }
                    }
                } else {
                    schemaKeys << key;
                    columnData << key;
                    columnData << (s->required?"Y":"");
                    addTypeList(s->types, columnData);
                    columnData << (s->nullable?"Y":"");
                    addValue(s->defaultValue, columnData);
                    QStringList strlist = gettAllAllowedValues(s);
                    if (strlist.isEmpty())
                        columnData << "";
                    else
                        columnData << strlist.join(",");
                    columnData << "";
                    columnData << QVariant(schemaKeys);
                    columnData << QVariant(true);
                    if (s && !s->excludes.isEmpty())
                        columnData << s->excludes.join(",");
                    else
                        columnData << "";
                    columnData << (s ? QVariant::fromValue(s->defaultValue.value.stringval) : QVariant() );
                    SchemaDefinitionItem* item = new SchemaDefinitionItem(schemaName, columnData, parents.last());
                    parents.last()->appendChild(item);

                    if (s->schemaDefined) {
                        setupSchemaTree(schemaName, key, schemaKeys, parents, schema);
                    }
                }
                schemaKeys.removeLast();
            }
        }
    }
}

void SchemaDefinitionModel::setupOneofAnyofSchemaTree(const QString &schemaName, const QString &key, QStringList &schemaKeys, QList<SchemaDefinitionItem *> &parents, ConnectSchema *schema)
{
    Schema* schemaHelper = schema->getSchema(key);
    if (schemaHelper) {
        QList<QVariant> listData;
        listData << (key.contains(":") ? key.mid(key.lastIndexOf(":")+1) : key) ;
        listData << (schemaHelper->required?"Y":"");
        addTypeList(schemaHelper->types, listData);
        listData << (schemaHelper->nullable?"Y":"");
        addValue(schemaHelper->defaultValue, listData);
        const QStringList strlist = gettAllAllowedValues(schemaHelper);
        if (strlist.isEmpty())
            listData << "";
        else
            listData << strlist.join(",");
        listData << "";
        listData << QVariant(schemaKeys);
        listData << (schemaHelper->isOneOf || schemaHelper->isAnyOf ? QVariant(false) : QVariant(true));
        listData << (!schemaHelper->excludes.isEmpty() ? schemaHelper->excludes.join(",") : "");
        listData << QVariant::fromValue(schemaHelper->defaultValue.value.stringval);
        parents << parents.last()->child(parents.last()->childCount()-1);
        SchemaDefinitionItem* item = new SchemaDefinitionItem(schemaName, listData, parents.last());
        parents.last()->appendChild(item);

        if (schemaHelper->schemaDefined) {
            setupSchemaTree(schemaName, key, schemaKeys, parents, schema);
        }

        parents.pop_back();
    } else {
        QStringList sk(schemaKeys);
        sk.removeFirst();
        schemaHelper = schema->getSchema(sk.join(":"));
        if (schemaHelper) {
            QList<QVariant> listData;
            listData << (key.contains(":") ? key.mid(key.lastIndexOf(":")+1) : key) ;
            listData << (schemaHelper->required?"Y":"");
            addTypeList(schemaHelper->types, listData);
            listData << (schemaHelper->nullable?"Y":"");
            addValue(schemaHelper->defaultValue, listData);
            QStringList strlist = gettAllAllowedValues(schemaHelper);
            if (strlist.isEmpty())
                listData << "";
            else
                listData << strlist.join(",");
            listData << "";
            listData << QVariant(schemaKeys);
            listData << (schemaHelper->isOneOf || schemaHelper->isAnyOf ? QVariant(false) : QVariant(true));
            listData << (!schemaHelper->excludes.isEmpty() ? schemaHelper->excludes.join(",") : "");
            listData << QVariant::fromValue(schemaHelper->defaultValue.value.stringval);
            parents << parents.last()->child(parents.last()->childCount()-1);
            SchemaDefinitionItem* item = new SchemaDefinitionItem(schemaName, listData, parents.last());
            parents.last()->appendChild(item);
            if (schemaHelper->schemaDefined) {
                setupSchemaTree(schemaName, key, schemaKeys, parents, schema);
            }
            parents.pop_back();
        }
    }
    if (schemaKeys.last().compare("-")==0)
        schemaKeys.removeLast();
}

void SchemaDefinitionModel::setupSchemaTree(const QString& schemaName, const QString& key,
                                            QStringList& schemaKeys, QList<SchemaDefinitionItem*>& parents, ConnectSchema* schema) {

    QString prefix = key;
    Schema* s = schema->getSchema(key);
    if (!s) {
        QStringList sk(schemaKeys);
        sk.removeFirst();
        s = schema->getSchema(sk.join(":"));
    }
    if (s->hasType(SchemaType::List)) {
        prefix += ":-";
        schemaKeys << "-";
    }
    Schema* schemaHelper = schema->getSchema(prefix);
    if (!schemaHelper) {
        QStringList sk(schemaKeys);
        sk.removeFirst();
        schemaHelper = schema->getSchema(sk.join(":"));
    }
    if (schemaHelper) {
        bool differentSchemaKeys = (QString::compare(key,prefix, Qt::CaseInsensitive)!=0);
        parents << parents.last()->child(parents.last()->childCount()-1);
        QList<QVariant> listData;
        listData << "schema";
        listData << (differentSchemaKeys ? (schemaHelper->required?"Y":"") : QVariant());
        if (differentSchemaKeys)
            addTypeList(schemaHelper->types, listData);
        else
            listData << QVariant();
        listData << (differentSchemaKeys ? (schemaHelper->nullable?"Y":"") : QVariant());
        if (differentSchemaKeys)
            addValue(schemaHelper->defaultValue, listData);
        else
            listData << QVariant();
        QStringList strlist = gettAllAllowedValues(schemaHelper);
        if (strlist.isEmpty())
            listData << "";
        else
            listData << strlist.join(",");
        listData << "";
        listData << QVariant(schemaKeys);
        listData << QVariant(false);
        listData << (differentSchemaKeys ? (!schemaHelper->excludes.isEmpty() ? schemaHelper->excludes.join(",") : QVariant()) : QVariant());
        listData << (differentSchemaKeys ? QVariant::fromValue(schemaHelper->defaultValue.value.stringval) : QVariant());
        parents.last()->appendChild(new SchemaDefinitionItem(schemaName, listData, parents.last()));
        const QStringList nextlevelList = schema->getNextLevelKeyList(prefix);
        if (!nextlevelList.isEmpty()) {
            parents << parents.last()->child(parents.last()->childCount()-1);
            for (const QString& k :  nextlevelList) {
                QStringList schemaDataKeys(schemaKeys);
                schemaHelper = schema->getSchema(k);
                const QString schemaKeyStr = k.mid(prefix.length()+1);
                if (k.endsWith(":-")) {
                   setupSchemaTree(schemaName, k.left(k.lastIndexOf(":")), schemaDataKeys, parents, schema);
                } else if (k.endsWith("]")) {
                          schemaDataKeys << schemaKeyStr;
                          QList<QVariant> data;
                          data <<  schemaKeyStr;
                          data << (schemaHelper->required?"Y":"");
                          if (schemaHelper->isOneOf)
                              data << "oneof";
                          else if (schemaHelper->isAnyOf)
                                   data << "anyof";
                          else
                               addTypeList(schemaHelper->types, data);
                          data << (schemaHelper->nullable?"Y":"");
                          addValue(schemaHelper->defaultValue, data);
                          const QStringList strlist = gettAllAllowedValues(schemaHelper);
                          if (strlist.isEmpty())
                              data << "";
                          else
                              data << strlist.join(",");
                          data << "";
                          data << QVariant(schemaDataKeys);
                          data << (schema->isOneOfDefined(k) || schema->isOneOfDefined(k) ? QVariant(false)
                                                                                          : QVariant(true));
                          data << (!schemaHelper->excludes.isEmpty() ? schemaHelper->excludes.join(",") : "");
                          data << QVariant::fromValue(schemaHelper->defaultValue.value.stringval);
                          parents.last()->appendChild(new SchemaDefinitionItem(schemaName, data, parents.last()));

                          QString anyofPrefix = "";
                          foreach(const QString& kk, schema->getNextLevelKeyList(k)) {
                              QStringList dataKeys(schemaDataKeys);
                              parents << parents.last()->child(parents.last()->childCount()-1);
                              if (schema->isOneOfDefined(kk) || schema->isAnyOfDefined(kk)) {
                                  anyofPrefix = kk;
                                  QList<QVariant> columnData;
                                  dataKeys << kk.mid(kk.lastIndexOf(":")+1);
                                  columnData << kk.mid(kk.lastIndexOf(":")+1);
                                  columnData << "";
                                  columnData << (schema->isOneOfSchemaDefined(k) ? "oneof"
                                                                                 : (schema->isAnyOfDefined(k) ? "anyof" : ""));
                                  columnData << "";
                                  addValue(schemaHelper->defaultValue, columnData);
                                  columnData << "";
                                  columnData << "";
                                  columnData << QVariant(kk);
                                  columnData << (schema->isOneOfDefined(k) || schema->isOneOfDefined(k) ? QVariant(false)
                                                                                                        : QVariant(true));
                                  columnData << (!schemaHelper->excludes.isEmpty() ? schemaHelper->excludes.join(",") : "");
                                  columnData << QVariant::fromValue(schemaHelper->defaultValue.value.stringval);
                                  SchemaDefinitionItem* item = new SchemaDefinitionItem(schemaName, columnData, parents.last());
                                  parents.last()->appendChild(item);

                                  for (int i =0; i<schema->getNumberOfAnyOfDefined(kk); ++i) {
                                      QString keystr = QString("%1[%2]").arg(kk).arg(i);
                                      dataKeys.removeLast();
                                      dataKeys << keystr.split(":").last(); //keystr;
                                      setupOneofAnyofSchemaTree(schemaName, keystr, dataKeys, parents, schema);
                                  }
                              } /*else if (!kk.startsWith(anyofPrefix)) {
                                  dataKeys << kk.split(":").last();
                                  setupOneofSchemaTree(schemaName, kk, dataKeys, parents, schema);
                              }*/
                              parents.pop_back();
                          }
                } else {
                    schemaDataKeys << schemaKeyStr;
                    if (schema->isOneOfDefined(k) || schema->isAnyOfDefined(k)) {
                        QList<QVariant> columnData;
                        columnData << schemaKeyStr;
                        columnData << (schemaHelper->required?"Y":"");
                        if (schema->isOneOfDefined(k))
                            columnData << "oneof";
                        else if (schema->isAnyOfDefined(k))
                                columnData << "anyof";
                        else
                            addTypeList(schemaHelper->types, columnData);
                        columnData << (schemaHelper->nullable?"Y":"");
                        addValue(schemaHelper->defaultValue, columnData);
                        QStringList strlist = gettAllAllowedValues(schemaHelper);
                        columnData << (strlist.isEmpty() ? "" : strlist.join(","));
                        columnData << "";                         // min
                        columnData << QVariant(schemaDataKeys);   // schemaKey
                        columnData << (schema->isOneOfDefined(k) || schema->isOneOfDefined(k) ? QVariant(false)
                                                                                              : QVariant(true));
                        columnData << (!schemaHelper->excludes.isEmpty() ? schemaHelper->excludes.join(",") : "");
                        columnData << QVariant::fromValue(schemaHelper->defaultValue.value.stringval);
                        SchemaDefinitionItem* item = new SchemaDefinitionItem(schemaName, columnData, parents.last());
                        parents.last()->appendChild(item);
                        for (int i =0; i<schema->getNumberOfAnyOfDefined(schemaKeyStr); ++i) {
                            const QString keystr = QString("%1[%2]").arg(schemaKeyStr).arg(i);
                            if (i>0)
                               schemaKeys.removeLast();
                            schemaKeys << keystr;
                            setupOneofAnyofSchemaTree(schemaName, keystr, schemaKeys, parents, schema);
                        }
                        schemaKeys.removeLast();
                    } else {
                        QList<QVariant> data;
                        data <<  schemaKeyStr;
                        data << (schemaHelper->required?"Y":"");
                        addTypeList(schemaHelper->types, data);
                        data << (schemaHelper->nullable?"Y":"");
                        addValue(schemaHelper->defaultValue, data);
                        const QStringList strlist = gettAllAllowedValues(schemaHelper);
                        data << (strlist.isEmpty() ? "" : strlist.join(","));
                        data << "";
                        data << QVariant(schemaDataKeys);
                        data << (schemaHelper->isOneOf || schemaHelper->isAnyOf ? QVariant(false) : QVariant(true));
                        data << (!schemaHelper->excludes.isEmpty() ? schemaHelper->excludes.join(",") : "");
                        data << QVariant::fromValue(schemaHelper->defaultValue.value.stringval);
                        parents.last()->appendChild(new SchemaDefinitionItem(schemaName, data, parents.last()));
                    }
               }
            }
            parents.pop_back();
        } else {
            QStringList anyOfDefinedKeys;
            QList<QVariant> columnData;
            Schema* ss = schema->getSchema(key);
            if (ss && (ss->isOneOf || ss->isAnyOf)) {
                schemaKeys << key;
                columnData << key;
                columnData << (ss->required?"Y":"");
                if (ss->isOneOf)
                    columnData << "oneof";
                else if (ss->isAnyOf)
                    columnData << "anyof";
                else
                    addTypeList(ss->types, columnData);
                columnData << (ss->nullable?"Y":"");;
                addValue(ss->defaultValue, columnData);
                QStringList strlist = gettAllAllowedValues(ss);
                columnData << (strlist.isEmpty() ? "" : strlist.join(","));
                columnData << "";
                columnData << "";
                columnData << (ss->isOneOf || ss->isAnyOf ? QVariant(false)
                                                          : QVariant(true));
                columnData << (ss ? ss->excludes : QStringList());
                if (ss && !ss->excludes.isEmpty())
                    columnData << ss->excludes.join(",");
                else
                    columnData << "";
                columnData << QVariant::fromValue(schemaHelper->defaultValue.value.stringval);
                SchemaDefinitionItem* item = new SchemaDefinitionItem(schemaName, columnData, parents.last());
                parents.last()->appendChild(item);

                for (int i =0; i<schema->getNumberOfAnyOfDefined(prefix); ++i) {
                    QString keystr = QString("%1[%2]").arg(prefix).arg(i);
                    schemaKeys.removeLast();
                    schemaKeys << keystr;
//                setupAnyofSchemaTree(schemaName, keystr, schemaKeys, parents, schema);
                }
            }
        }
        parents.pop_back();
    }
    if (s->hasType(SchemaType::List)) {
        schemaKeys.removeLast();
    }
}

} // namespace connect
} // namespace studio
} // namespace gams
