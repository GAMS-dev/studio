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

SolverOptionTableModel::SolverOptionTableModel(const QList<SolverOptionItem *> itemList, OptionTokenizer *tokenizer, QObject *parent):
    QAbstractTableModel(parent), mOptionItem(itemList), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption())
{
    mHeader << "Option" << "Value" << "Debug Entry";

    for(SolverOptionItem* item : mOptionItem) {
        if (item->disabled)
            item->error = No_Error;
        else
            item->error = mOption->getValueErrorType(item->key, item->value.toString());
    }
    updateCheckState();
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
//      mOption->getValueErrorType(mOptionItem.at(index)->key, mOptionItem.at(index)->value.toString()))
        if (mOptionItem.at(index)->disabled) {
            return QString("%1").arg(mOptionItem.at(index)->text);
        } else {
          switch(mOptionItem.at(index)->error) {
          case Invalid_Key:
              return QString("Unknown option '%1'").arg(mOptionItem.at(index)->key);
          case Incorrect_Value_Type:
              return QString("Option key '%1' has a value of incorrect type").arg(mOptionItem.at(index)->key);
          case Value_Out_Of_Range:
              return QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(index)->value.toString()).arg(mOptionItem.at(index)->key);
          case Deprecated_Option:
              return QString("Option '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(index)->key);
          default:
             break;
          }
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

int SolverOptionTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return  mOptionItem.size();
}

int SolverOptionTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant SolverOptionTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (mOptionItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        if (mOptionItem.at(row)->disabled) { // comment
            if (col==0) {
                QString text = mOptionItem.at(row)->text;
                if (mOptionItem.at(row)->text.startsWith("*"))
                    text = mOptionItem.at(row)->text.mid(1);
                return QVariant(text);
            } else if (col==1) {
                       return QVariant("");
            } else if (col==2) {
                QString key = mOptionItem.at(row)->key;
                if (mOption->isASynonym(mOptionItem.at(row)->key))
                    key = mOption->getNameFromSynonym(mOptionItem.at(row)->key);
                return QVariant(mOption->getOptionDefinition(key).number);
            }
        } else { // not a comment
            if (col==0) {
                return QVariant(mOptionItem.at(row)->key);
            } else if (col==1) {
                     return mOptionItem.at(row)->value;
            } else if (col==2) {
                QString key = mOptionItem.at(row)->key;
                if (mOption->isASynonym(mOptionItem.at(row)->key))
                    key = mOption->getNameFromSynonym(mOptionItem.at(row)->key);
                return QVariant(mOption->getOptionDefinition(key).number);
            }
        }
        break;
    }
    case Qt::TextAlignmentRole: {
        return Qt::AlignLeft;
    }
    case Qt::ToolTipRole: {
        if (mOptionItem.at(row)->disabled) {
            return QString("%1").arg(mOptionItem.at(row)->text);
        } else {
          switch(mOptionItem.at(row)->error) {
          case Invalid_Key:
              return QString("Unknown option '%1'").arg(mOptionItem.at(row)->key);
          case Incorrect_Value_Type:
             return QString("Option key '%1' has a value of incorrect type").arg(mOptionItem.at(row)->key);
          case Value_Out_Of_Range:
             return QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(row)->value.toString()).arg(mOptionItem.at(row)->key);
          case Deprecated_Option:
              return QString("Option '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(row)->key);
          default:
             break;
          }
        }
        break;
    }
    case Qt::TextColorRole: {
        if (mOptionItem.at(row)->disabled) {
            return QVariant::fromValue(QColor(Qt::gray));
        } else {
            switch(mOptionItem.at(row)->error) {
                case Incorrect_Value_Type:
                     return QVariant::fromValue(QColor(Qt::red));
                case Value_Out_Of_Range:
                    return QVariant::fromValue(QColor(Qt::red));
                case Deprecated_Option:
                    return QVariant::fromValue(QColor(Qt::red));
                case No_Error:
                    return QVariant::fromValue(QColor(Qt::black));
                default:
                    return QVariant::fromValue(QColor(Qt::black));
             }
        }
     }
     default:
        break;
    }
    return QVariant();

}

QSize SolverOptionTableModel::span(const QModelIndex &index) const
{
    if (mOptionItem.at(index.row())->disabled) {
        if (index.column()==0)
           return QSize(3,1);
    }

    return QSize(1,1);
//    return QAbstractItemModel::span(index);
}

Qt::ItemFlags SolverOptionTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
     if (!index.isValid()) {
         return Qt::NoItemFlags | Qt::ItemIsDropEnabled ;
     } else {
         if (index.column()!=1) {
            return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
         } else {
            return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
         }
     }
}

