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
#include <QColor>
#include <QDebug>
#include "optiondefinitionmodel.h"

namespace gams {
namespace studio {
namespace option {

OptionDefinitionModel::OptionDefinitionModel(Option* data, int optionGroup, QObject* parent)
    : QAbstractItemModel(parent), mOptionGroup(optionGroup), mOption(data)
{
    QList<QVariant> rootData;
    rootData << "Option" << "Synonym" << "DefValue" << "Range"
             << "Type" << "Description" << "Debug Entry" ;
    rootItem = new OptionDefinitionItem(rootData);

    setupTreeItemModelData(mOption, rootItem);
}

OptionDefinitionModel::~OptionDefinitionModel()
{
    delete rootItem;
}

int OptionDefinitionModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return static_cast<OptionDefinitionItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

OptionDefinitionItem *OptionDefinitionModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

void OptionDefinitionModel::insertItem(int position, OptionDefinitionItem *item, const QModelIndex &parent)
{
    OptionDefinitionItem* parentItem = getItem(parent);

    beginInsertRows(parent, position, position);
    parentItem->insertChild(position, item);
    endInsertRows();
}

bool OptionDefinitionModel::removeItem(const QModelIndex &index)
{
    OptionDefinitionItem* treeItem = getItem(index);
    if (treeItem)
        return removeRows(treeItem->row(), 1, parent(index));
    else
        return false;
}

bool OptionDefinitionModel::removeRows(int row, int count, const QModelIndex &parent)
{
    OptionDefinitionItem* parentItem = getItem(parent);
    bool success = false;

    if (count > 0) {
        beginRemoveRows(parent, row, row + count - 1);
        success = parentItem->removeChildren(row, count);
        endRemoveRows();
    }

    return success;
}

//QStringList OptionDefinitionModel::mimeTypes() const
//{
//    QStringList types;
//    types << "application/vnd.gams-pf.text";
//    return types;
//}

//QMimeData* OptionDefinitionModel::mimeData(const QModelIndexList &indexes) const
//{
//    QMimeData* mimeData = new QMimeData();
//    QByteArray encodedData;

//    QDataStream stream(&encodedData, QIODevice::WriteOnly);

//    foreach (const QModelIndex &index, indexes) {
//        if (index.isValid()) {
//            if (index.column()>0) {
//                continue;
//            }

//            QString text;
//            OptionDefinitionItem *childItem = static_cast<OptionDefinitionItem*>(index.internalPointer());
//            OptionDefinitionItem *parentItem = childItem->parentItem();

//            if (parentItem == rootItem) {
//                QModelIndex defValueIndex = index.sibling(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE);
//                text = QString("%1=%2").arg(data(index, Qt::DisplayRole).toString()).arg(data(defValueIndex, Qt::DisplayRole).toString());
//            } else {
//                text = QString("%1=%2").arg(parentItem->data(index.column()).toString()).arg(data(index, Qt::DisplayRole).toString());
//            }
//            stream << text;
//        }
//    }

//    mimeData->setData("application/vnd.gams-pf.text", encodedData);
//    return mimeData;
//}

void OptionDefinitionModel::loadOptionFromGroup(const int group)
{
    qDebug() << "option from group " << group << " to be loaded!";
    mOptionGroup = group;
    beginResetModel();

    removeRows(0, rowCount(), QModelIndex());

    setupTreeItemModelData(mOption, rootItem);

    endResetModel();
}

void OptionDefinitionModel::updateModifiedOptionDefinition(const QList<OptionItem> &optionItems)
{
    QStringList optionNameList;
    for(OptionItem item : optionItems) {
        optionNameList << item.key;
    }
    for(int i=0; i<rowCount(); ++i)  {
        QModelIndex node = index(i, OptionDefinitionModel::COLUMN_OPTION_NAME);

        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(node.internalPointer());
        OptionDefinitionItem *parentItem = item->parentItem();
        if (parentItem == rootItem) {
            OptionDefinition optdef = mOption->getOptionDefinition(item->data(OptionDefinitionModel::COLUMN_OPTION_NAME).toString());
            if (optionNameList.contains(optdef.name, Qt::CaseInsensitive) || optionNameList.contains(optdef.synonym, Qt::CaseInsensitive))
                optdef.modified = true;
            else
                optdef.modified = false;
            setData(node, optdef.modified ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

    }
}

void OptionDefinitionModel::modifyOptionDefinition(const QList<SolverOptionItem *> &optionItems)
{
    QMap<QString, int> modifiedOption;
    for(int i = 0; i<optionItems.size(); ++i)
        modifiedOption[optionItems.at(i)->key] = i;

    QStringList keys = modifiedOption.keys();
    for(int i=0; i<rowCount(); ++i)  {
        QModelIndex node = index(i, OptionDefinitionModel::COLUMN_OPTION_NAME);

        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(node.internalPointer());
        OptionDefinitionItem *parentItem = item->parentItem();
        if (parentItem == rootItem) {
            OptionDefinition optdef = mOption->getOptionDefinition(item->data(OptionDefinitionModel::COLUMN_OPTION_NAME).toString());
            if (keys.contains(optdef.name, Qt::CaseInsensitive)) {
                if (optionItems.at(modifiedOption[optdef.name])->disabled)
                    optdef.modified = false;
                else
                    optdef.modified = true;
            } else if (keys.contains(optdef.synonym, Qt::CaseInsensitive)) {
                      if (optionItems.at(modifiedOption[optdef.synonym])->disabled)
                          optdef.modified = false;
                      else
                          optdef.modified = true;
            } else {
                optdef.modified = false;
            }
            setData(node, optdef.modified ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
    }
}

QVariant OptionDefinitionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(index.internalPointer());
        return item->data(index.column());
    }
    case Qt::CheckStateRole: {
        if (index.column()==COLUMN_OPTION_NAME) {
           OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(index.internalPointer());
           OptionDefinitionItem *parentItem = item->parentItem();
           if (parentItem == rootItem)
              return Qt::CheckState(item->modified() ? Qt::Checked : Qt::Unchecked );
        }
        return QVariant();
    }
    default:
         break;
    }
    return QVariant();
}

Qt::ItemFlags OptionDefinitionModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::NoItemFlags;
    else
        return Qt::ItemIsDragEnabled | defaultFlags;
}

bool OptionDefinitionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    switch (role) {
    case Qt::DisplayRole: {
        OptionDefinitionItem *item = getItem(index);
        bool result = item->setData(index.column(), value);

        if (result)
            emit dataChanged(index, index);

        return result;
    }
    case Qt::CheckStateRole: {
        OptionDefinitionItem *item = getItem(index);
        item->setModified(value.toInt() == Qt::Checked);
        emit dataChanged(index, index);
        return true;
    }
    default:
        break;
    }

