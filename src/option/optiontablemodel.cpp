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
#include <QMimeData>
#include <QIcon>
#include <QDebug>

#include "optiontablemodel.h"

namespace gams {
namespace studio {
namespace option {

OptionTableModel::OptionTableModel(const QList<OptionItem> itemList, OptionTokenizer *tokenizer, QObject *parent) :
    QAbstractTableModel(parent), mOptionItem(itemList), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption())
{
    mHeader << "Option" << "Value" << "Debug Entry";
}

QVariant OptionTableModel::headerData(int index, Qt::Orientation orientation, int role) const
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
        if (mOptionItem.isEmpty())
            return QVariant();
        else
            return mCheckState[index];
    case Qt::ToolTipRole: {
        switch (mOption->getValueErrorType(mOptionItem.at(index).key, mOptionItem.at(index).value)) {
        case Incorrect_Value_Type:
             return QString("Option key '%1' has a value of incorrect type").arg(mOptionItem.at(index).key);
        case Value_Out_Of_Range:
             return QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(index).value).arg(mOptionItem.at(index).key);
        default:
            break;
        }
        break;
    }
    case Qt::DecorationRole:
        if (Qt::CheckState(mCheckState[index].toUInt())==Qt::Checked) {
            return QVariant::fromValue(QIcon(":/img/square-red"));
        } else if (Qt::CheckState(mCheckState[index].toUInt())==Qt::PartiallyChecked) {
            return QVariant::fromValue(QIcon(":/img/square-gray"));
        } else {
            return QVariant::fromValue(QIcon(":/img/square-green"));
        }
    }

    return QVariant();
}

int OptionTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return  mOptionItem.size();
}

int OptionTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant OptionTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (mOptionItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        if (col==0) {
            return QVariant(mOptionItem.at(row).key);
        } else if (col==1) {
                 return QVariant(mOptionItem.at(row).value);
        } else if (col==2) {
            QString key = mOptionItem.at(row).key;
            if (mOption->isASynonym(mOptionItem.at(row).key))
                key = mOption->getNameFromSynonym(mOptionItem.at(row).key);
            return QVariant(mOption->getOptionDefinition(key).number);
        }
        break;
    }
    case Qt::TextAlignmentRole: {
        return Qt::AlignLeft;
    }
    case Qt::ToolTipRole: {
        switch (mOptionItem.at(row).error) {
        case Incorrect_Value_Type:
             return QString("Option key '%1' has a value of incorrect type").arg(mOptionItem.at(row).key);
        case Value_Out_Of_Range:
             return QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(row).value).arg(mOptionItem.at(row).key);
        default:
             break;
        }
        break;
    }
    case Qt::TextColorRole: {
        if (col==1) {
            switch (mOptionItem.at(row).error) {
                case Incorrect_Value_Type:
                     return QVariant::fromValue(QColor(Qt::red));
                case Value_Out_Of_Range:
                    return QVariant::fromValue(QColor(Qt::red));
                case No_Error:
                    return QVariant::fromValue(QColor(Qt::black));
                default:
                    return QVariant::fromValue(QColor(Qt::black));
            }
         }
        break;
     }
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags OptionTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
     if (!index.isValid()) {
         return Qt::NoItemFlags | Qt::ItemIsDropEnabled ;
     } else {
         if (index.column() !=1)
              return Qt::ItemIsDropEnabled | defaultFlags;
         else
             return Qt::ItemIsEditable | Qt::ItemIsDropEnabled | defaultFlags;
     }
}

bool OptionTableModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    emit headerDataChanged(orientation, index, index);

    return true;
}

bool OptionTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QVector<int> roles;
    if (role == Qt::EditRole)   {
        roles = { Qt::EditRole };
        QString dataValue = value.toString().simplified();
        if (dataValue.isEmpty())
            return false;

        if (index.row() > mOptionItem.size())
            return false;

        if (index.column() == 0) { // key
            mOptionItem[index.row()].key = dataValue;
        } else if (index.column() == 1) { // value
                  mOptionItem[index.row()].value = dataValue;
                  emit optionValueChanged();
        }
    } else if (role == Qt::CheckStateRole) {
        roles = { Qt::CheckStateRole };
        if (index.row() > mOptionItem.size())
            return false;

        mOptionItem[index.row()].disabled = value.toBool();
    }
    emit dataChanged(index, index, roles);
    return true;
}

QModelIndex OptionTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