bool SolverOptionTableModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    if (value.toInt()==Qt::CheckState(Qt::PartiallyChecked))
        mOptionItem.at(index)->disabled = true;
    else
         mOptionItem.at(index)->disabled = false;

    emit headerDataChanged(orientation, index, index);
    return true;
}

bool SolverOptionTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QVector<int> roles;
    if (role == Qt::EditRole)   {
        roles = { Qt::EditRole };
        QString dataValue = value.toString().simplified();
        if (dataValue.isEmpty())
            return false;

        if (index.row() > mOptionItem.size())
            return false;

        if (mOptionItem[index.row()]->disabled) {
            if (index.column() == 0) { // text
                mOptionItem[index.row()]->text = dataValue;
            }
        } else {
            if (index.column() == 0) { // key
                mOptionItem[index.row()]->key = dataValue;
            } else if (index.column() == 1) { // value
                      mOptionItem[index.row()]->value = dataValue;
                      emit solverOptionValueChanged();
            }
        }
    } else if (role == Qt::CheckStateRole) {
        roles = { Qt::CheckStateRole };
        if (index.row() > mOptionItem.size())
            return false;

        mOptionItem[index.row()]->disabled = (Qt::CheckState(value.toUInt())==Qt::PartiallyChecked);
        mCheckState[index.row()] = value;
    }
    emit dataChanged(index, index, roles);
    return true;
}

QModelIndex SolverOptionTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

bool SolverOptionTableModel::insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
{
    Q_UNUSED(parent);
    if (count < 1 || row < 0 || row > mOptionItem.size())
         return false;

    beginInsertRows(QModelIndex(), row, row + count - 1);
    if (mOptionItem.size() == row)
         mOptionItem.append(new SolverOptionItem(-1, "", "", "", false, false));
    else
        mOptionItem.insert(row, (new SolverOptionItem(-1, "", "", "", false, false)));

    updateCheckState();

    endInsertRows();
    return true;
}

bool SolverOptionTableModel::removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
{
    Q_UNUSED(parent);
    if (count < 1 || row < 0 || row > mOptionItem.size() || mOptionItem.size() ==0)
         return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i=row+count-1; i>=row; --i) {
        SolverOptionItem* item = mOptionItem.at(i);
        delete item;
        mOptionItem.removeAt(i);
    }
    endRemoveRows();
    emit  solverOptionValueChanged();
    return true;
}

bool SolverOptionTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (mOptionItem.size() == 0 || count < 1 || destinationChild < 0 ||  destinationChild > mOptionItem.size())
         return false;

    Q_UNUSED(sourceParent); Q_UNUSED(destinationParent);
    beginMoveRows(QModelIndex(), sourceRow, sourceRow  + count -1 , QModelIndex(), destinationChild);
//    mOptionItem.insert(destinationChild, mOptionItem.at(sourceRow));
//    int removeIndex = destinationChild > sourceRow ? sourceRow : sourceRow+1;
//    mOptionItem.removeAt(removeIndex);
    if (destinationChild > sourceRow) { // move down
       for(int i=0; i<count; ++i) {
           mOptionItem.insert(destinationChild, mOptionItem.at(sourceRow));
           mOptionItem.removeAt(sourceRow);
       }
    } else { // move up
           for(int i=0; i<count; ++i) {
               SolverOptionItem* item = mOptionItem.at(sourceRow+i);
               mOptionItem.removeAt(sourceRow+i);
               mOptionItem.insert(destinationChild+i, item);
           }
    }
    updateCheckState();
    endMoveRows();
    return true;
}

QStringList SolverOptionTableModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.solver-opt.text";
    return types;
}


Qt::DropActions SolverOptionTableModel::supportedDropActions() const
{
    return Qt::CopyAction ;
}

bool SolverOptionTableModel::dropMimeData(const QMimeData* mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
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

        foreach (const QString &text, newItems) {
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

QList<SolverOptionItem *> SolverOptionTableModel::getCurrentListOfOptionItems() const
{
    return mOptionItem;
}

void SolverOptionTableModel::reloadSolverOptionModel(const QList<SolverOptionItem *> &optionItem)
{
    beginResetModel();

    qDeleteAll(mOptionItem);
    mOptionItem.clear();

    mOptionItem = optionItem;
    mOptionTokenizer->validateOption(mOptionItem);
    updateCheckState();

    setRowCount(mOptionItem.size());

    for (int i=0; i<mOptionItem.size(); ++i) {
        if (mOptionItem.at(i)->disabled) {
            setData( index(i, 0), QVariant(mOptionItem.at(i)->text), Qt::EditRole);
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
        } else {
           setData( index(i, 0), QVariant(mOptionItem.at(i)->key), Qt::EditRole);
           setData( index(i, 1), QVariant(mOptionItem.at(i)->value), Qt::EditRole);
           setData( index(i, 2), QVariant(mOptionItem.at(i)->optionId), Qt::EditRole);
           if (mOptionItem.at(i)->error == No_Error)
               setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Unchecked),
                              Qt::CheckStateRole );
           else if (mOptionItem.at(i)->error == Deprecated_Option)
                   setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
           else setHeaderData( i, Qt::Vertical,
                          Qt::CheckState(Qt::Checked),
                          Qt::CheckStateRole );
        }
    }
    emit solverOptionModelChanged(mOptionItem);
    endResetModel();
}

