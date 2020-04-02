/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "envvartablemodel.h"
#include "scheme.h"

#include <QApplication>

namespace gams {
namespace studio {
namespace option {

EnvVarTableModel::EnvVarTableModel(QList<EnvVarConfigItem *> itemList, QObject *parent):
    QAbstractTableModel(parent), mEnvVarItem(itemList)
{
    mHeader << "Name"  << "Value" << "minVersion" << "maxVersion" << "pathVariable";
}

QVariant EnvVarTableModel::headerData(int index, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
       if (role == Qt::DisplayRole) {
          if (index <= mHeader.size())
              return mHeader.at(index);
       }
       return QVariant();
    }

    // orientation == Qt::Vertical
    switch(role) {
    case Qt::CheckStateRole:
        if (mEnvVarItem.isEmpty())
            return QVariant();
        else
            return mCheckState[index];
    case Qt::DecorationRole:
        if (Qt::CheckState(mCheckState[index].toUInt())==Qt::Checked) {
            return QVariant::fromValue(Scheme::icon(":/img/square-red"));
        } else if (Qt::CheckState(mCheckState[index].toUInt())==Qt::PartiallyChecked) {
                  return QVariant::fromValue(Scheme::icon(":/img/square-gray"));
        } else {
                return QVariant::fromValue(Scheme::icon(":/img/square-green"));
        }
    case Qt::ToolTipRole:
        QString tooltipText = "";
        return tooltipText;
    }
    return QVariant();

}

int EnvVarTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return  mEnvVarItem.size();
}

int EnvVarTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant EnvVarTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (mEnvVarItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        if (col==COLUMN_PARAM_KEY) {
            return mEnvVarItem.at(row)->key;
        } else if (col== COLUMN_PARAM_VALUE) {
                 return mEnvVarItem.at(row)->value;
        } else if (col==COLUMN_MIN_VERSION) {
                  return mEnvVarItem.at(row)->minVersion;
        } else if (col==COLUMN_MAX_VERSION) {
                  return mEnvVarItem.at(row)->maxVersion;
        } else if (col==COLUMN_PATH_VAR) {
            if (mEnvVarItem.at(row)->pathVariable == EnvVarConfigItem::pathDefinition::NONE)
                return "";
            else if (mEnvVarItem.at(row)->pathVariable == EnvVarConfigItem::pathDefinition::PATH_DEFINED)
                    return "True";
            else if (mEnvVarItem.at(row)->pathVariable == EnvVarConfigItem::pathDefinition::NO_PATH_DEFINED)
                return "False";
            else
                return QString(mEnvVarItem.at(row)->pathVariable);
        }
        break;
    }
    case Qt::TextAlignmentRole: {
        if (col==COLUMN_MIN_VERSION || col==COLUMN_MAX_VERSION)
            return Qt::AlignRight+ Qt::AlignVCenter;
        else
           return Qt::AlignLeft+ Qt::AlignVCenter;
    }
    case Qt::ToolTipRole: {
        QString tooltipText = "";
        return tooltipText;
    }
    case Qt::TextColorRole: {
         return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
     }
    default:
        break;
    }
    return QVariant();

}

Qt::ItemFlags EnvVarTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
     if (!index.isValid())
         return Qt::NoItemFlags ;
     else
         return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
}

bool EnvVarTableModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    emit headerDataChanged(orientation, index, index);

    return true;
}

bool EnvVarTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() > mEnvVarItem.size())
        return false;

    QVector<int> roles;
    if (role == Qt::EditRole)   {
        roles = { Qt::EditRole };
        QString dataValue = value.toString().simplified();
        if (dataValue.isEmpty())
            return false;

        if (index.row() > mEnvVarItem.size())
            return false;

        if (index.column() == COLUMN_PARAM_KEY) { // key
            QString from = data(index, Qt::DisplayRole).toString();
            mEnvVarItem[index.row()]->key = dataValue;
        } else if (index.column() == COLUMN_PARAM_VALUE) { // value
                  mEnvVarItem[index.row()]->value = dataValue;
        } else if (index.column() == COLUMN_PATH_VAR) {
                  if (dataValue.isEmpty())
                     mEnvVarItem[index.row()]->pathVariable = EnvVarConfigItem::pathDefinition::NONE;
                  else if (QString::compare(dataValue, "true", Qt::CaseInsensitive) == 0)
                          mEnvVarItem[index.row()]->pathVariable = EnvVarConfigItem::pathDefinition::PATH_DEFINED;
                  else
                      mEnvVarItem[index.row()]->pathVariable = EnvVarConfigItem::pathDefinition::NO_PATH_DEFINED;
        } else if (index.column() == COLUMN_MIN_VERSION) {
                   mEnvVarItem[index.row()]->minVersion = dataValue;
        } else if (index.column() == COLUMN_MAX_VERSION) {
            mEnvVarItem[index.row()]->maxVersion = dataValue;
        }
        emit dataChanged(index, index, roles);
    } else if (role == Qt::CheckStateRole) {
        roles = { Qt::CheckStateRole };
        mCheckState[index.row()] = value;
        emit dataChanged(index, index, roles);
    }
    return true;

}

QModelIndex EnvVarTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

bool EnvVarTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if (count < 1 || row < 0 || row > mEnvVarItem.size())
         return false;

     beginInsertRows(QModelIndex(), row, row + count - 1);
     if (mEnvVarItem.size() == row)
         mEnvVarItem.append(new EnvVarConfigItem());
     else
         mEnvVarItem.insert(row, new EnvVarConfigItem());

    endInsertRows();
    return true;
}

bool EnvVarTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if (count < 1 || row < 0 || row > mEnvVarItem.size() || mEnvVarItem.size() ==0)
         return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i=row+count-1; i>=row; --i) {
        mEnvVarItem.removeAt(i);
    }
    endRemoveRows();
    return true;
}

bool EnvVarTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (mEnvVarItem.size() == 0 || count < 1 || destinationChild < 0 ||  destinationChild > mEnvVarItem.size())
         return false;

    Q_UNUSED(sourceParent)
    Q_UNUSED(destinationParent)
    beginMoveRows(QModelIndex(), sourceRow, sourceRow  + count -1 , QModelIndex(), destinationChild);
//    mEnvVarItem.insert(destinationChild, mEnvVarItem.at(sourceRow));
//    int removeIndex = destinationChild > sourceRow ? sourceRow : sourceRow+1;
//    mEnvVarItem.removeAt(removeIndex);
    if (destinationChild > sourceRow) { // move down
       for(int i=0; i<count; ++i) {
           mEnvVarItem.insert(destinationChild, mEnvVarItem.at(sourceRow));
           mEnvVarItem.removeAt(sourceRow);
       }
    } else { // move up
           for(int i=0; i<count; ++i) {
               EnvVarConfigItem* item = mEnvVarItem.at(sourceRow+i);
               mEnvVarItem.removeAt(sourceRow+i);
               mEnvVarItem.insert(destinationChild+i, item);
           }
    }
//    updateCheckState();
    endMoveRows();
    return true;
}

QList<EnvVarConfigItem *> EnvVarTableModel::envVarConfigItems()
{
    return mEnvVarItem;
}

} // namepsace option
} // namespace studio
} // namespace gams