bool OptionTableModel::insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
{
    Q_UNUSED(parent);
    if (count < 1 || row < 0 || row > mOptionItem.size())
         return false;

     beginInsertRows(QModelIndex(), row, row + count - 1);
     if (mOptionItem.size() == row)
         mOptionItem.append(OptionItem("", "", -1, -1));
     else
        mOptionItem.insert(row, OptionItem(OptionItem("", "", -1, -1)));

    endInsertRows();
    return true;
}

bool OptionTableModel::removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
{
    Q_UNUSED(parent);
    if (count < 1 || row < 0 || row > mOptionItem.size() || mOptionItem.size() ==0)
         return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i=row+count-1; i>=row; --i) {
        mOptionItem.removeAt(i);
    }
    endRemoveRows();
    emit  optionValueChanged();
    return true;
}

bool OptionTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (mOptionItem.size() == 0 || count < 1 || destinationChild < 0 ||  destinationChild > mOptionItem.size())
         return false;

    Q_UNUSED(sourceParent); Q_UNUSED(destinationParent);
    beginMoveRows(QModelIndex(), sourceRow, sourceRow  + count - 1, QModelIndex(), destinationChild);
    mOptionItem.insert(destinationChild, mOptionItem.at(sourceRow));
    int removeIndex = destinationChild > sourceRow ? sourceRow : sourceRow+1;
    mOptionItem.removeAt(removeIndex);
    endMoveRows();
//    emit  optionValueChanged();
    return true;
}

QStringList OptionTableModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.solver-opt.text";
    return types;
}

QList<OptionItem> OptionTableModel::getCurrentListOfOptionItems() const
{
    return mOptionItem;
}


void OptionTableModel::reloadOptionModel(const QList<OptionItem> &optionItem)
{
    beginResetModel();
    mOptionItem = optionItem;
    mOptionTokenizer->validateOption(mOptionItem);

    setRowCount(mOptionItem.size());

    for (int i=0; i<mOptionItem.size(); ++i) {
        setData(QAbstractTableModel::createIndex(i, 0), QVariant(mOptionItem.at(i).key), Qt::EditRole);
        setData(QAbstractTableModel::createIndex(i, 1), QVariant(mOptionItem.at(i).value), Qt::EditRole);
        if (mOptionItem.at(i).error == No_Error)
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Unchecked),
                              Qt::CheckStateRole );
        else if (mOptionItem.at(i).error == Deprecated_Option)
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
        else setHeaderData( i, Qt::Vertical,
                          Qt::CheckState(Qt::Checked),
                          Qt::CheckStateRole );
    }
    emit optionModelChanged(mOptionItem);
    endResetModel();
}

void OptionTableModel::on_optionValueChanged()
{
    beginResetModel();
    mOptionTokenizer->validateOption(mOptionItem);

    setRowCount(mOptionItem.size());

    for (int i=0; i<mOptionItem.size(); ++i) {
        if (mOptionItem.at(i).error == No_Error)
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Unchecked),
                              Qt::CheckStateRole );
        else if (mOptionItem.at(i).error == Deprecated_Option)
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
        else setHeaderData( i, Qt::Vertical,
                          Qt::CheckState(Qt::Checked),
                          Qt::CheckStateRole );
    }
    endResetModel();
}

void OptionTableModel::setRowCount(int rows)
{
    int rc = mOptionItem.size();
    if (rows < 0 ||  rc == rows)
       return;

    if (rc < rows)
       insertRows(qMax(rc, 0), rows - rc);
    else
       removeRows(qMax(rows, 0), rc - rows);
}

Qt::DropActions OptionTableModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

bool OptionTableModel::dropMimeData(const QMimeData* mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column);
    for(QString f : mimedata->formats()) {
       qDebug() << "format:" << f;
    }
    if (action == Qt::IgnoreAction)
        return true;

    if (!mimedata->hasFormat("application/vnd.solver-opt.text"))
        return false;

    QByteArray encodedData = mimedata->data("application/vnd.solver-opt.text");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;
    int rows = 0;

    while (!stream.atEnd()) {
       QString text;
       stream >> text;
       newItems << text;
       ++rows;
    }

    int beginRow;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    if (action ==  Qt::CopyAction) {
        insertRows(beginRow, rows, QModelIndex());

        for (const QString &text : newItems) {
            QStringList textList = text.split("=");
            QModelIndex idx = index(beginRow, 0, QModelIndex());
            setData(idx, textList.at(0), Qt::EditRole);
            idx = index(beginRow, 1, QModelIndex());
            setData(idx, textList.at(1), Qt::EditRole);
            emit newTableRowDropped(index(beginRow, 0, QModelIndex()));
            beginRow++;
        }
        return true;

    }

    return false;
}

} // namepsace option
} // namespace studio
} // namespace gams
