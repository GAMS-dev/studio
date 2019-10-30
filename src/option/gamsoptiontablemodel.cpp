/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "option.h"
#include "gamsoptiontablemodel.h"

namespace gams {
namespace studio {
namespace option {

GamsOptionTableModel::GamsOptionTableModel(const QString normalizedCommandLineStr, OptionTokenizer* tokenizer, QObject* parent):
    QAbstractTableModel(parent), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption()), mTokenizerUsed(true)
{
    mHeader << "Key"  << "Value" << "Debug Entry";

    if (!normalizedCommandLineStr.simplified().isEmpty())
        on_optionTableModelChanged(normalizedCommandLineStr);
}

GamsOptionTableModel::GamsOptionTableModel(const QList<OptionItem> itemList, OptionTokenizer *tokenizer, QObject *parent):
    QAbstractTableModel(parent), mOptionItem(itemList), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption()), mTokenizerUsed(false)
{
    mHeader << "Key"  << "Value" << "Debug Entry";
}

QVariant GamsOptionTableModel::headerData(int index, Qt::Orientation orientation, int role) const
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

int GamsOptionTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return  mOptionItem.size();
}

int GamsOptionTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant GamsOptionTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (mOptionItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        if (col==0) {
            return mOptionItem.at(row).key;
        } else if (col== 1) {
                 return mOptionItem.at(row).value;
        } else if (col==2) {
            QString key = mOptionItem.at(row).key;
            if (mOption->isASynonym(mOptionItem.at(row).key))
                key = mOption->getNameFromSynonym(mOptionItem.at(row).key);
            if (mOption->isValid(key) || mOption->isASynonym(key))
                return QVariant(mOption->getOptionDefinition(key).number);
            else
                return QVariant(-1);
        } else {
            break;
        }
    }
    case Qt::TextAlignmentRole: {
        return Qt::AlignLeft+ Qt::AlignVCenter;
    }
//    case Qt::DecorationRole
    case Qt::ToolTipRole: {
//        if (Qt::CheckState(mCheckState[index.row()].toUInt()))
//            return QString("'%1' has been disabled").arg(mOptionItem.at(row).key);
        if (col==0) {
            if ( mOption->isDoubleDashedOption(mOptionItem.at(row).key) ) {
                if (!mOption->isDoubleDashedOptionNameValid( mOption->getOptionKey(mOptionItem.at(row).key))) {
                    return QString("'%1' is an invalid double dashed option (Either start with other character than [a-z or A-Z], or a subsequent character is not one of (a-z, A-Z, 0-9, or _))").arg(mOption->getOptionKey(mOptionItem.at(row).key));
                } else {
                    break;
                }
            } else if ( !mOption->isValid(mOptionItem.at(row).key) &&
                        !mOption->isASynonym(mOptionItem.at(row).key)
                      )  {
                         return QString("'%1' is an unknown option Key").arg(mOptionItem.at(row).key);
            } else if (mOption->isDeprecated(mOptionItem.at(row).key)) {
                      return QString("Option '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(row).key);
            }
        } else if (col==1) {
            switch (mOption->getValueErrorType(mOptionItem.at(row).key, mOptionItem.at(row).value)) {
              case Incorrect_Value_Type:
                   return QString("Option key '%1' has an incorrect value type").arg(mOptionItem.at(row).key);
              case Value_Out_Of_Range:
                   return QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(row).value).arg(mOptionItem.at(row).key);
              default:
                   break;
            }
        }
        break;
    }
    case Qt::TextColorRole: {
//        if (Qt::CheckState(headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toBool()))
//            return QVariant::fromValue(QColor(Qt::gray));

        if (mOption->isDoubleDashedOption(mOptionItem.at(row).key)) { // double dashed parameter
            if (!mOption->isDoubleDashedOptionNameValid( mOption->getOptionKey(mOptionItem.at(row).key)) )
                return QVariant::fromValue(QColor(Qt::red));
            else
                 return QVariant::fromValue(QColor(Qt::black));
        }
        if (mOption->isValid(mOptionItem.at(row).key) || mOption->isASynonym(mOptionItem.at(row).key)) { // valid option
            if (col==0) { // key
                if (mOption->isDeprecated(mOptionItem.at(row).key)) { // deprecated option
                    return QVariant::fromValue(QColor(Qt::gray));
                } else {
                    return  QVariant::fromValue(QColor(Qt::black));
                }
            } else { // value
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
        } else { // invalid option
            if (col == 0)
               return QVariant::fromValue(QColor(Qt::red));
            else
                return QVariant::fromValue(QColor(Qt::black));
        }

     }
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags GamsOptionTableModel::flags(const QModelIndex &index) const
{
   Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::NoItemFlags | Qt::ItemIsDropEnabled ;
    else
        return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
}

bool GamsOptionTableModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    emit headerDataChanged(orientation, index, index);

    return true;
}

