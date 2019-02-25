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
    if (mOption->isEOLCharDefined()) {
        mHeader << "Option" << "Value" << "Comment" << "Debug Entry";
        columnEntryNumber = 3;
    } else {
         mHeader << "Option" << "Value" << "Debug Entry";
         columnEntryNumber = 2;
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
        if (mOptionItem.at(index)->disabled) {
            if (mOptionItem.at(index)->key.startsWith("*"))
                return QString("%1 %2").arg(mOptionItem.at(index)->key).arg(mOptionItem.at(index)->value.toString());
            else
                return QString("* %1 %2").arg(mOptionItem.at(index)->key).arg(mOptionItem.at(index)->value.toString()); //->text);
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
            if (mOptionItem.at(index)->error == Deprecated_Option)
                return QVariant::fromValue(QIcon(":/img/square-darkyellow"));
            else
                return QVariant::fromValue(QIcon(":/img/square-red"));
        } else if (Qt::CheckState(mCheckState[index].toUInt())==Qt::PartiallyChecked) {
            return QVariant::fromValue(QIcon(":/img/square-gray"));
        } else {
            if (mOptionItem.at(index)->error == Deprecated_Option)
                return QVariant::fromValue(QIcon(":/img/square-square-darkyellow"));
            else
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
        if (col==COLUMN_OPTION_KEY) {
            QString text = mOptionItem.at(row)->key;
            if (mOptionItem.at(row)->key.startsWith("*"))
                text = mOptionItem.at(row)->key.mid(1);
            return QVariant(text);
        } else if (col==COLUMN_OPTION_VALUE) {
                   return mOptionItem.at(row)->value;
        } else {
            if (mOption->isEOLCharDefined()) {
                   if (col==COLUMN_EOL_COMMENT)
                       return QVariant(mOptionItem.at(row)->text);
                   else if (col==columnEntryNumber)
                       return QVariant(mOptionItem.at(row)->optionId);
            } else {
               if (col==columnEntryNumber)
                  return QVariant(mOptionItem.at(row)->optionId);
            }
        }
        break;
    }
    case Qt::TextAlignmentRole: {
        return Qt::AlignLeft;
    }
    case Qt::ToolTipRole: {
        if (mOptionItem.at(row)->disabled) {
            return mOptionItem.at(row)->key;
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
          case UserDefined_Error:
              return QString("Invalid option key or value or comment defined").arg(mOptionItem.at(row)->text);
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
            case UserDefined_Error:
            case Invalid_Key:
            case Incorrect_Value_Type:
            case Value_Out_Of_Range:
                 return QVariant::fromValue(QColor(Qt::red));
            case Deprecated_Option:
                 return QVariant::fromValue(QColor(Qt::darkYellow));
            case No_Error:
                if (mOption->isEOLCharDefined() && col==COLUMN_EOL_COMMENT)
                    return QVariant::fromValue(QColor(Qt::gray));
                else
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
        if (index.column()==COLUMN_OPTION_KEY)
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
     } else if (index.column()==getColumnEntryNumber()) {
                return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
     } else {
         return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
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
    if (index.row() > mOptionItem.size())
        return false;

    QVector<int> roles;
    if (role == Qt::EditRole)   {
        roles = { Qt::EditRole };
        QString dataValue = value.toString();
        if (index.column()==COLUMN_OPTION_KEY) {
            if (mOptionItem[index.row()]->disabled) {
                if (!dataValue.startsWith("*"))
                    mOptionItem[index.row()]->key = QString("* %1").arg(dataValue);
                else
                    mOptionItem[index.row()]->key = dataValue;
            } else {
               mOptionItem[index.row()]->key = dataValue;
            }

        } else if (index.column()==COLUMN_OPTION_VALUE) {
            mOptionItem[index.row()]->value = dataValue;
        } else {
            if (mOption->isEOLCharDefined()) {
                if (index.column()==COLUMN_EOL_COMMENT) {
                    mOptionItem[index.row()]->text = dataValue;
                } else {
                    mOptionItem[index.row()]->optionId = dataValue.toInt();
                }
            } else {
                if (index.column()==columnEntryNumber)
                    mOptionItem[index.row()]->optionId = dataValue.toInt();
//                else
//                    mOptionItem[index.row()]->text = "";
            }
        }
        emit dataChanged(index, index, roles);
    } else if (role == Qt::CheckStateRole) {
        roles = { Qt::CheckStateRole };
        mOptionItem[index.row()]->disabled = (Qt::CheckState(value.toUInt())==Qt::PartiallyChecked);
        mCheckState[index.row()] = value;
        emit dataChanged(index, index, roles);
    }
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
         mOptionItem.append(new SolverOptionItem());
    else
        mOptionItem.insert(row, (new SolverOptionItem()));

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
    emit  solverOptionItemRemoved();
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
       qDebug() << text;
       newItems << text;
       ++rows;
    }

    int beginRow = -1;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
            beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    if (action ==  Qt::CopyAction) {
        disconnect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);

        for (const QString &text : newItems) {
            insertRows(beginRow, 1, QModelIndex());
            if (text.startsWith("*")) {
                QModelIndex idx = index(beginRow, COLUMN_OPTION_KEY);
                setData(idx, text, Qt::EditRole);
                setHeaderData( idx.row(), Qt::Vertical,
                            Qt::CheckState(Qt::PartiallyChecked),
                            Qt::CheckStateRole );
            } else {
                qDebug() << " HEY, split!";
                QStringList textList = text.split("=");
                QModelIndex keyidx = index(beginRow, SolverOptionTableModel::COLUMN_OPTION_KEY);
                setData(keyidx, textList.at( SolverOptionTableModel::COLUMN_OPTION_KEY ), Qt::EditRole);
                QModelIndex validx = index(beginRow, SolverOptionTableModel::COLUMN_OPTION_VALUE);
                setData(validx, textList.at(SolverOptionTableModel::COLUMN_OPTION_VALUE), Qt::EditRole);
                if (addEOLComment) {
                    QModelIndex commentidx = index(beginRow, SolverOptionTableModel::COLUMN_EOL_COMMENT);
                    qDebug() << " add [" << textList.at(SolverOptionTableModel::COLUMN_EOL_COMMENT) << "] into ("
                             << commentidx.row() << "," << commentidx.column() << ")";
                    setData(commentidx, textList.at(SolverOptionTableModel::COLUMN_EOL_COMMENT), Qt::EditRole);
                }
                QModelIndex ididx = index(beginRow, columnEntryNumber);
                setData(ididx, textList.at(columnEntryNumber), Qt::EditRole);
                setHeaderData( validx.row(), Qt::Vertical,
                            Qt::CheckState(Qt::Unchecked),
                            Qt::CheckStateRole );
                emit newTableRowDropped(keyidx);
            }
            beginRow++;
        }
        connect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);
        return true;

    }

    return false;
}