    return false;
}

QVariant OptionDefinitionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex OptionDefinitionModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    OptionDefinitionItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<OptionDefinitionItem*>(parent.internalPointer());

    OptionDefinitionItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex OptionDefinitionModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    OptionDefinitionItem *childItem = static_cast<OptionDefinitionItem*>(index.internalPointer());
    OptionDefinitionItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int OptionDefinitionModel::rowCount(const QModelIndex& parent) const
{
    OptionDefinitionItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<OptionDefinitionItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void OptionDefinitionModel::setupTreeItemModelData(Option* option, OptionDefinitionItem* parent)
{
    QList<OptionDefinitionItem*> parents;
    parents << parent;

    for(auto it = option->getOption().cbegin(); it != option->getOption().cend(); ++it)  {
        OptionDefinition optdef =  it.value();
        if ((optdef.deprecated) || (!optdef.valid))
            continue;
        if (mOptionGroup > 0 && mOptionGroup != optdef.groupNumber)
            continue;

        QList<QVariant> columnData;

        QString optionStr = optdef.name
                              + (optdef.synonym.isEmpty() ? "" : QString("[%1]").arg(optdef.synonym));
        columnData.append(optdef.name);
        columnData.append(optdef.synonym);
        switch(optdef.dataType) {
        case optDataInteger:
            columnData.append(optdef.defaultValue.toInt());
            break;
        case optDataDouble:
            columnData.append(optdef.defaultValue.toDouble());
            break;
        case optDataString:
            columnData.append(optdef.defaultValue.toString());
            break;
        default:
            columnData.append("");
            break;
        }
        switch(optdef.type){
        case optTypeInteger :
            columnData.append( QString("[%1, %2]").arg( optdef.lowerBound.canConvert<int>() ? optdef.lowerBound.toInt() : optdef.lowerBound.toDouble() )
                                                 .arg( optdef.upperBound.canConvert<int>() ? optdef.upperBound.toInt() : optdef.upperBound.toDouble() ) );
            columnData.append("Integer");
            break;
        case optTypeDouble :
            columnData.append( QString("[%1, %2]").arg( optdef.lowerBound.canConvert<double>() ? optdef.lowerBound.toDouble() : optdef.lowerBound.toInt() )
                                                 .arg( optdef.upperBound.canConvert<double>() ? optdef.upperBound.toDouble() : optdef.upperBound.toInt() ) );
            columnData.append("Double");
            break;
        case optTypeString :
            columnData.append("");
            columnData.append("String");
            break;
        case optTypeBoolean :
            columnData.append("");
            columnData.append("Boolean");
            break;
        case optTypeEnumStr :
            columnData.append("");
            columnData.append("EnumStr");
            break;
        case optTypeEnumInt :
            columnData.append("");
            columnData.append("EnumInt");
            break;
        case optTypeMultiList :
            columnData.append("");
            columnData.append("MultiList");
            break;
        case optTypeStrList   :
            columnData.append("");
            columnData.append("StrList");
            break;
        case optTypeMacro     :
            columnData.append("");
            columnData.append("Macro");
            break;
        case optTypeImmediate :
            columnData.append("");
            columnData.append("Immediate");
            break;
        default:
            columnData.append("");
            columnData.append("");
            break;
        }
        columnData.append(optdef.description);
        columnData.append(optdef.number);
        OptionDefinitionItem* item = new OptionDefinitionItem(columnData, parents.last());
        item->setModified(optdef.modified);

        parents.last()->appendChild(item);

        if (optdef.valueList.size() > 0) {
            parents << parents.last()->child(parents.last()->childCount()-1);
            for(int j=0; j<optdef.valueList.size(); ++j) {
                OptionValue enumValue = optdef.valueList.at(j);
                if (enumValue.hidden)
                    continue;
                QList<QVariant> enumData;
                enumData << enumValue.value;
                enumData << "";
                enumData << "";
                enumData << "";
                enumData << "";
                enumData << enumValue.description;
                enumData << "";
                parents.last()->appendChild(new OptionDefinitionItem(enumData, parents.last()));
            }
            parents.pop_back();
        }
    }
}

} // namespace option
} // namespace studio
} // namespace gams