bool GamsOptionTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole)   {
        QString dataValue = value.toString().simplified();
        if (dataValue.isEmpty())
            return false;

        if (index.row() > mOptionItem.size())
            return false;

        if (index.column() == 0) { // key
            QString from = data(index, Qt::DisplayRole).toString();
            mOptionItem[index.row()].key = dataValue;
            if (QString::compare(from, dataValue, Qt::CaseInsensitive)!=0)
                emit optionNameChanged(from, dataValue);
        } else if (index.column() == 1) { // value
                  mOptionItem[index.row()].value = dataValue;
                  emit optionValueChanged(index);
        }
        emit optionModelChanged(  mOptionItem );
    } else if (role == Qt::CheckStateRole) {
        if (index.row() > mOptionItem.size())
            return false;

        mOptionItem[index.row()].disabled = value.toBool();
        emit optionModelChanged(  mOptionItem );
    }
    emit dataChanged(index, index);
    return true;
}

QModelIndex GamsOptionTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

bool GamsOptionTableModel::insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
{
    Q_UNUSED(parent)
    if (count < 1 || row < 0 || row > mOptionItem.size())
         return false;

     beginInsertRows(QModelIndex(), row, row + count - 1);
     if (mOptionItem.size() == row)
         mOptionItem.append(OptionItem(OptionTokenizer::keyGeneratedStr, OptionTokenizer::valueGeneratedStr, -1, -1));
     else
         mOptionItem.insert(row, OptionItem(OptionItem(OptionTokenizer::keyGeneratedStr,  OptionTokenizer::valueGeneratedStr, -1, -1)));

    endInsertRows();
    emit optionModelChanged(mOptionItem);
    return true;
}

bool GamsOptionTableModel::removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
{
    Q_UNUSED(parent)
    if (count < 1 || row < 0 || row > mOptionItem.size() || mOptionItem.size() ==0)
         return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i=row+count-1; i>=row; --i) {
        mOptionItem.removeAt(i);
    }
    endRemoveRows();
    emit optionModelChanged(mOptionItem);
    return true;
}

bool GamsOptionTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (mOptionItem.size() == 0 || count < 1 || destinationChild < 0 ||  destinationChild > mOptionItem.size())
         return false;

    Q_UNUSED(sourceParent)
    Q_UNUSED(destinationParent)
    beginMoveRows(QModelIndex(), sourceRow, sourceRow  + count - 1, QModelIndex(), destinationChild);
    mOptionItem.insert(destinationChild, mOptionItem.at(sourceRow));
    int removeIndex = destinationChild > sourceRow ? sourceRow : sourceRow+1;
    mOptionItem.removeAt(removeIndex);
    endMoveRows();
    emit optionModelChanged(mOptionItem);
    return true;
}

QStringList GamsOptionTableModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.gams-pf.text";
    return types;
}

QMimeData *GamsOptionTableModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            if (index.column()>0) {
                continue;
            }

            QModelIndex valueIndex = index.sibling(index.row(), 1);
            QString text = QString("%1=%2").arg(data(index, Qt::DisplayRole).toString()).arg(data(valueIndex, Qt::DisplayRole).toString());
            stream << text;
        }
    }

    mimeData->setData("application/vnd.gams-pf.text", encodedData);
    return mimeData;
}

