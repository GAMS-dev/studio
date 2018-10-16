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
#include <QIcon>
#include <QDebug>

#include "solveroptiontablemodel.h"

namespace gams {
namespace studio {
namespace option {

SolverOptionTableModel::SolverOptionTableModel(const QList<OptionItem> itemList, OptionTokenizer *tokenizer, QObject *parent):
    OptionTableModel(itemList, tokenizer, parent)
{
}

QVariant SolverOptionTableModel::headerData(int index, Qt::Orientation orientation, int role) const
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

QVariant SolverOptionTableModel::data(const QModelIndex &index, int role) const
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
        switch (mOption->getValueErrorType(mOptionItem.at(row).key, mOptionItem.at(row).value)) {
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
            switch (mOption->getValueErrorType(mOptionItem.at(row).key, mOptionItem.at(row).value)) {
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

bool SolverOptionTableModel::insertRows(int row, int count, const QModelIndex &parent)
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
    emit optionModelChanged(mOptionItem);
    return true;
}

Qt::ItemFlags SolverOptionTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
     if (!index.isValid()) {
         qDebug() << "flags : (" << index.row() << "," << index.column() << ") NOT VALID!" ;
         return Qt::NoItemFlags | Qt::ItemIsDropEnabled ;
     } else {
         if (index.column()!=1) {
             qDebug() << "flags : (" << index.row() << "," << index.column() << ") NOT EDITABLE" ;
            return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
         } else {
             qDebug() << "flags : (" << index.row() << "," << index.column() << ") EDITABLE" ;
            return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
         }
     }
}

Qt::DropActions SolverOptionTableModel::supportedDropActions() const
{
    return Qt::CopyAction ;
}

bool SolverOptionTableModel::dropMimeData(const QMimeData* mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column);
    if (action == Qt::IgnoreAction)
        return true;
    for(QString f : mimedata->formats()) {
       qDebug() << "format:" << f;
    }
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

//        QList<int> insertRowList;
        insertRows(beginRow, rows, QModelIndex());

        foreach (const QString &text, newItems) {
//            insertRowList.append( beginRow );

            QStringList textList = text.split("=");
            QModelIndex idx = index(beginRow, 0, QModelIndex());
            setData(idx, textList.at(0), Qt::EditRole);
            idx = index(beginRow, 1, QModelIndex());
            setData(idx, textList.at(1), Qt::EditRole);
            beginRow++;
        }

//        foreach (const QString &text, newItems) {
//            QStringList textList = text.split("=");
//            QModelIndex idx;
//            for(int i=0; i<rowCount(); ++i) {
//                if (insertRowList.contains(i))
//                    continue;

//                idx = index(i, 0, QModelIndex());
//                QString key = data(idx, Qt::DisplayRole).toString();
//                if (QString::compare(key, textList.at(0), Qt::CaseInsensitive)==0)
//                    break;
//            }
//            if (idx.row() < rowCount())
//               removeRows(idx.row(), 1, QModelIndex());
//        }
        return true;

    }  /*else if (action == Qt::MoveAction ) {

        foreach (const QString &text, newItems) {
            qDebug() << text;
            QStringList textList = text.split("=");
            QModelIndex idx;
            for(int i=0; i<rowCount(QModelIndex()); ++i) {
                idx = index(i, 0, QModelIndex());
                QString key = data(idx, Qt::DisplayRole).toString();
                if (QString::compare(key, textList.at(0), Qt::CaseInsensitive)==0)
                    break;
            }
            if (idx.row() < beginRow)
                moveRows(QModelIndex(), idx.row(), 1, QModelIndex(), beginRow+1 );
            else
                moveRows(QModelIndex(), idx.row(), 1, QModelIndex(), beginRow );
        }
        return true;
    }*/

    return false;
}

} // namepsace option
} // namespace studio
} // namespace gams
