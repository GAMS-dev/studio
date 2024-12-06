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
#include "optiondefinitionmodel.h"
#include "theme.h"

namespace gams {
namespace studio {
namespace option {

OptionDefinitionModel::OptionDefinitionModel(Option* data, int optionGroup, QObject* parent)
    : QAbstractItemModel(parent), mOptionGroup(optionGroup), mOption(data)
{
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
        if (item != nullptr)
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
    if (treeItem != nullptr)
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

void OptionDefinitionModel::loadOptionFromGroup(const int group)
{
    mOptionGroup = group;
    beginResetModel();

    removeRows(0, rowCount(), QModelIndex());

    setupTreeItemModelData(mOption, rootItem);

    endResetModel();
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
    case Qt::ForegroundRole: {
        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(index.internalPointer());
        OptionDefinitionItem *parentItem = item->parentItem();
        if (parentItem == rootItem &&  item->modified())
            return  QVariant::fromValue(Theme::color(Theme::Normal_Blue));
        else
            return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
    }
    case Qt::BackgroundRole: {
        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(index.internalPointer());
        OptionDefinitionItem *parentItem = item->parentItem();
        if (parentItem == rootItem) {
            if (index.row() % 2 == 0)
               return QVariant::fromValue(QApplication::palette().color(QPalette::Base));
            else
                return QVariant::fromValue(QGuiApplication::palette().color(QPalette::Window));
        } else {
            return QVariant::fromValue(QApplication::palette().color(QPalette::Base));
        }
    }
    case Qt::ToolTipRole: {
        OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(index.internalPointer());
        return item->data(index.column());
    }
    default:
         break;
    }
    return QVariant();
}

Qt::ItemFlags OptionDefinitionModel::flags(const QModelIndex& index) const
{
    const Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::NoItemFlags;
    else
        return Qt::ItemIsDragEnabled | defaultFlags;
}

bool OptionDefinitionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QVector<int> roles;
    switch (role) {
    case Qt::DisplayRole: {
        roles = { Qt::EditRole };
        OptionDefinitionItem *item = getItem(index);
        const bool result = item->setData(index.column(), value);

        if (result)
            emit dataChanged(index, index, roles);

        return result;
    }
    case Qt::CheckStateRole: {
        roles = { Qt::CheckStateRole };
        OptionDefinitionItem *item = getItem(index);
        item->setModified(value.toInt() == Qt::Checked);
        emit dataChanged(index, index, roles);
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

    OptionDefinitionItem *parentItem = nullptr;

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
    OptionDefinitionItem* parentItem = nullptr;
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

    auto optList = option->getOption();
    for(auto it = optList.cbegin(); it != optList.cend(); ++it)  {
        OptionDefinition optdef =  it.value();

        if ((optdef.deprecated) || (!optdef.valid))
            continue;
        if (mOptionGroup > 0 && mOptionGroup != optdef.groupNumber)
            continue;

        QList<QVariant> columnData;

        columnData.append(optdef.name);
        columnData.append(optdef.synonym);
        switch(optdef.dataType) {
        case optDataInteger:
            if (optdef.type == optTypeBoolean && optdef.subType == optsubNoValue)
                columnData.append("");
            else
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
        case optTypeInteger : {
            columnData.append( QString("[%1, %2]").arg( optdef.lowerBound.toInt() ).arg( optdef.upperBound.toInt() ) );
            columnData.append("Integer");
            break;
        }
        case optTypeDouble : {
            columnData.append( QString("[%1, %2]").arg( optdef.lowerBound.canConvert<double>() ? optdef.lowerBound.toDouble() : optdef.lowerBound.toInt() )
                                                 .arg( optdef.upperBound.canConvert<double>() ? optdef.upperBound.toDouble() : optdef.upperBound.toInt() ) );
            columnData.append("Double");
            break;
        }
        case optTypeString : {
            columnData.append("");
            if (optdef.subType == optsubNoValue)
               columnData.append("String (no Value)");
            else
               columnData.append("String");
            break;
        }
        case optTypeBoolean : {
            if (optdef.subType == optsubNoValue) {
                columnData.append("");
                columnData.append("Boolean (no Value)");
            } else {
                columnData.append("{0, 1}");
                columnData.append("Boolean");
            }
            break;
        }
        case optTypeEnumStr : {
            QString range("");
            QList<OptionValue> valueList;
            for(const OptionValue &value : std::as_const(optdef.valueList)) {
                if (!value.hidden)
                    valueList.append( value );
            }
            if (!valueList.isEmpty()) {
                range.append("{");
                int i = 0;
                for(i =0; i<valueList.size()-1; i++) {
                    range.append(valueList.at(i).value.toString());
                    if (valueList.size()>=NUMBER_DISPLAY_ENUMSTR_RANGE) {
                        if (i+1>=NUMBER_DISPLAY_ENUMSTR_RANGE && i<valueList.size()-2) {
                           range.append(",..,");
                           break;
                        }
                    }
                    range.append(", ");
                }
                range.append( valueList.at(valueList.size()-1).value.toString() );
                range.append("}");
            }
            columnData.append(range);
            columnData.append("EnumStr");
            break;
        }
        case optTypeEnumInt : {
            QString range("");
            QList<OptionValue> valueList;
            for(const OptionValue &value : std::as_const(optdef.valueList)) {
                if (!value.hidden)
                    valueList.append( value );
            }
            if (!valueList.isEmpty()) {
                range.append("{");
                int i = 0;
                for(i =0; i<valueList.size()-1; i++) {
                    range.append(valueList.at(i).value.toString());
                    range.append(",");
                }
                range.append( valueList.at(i).value.toString() );
                range.append("}");
            }
            columnData.append(range);
            columnData.append("EnumInt");
            break;
        }
        case optTypeMultiList : {
            columnData.append("");
            columnData.append("MultiList");
            break;
        }
        case optTypeStrList   : {
            columnData.append("");
            columnData.append("StrList");
            break;
        }
        case optTypeMacro     : {
            columnData.append("");
            columnData.append("Macro");
            break;
        }
        case optTypeImmediate : {
            columnData.append("");
            columnData.append("Immediate");
            break;
        }
        }
        columnData.append(optdef.description);
        columnData.append(optdef.number);
        OptionDefinitionItem* item = new OptionDefinitionItem(columnData, parents.last());
        item->setModified(optdef.modified);

        parents.last()->appendChild(item);

        if (!optdef.valueList.isEmpty()) {
            parents << parents.last()->child(parents.last()->childCount()-1);
            for(int j=0; j<optdef.valueList.size(); ++j) {
                const OptionValue enumValue = optdef.valueList.at(j);
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