QList<SolverOptionItem *> SolverOptionTableModel::getCurrentListOfOptionItems() const
{
    return mOptionItem;
}

int SolverOptionTableModel::getColumnEntryNumber() const
{
    return columnEntryNumber;
}

void SolverOptionTableModel::setColumnEntryNumber(int column)
{
    columnEntryNumber = column;
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
            setData( index(i, COLUMN_OPTION_KEY), QVariant(mOptionItem.at(i)->key), Qt::EditRole);
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
        } else {
           setData( index(i, COLUMN_OPTION_KEY), QVariant(mOptionItem.at(i)->key), Qt::EditRole);
           setData( index(i, COLUMN_OPTION_VALUE), QVariant(mOptionItem.at(i)->value), Qt::EditRole);
           if (mOption->isEOLCharDefined())
               setData( index(i, COLUMN_EOL_COMMENT), QVariant(mOptionItem.at(i)->text), Qt::EditRole);
           setData( index(i, getColumnEntryNumber()), QVariant(mOptionItem.at(i)->optionId), Qt::EditRole);
           if (mOptionItem.at(i)->error == No_Error)
               setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Unchecked),
                              Qt::CheckStateRole );
           else
               setHeaderData( i, Qt::Vertical,
                          Qt::CheckState(Qt::Checked),
                          Qt::CheckStateRole );
        }
    }
    emit solverOptionModelChanged(mOptionItem);
    endResetModel();
}

void SolverOptionTableModel::on_updateSolverOptionItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QModelIndex idx = topLeft;
    int row = idx.row();
    while(row <= bottomRight.row()) {
        idx = index(row++, idx.column());
        if (idx.column()==columnEntryNumber) {
            qDebug() << roles.first() << ", ignore update" << idx.row() << "," << idx.column();
            continue;
        }
        if (roles.first()==Qt::EditRole) {
            qDebug() << roles.first() << ", edit update" << idx.row() << "," << idx.column();
          if (mOptionItem.at(idx.row())->disabled) {
              setHeaderData( idx.row(), Qt::Vertical,
                          Qt::CheckState(Qt::PartiallyChecked),
                          Qt::CheckStateRole );
          } else {
              QString key = data( index(idx.row(), SolverOptionTableModel::COLUMN_OPTION_KEY), Qt::DisplayRole).toString();
              QString value = data( index(idx.row(), SolverOptionTableModel::COLUMN_OPTION_VALUE), Qt::DisplayRole).toString();
              QString text = "";
              if (mOption->isEOLCharDefined()) {
                  text = data( index(idx.row(), SolverOptionTableModel::COLUMN_EOL_COMMENT), Qt::DisplayRole).toString();
              }

              mOptionTokenizer->updateOptionItem(key, value, text, mOptionItem.at(idx.row()));

              if (mOptionItem.at(idx.row())->error == No_Error)
                   setHeaderData( idx.row(), Qt::Vertical,
                          Qt::CheckState(Qt::Unchecked),
                          Qt::CheckStateRole );
               else
                   setHeaderData( idx.row(), Qt::Vertical,
                      Qt::CheckState(Qt::Checked),
                      Qt::CheckStateRole );
          }
          emit solverOptionModelChanged(mOptionItem);
       } else if (roles.first()==Qt::CheckStateRole) {
            qDebug() << roles.first() << ", checkstate update " << idx.row() << "," << idx.column();
                  emit solverOptionModelChanged(mOptionItem);
       }
    }

}