Qt::DropActions GamsOptionTableModel::supportedDragActions() const
{
    return Qt::MoveAction ;
}

Qt::DropActions GamsOptionTableModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction ;
}

bool GamsOptionTableModel::dropMimeData(const QMimeData* mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column)
    if (action == Qt::IgnoreAction)
        return true;
    if (!mimedata->hasFormat("application/vnd.gams-pf.text"))
        return false;

    QByteArray encodedData = mimedata->data("application/vnd.gams-pf.text");
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

    if (row != -1) {
        beginRow = row;
    } else if (parent.isValid()) {
        beginRow = parent.row();
    } else {
        beginRow = rowCount(QModelIndex());
    }

    if (action ==  Qt::CopyAction) {

        QList<int> insertRowList;
        insertRows(beginRow, rows, QModelIndex());

        for (const QString &text : newItems) {          
            insertRowList.append( beginRow );

            QStringList textList = text.split("=");
            QModelIndex idx = index(beginRow, COLUMN_OPTION_KEY, QModelIndex());
            setData(idx, textList.at(0), Qt::EditRole);
            idx = index(beginRow, COLUMN_OPTION_VALUE, QModelIndex());
            setData(idx, textList.at(1), Qt::EditRole);
            idx = index(beginRow, COLUMN_ENTRY_NUMBER, QModelIndex());
            setData(idx, textList.at(2), Qt::EditRole);
            emit newTableRowDropped(index(beginRow, 0, QModelIndex()));
            beginRow++;
        }

        for (const QString &text : newItems) {
            QStringList textList = text.split("=");
            QModelIndex idx;
            for(int i=0; i<rowCount(); ++i) {
                if (insertRowList.contains(i))
                    continue;

                idx = index(i, COLUMN_OPTION_KEY, QModelIndex());
                QString key = data(idx, Qt::DisplayRole).toString();
                if (QString::compare(key, textList.at(0), Qt::CaseInsensitive)==0)
                    break;
            }
            if (idx.row() == rowCount())
               removeRows(idx.row(), COLUMN_OPTION_VALUE, QModelIndex());
        }
        return true;

    }

    return false;
}

QList<OptionItem> GamsOptionTableModel::getCurrentListOfOptionItems()
{
    return mOptionItem;
}

void GamsOptionTableModel::toggleActiveOptionItem(int index)
{
    if (mOptionItem.isEmpty() || index >= mOptionItem.size())
        return;

    bool checked = (headerData(index, Qt::Vertical, Qt::CheckStateRole).toUInt() != Qt::Checked) ? true : false;
    setHeaderData( index, Qt::Vertical,
                          Qt::CheckState(headerData(index, Qt::Vertical, Qt::CheckStateRole).toInt()),
                          Qt::CheckStateRole );
    setData(QAbstractTableModel::createIndex(index, 0), QVariant(checked), Qt::CheckStateRole);
    emit optionModelChanged(mOptionItem);
}

void GamsOptionTableModel::on_optionTableModelChanged(const QString &text)
{
    beginResetModel();
    itemizeOptionFromCommandLineStr(text);
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
    endResetModel();
    emit optionModelChanged(mOptionItem);
}


void GamsOptionTableModel::setRowCount(int rows)
{
   int rc = mOptionItem.size();
   if (rows < 0 ||  rc == rows)
      return;

   if (rc < rows)
      insertRows(qMax(rc, 0), rows - rc);
   else
      removeRows(qMax(rows, 0), rc - rows);
}

void GamsOptionTableModel::itemizeOptionFromCommandLineStr(const QString text)
{
    QMap<int, QVariant> previousCheckState = mCheckState;
    mOptionItem.clear();
    mOptionItem = mOptionTokenizer->tokenize(text);
    for(int idx = 0; idx<mOptionItem.size(); ++idx) {
       mCheckState[idx] = QVariant();
    }

}

} // namepsace option
} // namespace studio
} // namespace gams