void SolverOptionTableModel::on_solverOptionValueChanged()
{
    beginResetModel();
    mOptionTokenizer->validateOption(mOptionItem);

    setRowCount(mOptionItem.size());

    for (int i=0; i<mOptionItem.size(); ++i) {
        if (mOptionItem.at(i)->disabled) {
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
        } else {
            if (mOptionItem.at(i)->error == No_Error)
                setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Unchecked),
                              Qt::CheckStateRole );
            else if (mOptionItem.at(i)->error == Deprecated_Option)
                setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
            else setHeaderData( i, Qt::Vertical,
                          Qt::CheckState(Qt::Checked),
                          Qt::CheckStateRole );
        }
    }
    emit solverOptionModelChanged(mOptionItem);
    endResetModel();
}

void SolverOptionTableModel::on_toggleRowHeader(int logicalIndex)
{
    if (mCheckState[logicalIndex].toInt() == Qt::PartiallyChecked) { // from comment
        QStringList splittedOption = mOptionTokenizer->splitOptionFromComment(mOptionItem.at(logicalIndex));
        mOptionItem.at(logicalIndex)->key = splittedOption.at(0);
        mOptionItem.at(logicalIndex)->value = splittedOption.at(1);
        mOptionItem.at(logicalIndex)->text = "";
        mOptionItem.at(logicalIndex)->disabled = false;
        mOptionItem.at(logicalIndex)->error = mOption->getValueErrorType( mOptionItem.at(logicalIndex)->key, mOptionItem.at(logicalIndex)->value.toString() );
        updateCheckState();
        setData( index(logicalIndex,0), mOptionItem.at(logicalIndex)->key, Qt::EditRole );
        setData( index(logicalIndex,1), mOptionItem.at(logicalIndex)->value, Qt::EditRole );
        setData( index(logicalIndex,2), mOptionItem.at(logicalIndex)->text, Qt::EditRole );
        setHeaderData( logicalIndex, Qt::Vertical,  mCheckState[logicalIndex], Qt::CheckStateRole );
    } else {  // to comment
        QString formattedOption = mOptionTokenizer->formatOption(mOptionItem.at(logicalIndex), true);
        mOptionItem.at(logicalIndex)->key = "";
        mOptionItem.at(logicalIndex)->value = "";
        mOptionItem.at(logicalIndex)->text = formattedOption;
        mOptionItem.at(logicalIndex)->error = No_Error;
        mOptionItem.at(logicalIndex)->disabled = true;
        updateCheckState();
        setData( index(logicalIndex,0), mOptionItem.at(logicalIndex)->key, Qt::EditRole );
        setData( index(logicalIndex,1), mOptionItem.at(logicalIndex)->value, Qt::EditRole );
        setData( index(logicalIndex,2), mOptionItem.at(logicalIndex)->text, Qt::EditRole );
        setHeaderData( logicalIndex, Qt::Vertical,  mCheckState[logicalIndex], Qt::CheckStateRole );
    }
    emit solverOptionModelChanged(mOptionItem);
}

void SolverOptionTableModel::setRowCount(int rows)
{
    int rc = mOptionItem.size();
    if (rows < 0 ||  rc == rows)
       return;

    if (rc < rows)
       insertRows(qMax(rc, 0), rows - rc);
    else
        removeRows(qMax(rows, 0), rc - rows);
}

void SolverOptionTableModel::updateCheckState()
{
    for(int i = 0; i<mOptionItem.size(); ++i) {
        QVariant value =  QVariant(Qt::Unchecked);
        if (mOptionItem.at(i)->disabled)
            value = QVariant(Qt::PartiallyChecked);
        else if (mOptionItem.at(i)->error == No_Error)
                value = QVariant(Qt::Unchecked);
        else
            value = QVariant(Qt::Checked);

        mCheckState[i] = value;
    }
}

} // namepsace option
} // namespace studio
} // namespace gams