void SolverOptionTableModel::on_removeSolverOptionItem()
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
            else
                setHeaderData( i, Qt::Vertical,
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
        mOptionTokenizer->getOptionItemFromStr(mOptionItem.at(logicalIndex), false, mOptionItem.at(logicalIndex)->key);
        if (mOptionItem.at(logicalIndex)->error == No_Error)
            mCheckState[logicalIndex] = QVariant(Qt::Unchecked);
        else
            mCheckState[logicalIndex] = QVariant(Qt::Checked);
        disconnect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);
        setData( index(logicalIndex, SolverOptionTableModel::COLUMN_OPTION_KEY), mOptionItem.at(logicalIndex)->key, Qt::EditRole );
        setData( index(logicalIndex, SolverOptionTableModel::COLUMN_OPTION_VALUE), mOptionItem.at(logicalIndex)->value, Qt::EditRole );
        if (mOption->isEOLCharDefined())
            setData( index(logicalIndex, SolverOptionTableModel::COLUMN_EOL_COMMENT), mOptionItem.at(logicalIndex)->text, Qt::EditRole );
        connect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);
        setData( index(logicalIndex, columnEntryNumber), mOptionItem.at(logicalIndex)->optionId, Qt::EditRole );
        setHeaderData( logicalIndex, Qt::Vertical,  mCheckState[logicalIndex], Qt::CheckStateRole );
    } else {  // to comment
        QString key;
        if (mOptionItem.at(logicalIndex)->value.toString().isEmpty()) {
            if (mOption->isEOLCharDefined() && !mOptionItem.at(logicalIndex)->text.isEmpty())
                key = QString("%1  %2 %3").arg(mOptionItem.at(logicalIndex)->key)
                                          .arg(mOptionTokenizer->getEOLCommentChar())
                                          .arg(mOptionItem.at(logicalIndex)->text);
            else
                key = QString("%1").arg(mOptionItem.at(logicalIndex)->key);
        } else {
            if (mOption->isEOLCharDefined() && !mOptionItem.at(logicalIndex)->text.isEmpty())
                key = QString("%1 %2  %3 %4").arg(mOptionItem.at(logicalIndex)->key).arg(mOptionItem.at(logicalIndex)->value.toString())
                                             .arg(mOptionTokenizer->getEOLCommentChar()).arg(mOptionItem.at(logicalIndex)->text);
            else
                key = QString("%1 %2").arg(mOptionItem.at(logicalIndex)->key).arg(mOptionItem.at(logicalIndex)->value.toString());
       }
        mOptionItem.at(logicalIndex)->key = key;
        mOptionItem.at(logicalIndex)->value = "";
        mOptionItem.at(logicalIndex)->text = "";
        mOptionItem.at(logicalIndex)->optionId = -1;
        mOptionItem.at(logicalIndex)->disabled = true;
        mCheckState[logicalIndex] = QVariant(Qt::PartiallyChecked);
        disconnect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);
        setData( index(logicalIndex, SolverOptionTableModel::COLUMN_OPTION_KEY), mOptionItem.at(logicalIndex)->key, Qt::EditRole );
        setData( index(logicalIndex, SolverOptionTableModel::COLUMN_OPTION_VALUE), mOptionItem.at(logicalIndex)->value, Qt::EditRole );
        if (mOption->isEOLCharDefined())
            setData( index(logicalIndex, SolverOptionTableModel::COLUMN_EOL_COMMENT), mOptionItem.at(logicalIndex)->text, Qt::EditRole );
        connect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);
        setData( index(logicalIndex, columnEntryNumber), mOptionItem.at(logicalIndex)->optionId, Qt::EditRole );
        setHeaderData( logicalIndex, Qt::Vertical,  mCheckState[logicalIndex], Qt::CheckStateRole );
    }
    emit solverOptionItemModelChanged(mOptionItem.at(logicalIndex));
}

void SolverOptionTableModel::on_addCommentAbove_stateChanged(int checkState)
{
    addCommentAbove = (Qt::CheckState(checkState) == Qt::Checked);
}

void SolverOptionTableModel::on_addEOLCommentCheckBox_stateChanged(int checkState)
{
    addEOLComment = (Qt::CheckState(checkState) == Qt::Checked);
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
