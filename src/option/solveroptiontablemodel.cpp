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
#include <QMessageBox>
#include <QApplication>

#include "solveroptiontablemodel.h"
#include "settings.h"
#include "theme.h"

namespace gams {
namespace studio {
namespace option {

SolverOptionTableModel::SolverOptionTableModel(const QList<SolverOptionItem *> &itemList, OptionTokenizer *tokenizer, QObject *parent):
    QAbstractTableModel(parent), mOptionItem(itemList), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption())
{
    if (mOption->isEOLCharDefined()) {
        mHeader << "Key" << "Value" << "Comment" << "Debug Entry";
        columnEntryNumber = 3;
    } else {
         mHeader << "Key" << "Value" << "Debug Entry";
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
        QString lineComment = mOption->isEOLCharDefined() ? QString(mOption->getEOLChars().at(0)) : QString("*");
        if (mOptionItem.at(index)->disabled) {
            if (mOptionItem.at(index)->key.startsWith(lineComment))
                return QString("%1 %2").arg(mOptionItem.at(index)->key, mOptionItem.at(index)->value);
            else
                return QString("%1 %2 %3").arg(lineComment, mOptionItem.at(index)->key, mOptionItem.at(index)->value);
        } else {
            QString tooltipText = "";
            switch(mOptionItem.at(index)->error) {
            case OptionErrorType::Invalid_Key:
                tooltipText.append( QString("Unknown option '%1'").arg(mOptionItem.at(index)->key) );
                break;
            case OptionErrorType::Incorrect_Value_Type:
                tooltipText.append( QString("Option key '%1' has a value of incorrect type").arg(mOptionItem.at(index)->key));
                break;
            case OptionErrorType::Value_Out_Of_Range:
                tooltipText.append( QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(index)->value, mOptionItem.at(index)->key));
                break;
            case OptionErrorType::Deprecated_Option:
                tooltipText.append( QString("Option '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(index)->key));
                break;
            default:
               break;
            }
            if (mOptionItem.at(index)->recurrent) {
              if (!tooltipText.isEmpty())
                  tooltipText.append("\n");
              tooltipText.append( QString("Recurrent option '%1', only last entry of same options will not be ignored").arg(mOptionItem.at(index)->key));
            }
            return tooltipText;
        }
    }
    case Qt::DecorationRole:
        if (Qt::CheckState(mCheckState[index].toUInt())==Qt::Checked) {
            if (mOptionItem.at(index)->recurrent)
               return QVariant::fromValue(Theme::icon(":/img/square-red-yellow"));
            else
                return QVariant::fromValue(Theme::icon(":/img/square-red"));
        } else if (Qt::CheckState(mCheckState[index].toUInt())==Qt::PartiallyChecked) {
                  if (mOptionItem.at(index)->recurrent)
                      return QVariant::fromValue(Theme::icon(":/img/square-gray-yellow"));
                  else
                     return QVariant::fromValue(Theme::icon(":/img/square-gray"));
        } else {
            if (mOptionItem.at(index)->recurrent)
               return QVariant::fromValue(Theme::icon(":/img/square-green-yellow"));
           else
               return QVariant::fromValue(Theme::icon(":/img/square-green"));
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
//            QString lineComment = mOption->isEOLCharDefined() ? QString(mOption->getEOLChars().at(0)) : QString("*");
            if (!text.isEmpty()) {
                if (mOptionTokenizer->isValidLineCommentChar(text.at(0)))
                    text = mOptionItem.at(row)->key.mid(1);

            }
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
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
    case Qt::ToolTipRole: {
        if (mOptionItem.at(row)->disabled) {
            return mOptionItem.at(row)->key;
        } else {
            QString tooltipText = "";
            switch(mOptionItem.at(row)->error) {
            case OptionErrorType::Invalid_Key:
                tooltipText.append( QString("Unknown option '%1'").arg(mOptionItem.at(row)->key));
                break;
            case OptionErrorType::Incorrect_Value_Type:
                tooltipText.append( QString("Option key '%1' has a value of incorrect type").arg(mOptionItem.at(row)->key) );
                break;
            case OptionErrorType::Value_Out_Of_Range:
                tooltipText.append( QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(row)->value, mOptionItem.at(row)->key) );
                break;
            case OptionErrorType::Deprecated_Option:
                tooltipText.append( QString("Option '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(row)->key) );
                break;
            case OptionErrorType::UserDefined_Error:
                tooltipText.append( QString("Invalid option key or value or comment defined") );
                break;
            default:
                break;
            }
            if (mOptionItem.at(row)->recurrent) {
                if (!tooltipText.isEmpty())
                    tooltipText.append("\n");
                tooltipText.append( QString("Recurrent option '%1', only last entry of same options will not be ignored").arg(mOptionItem.at(row)->key));
            }
            return tooltipText;
        }
    }
    case Qt::ForegroundRole: {
        if (mOptionItem.at(row)->disabled) {
            return QVariant::fromValue(Theme::color(Theme::Disable_Gray));
        } else {
            switch(mOptionItem.at(row)->error) {
            case OptionErrorType::UserDefined_Error:
            case OptionErrorType::Invalid_Key:
            case OptionErrorType::Incorrect_Value_Type:
            case OptionErrorType::Value_Out_Of_Range:
                 return QVariant::fromValue(Theme::color(Theme::Normal_Red));
            case OptionErrorType::Deprecated_Option:
                if (mOptionItem.at(row)->recurrent && index.column()==COLUMN_OPTION_KEY)
                    return QVariant::fromValue(Theme::color(Theme::Normal_Yellow));
                else return QVariant::fromValue(Theme::color(Theme::Disable_Gray));
            case OptionErrorType::No_Error:
                if (mOptionItem.at(row)->recurrent && index.column()==COLUMN_OPTION_KEY)
                    return QVariant::fromValue(Theme::color(Theme::Normal_Yellow));
                else if (mOption->isEOLCharDefined() && col==COLUMN_EOL_COMMENT)
                    return QVariant::fromValue(Theme::color(Theme::Disable_Gray));
                else
                    return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
            default:
                if (mOptionItem.at(row)->recurrent && index.column()==COLUMN_OPTION_KEY)
                    return QVariant::fromValue(Theme::color(Theme::Normal_Yellow));
                else
                    return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
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
                QString lineComment = mOption->isEOLCharDefined() ? QString(mOption->getEOLChars().at(0)) : QString("*");
                if (!dataValue.startsWith(lineComment))
                    mOptionItem[index.row()]->key = QString("%1 %2").arg(lineComment, dataValue);
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
    Q_UNUSED(parent)
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
    Q_UNUSED(parent)
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

    Q_UNUSED(sourceParent)
    Q_UNUSED(destinationParent)
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
    types << optionMimeType(OptionDefinitionType::SolverOptionDefinition);
    return types;
}


Qt::DropActions SolverOptionTableModel::supportedDropActions() const
{
    return Qt::CopyAction ;
}

bool SolverOptionTableModel::dropMimeData(const QMimeData* mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column)
    if (action == Qt::IgnoreAction)
        return true;

    if (!mimedata->hasFormat(optionMimeType(OptionDefinitionType::SolverOptionDefinition)))
        return false;

    QByteArray encodedData = mimedata->data(optionMimeType(OptionDefinitionType::SolverOptionDefinition));
    QDataStream stream(&encodedData, QDataStream::ReadOnly);
    QStringList newItems;

    while (!stream.atEnd()) {
       QString text;
       stream >> text;
       newItems << text;
    }

    int beginRow = -1;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
            beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    Settings* settings = Settings::settings();
    if (action ==  Qt::CopyAction) {

        disconnect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);

        QList<SolverOptionItem *> itemList;
        QList<int> overrideIdRowList;
        for (const QString &text : std::as_const(newItems)) {
            QString lineComment = mOption->isEOLCharDefined() ? QString(mOption->getEOLChars().at(0)) : QString("*");
            if (text.startsWith(lineComment)) {
                itemList.append(new SolverOptionItem(-1, text, "", "", true));
            } else {
                QStringList textList = text.split("=");
                int optionid = mOption->getOptionDefinition(textList.at(0)).number;
                itemList.append(new SolverOptionItem(optionid,
                                                     textList.at( COLUMN_OPTION_KEY ),
                                                     textList.at( COLUMN_OPTION_VALUE ),
                                                     textList.at( COLUMN_EOL_COMMENT ),
                                                     false));
                QModelIndexList indices = match(index(0, getColumnEntryNumber()), Qt::DisplayRole, QVariant(optionid), Qt::MatchRecursive);

                if (settings && settings->toBool(skSoOverrideExisting)) {
                    for(const QModelIndex &idx : std::as_const(indices)) { overrideIdRowList.append(idx.row()); }
                }
            }
        }
        std::sort(overrideIdRowList.begin(), overrideIdRowList.end());

        bool replaceExistingEntry = false;
        bool singleEntryExisted = (overrideIdRowList.size()==1);
        bool multipleEntryExisted = (overrideIdRowList.size()>1);
        if (singleEntryExisted) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Option Entry exists");
            msgBox.setText(QString("Option '%1' already exists in your option file.").arg(data(index(overrideIdRowList.at(0), COLUMN_OPTION_KEY)).toString()));
            msgBox.setInformativeText("How do you want to proceed?");
            msgBox.setDetailedText(QString("Entry:  '%1'\nDescription:  %2 %3").arg(getOptionTableEntry(overrideIdRowList.at(0)))
                    .arg("When a solver option file contains multiple entries of the same options, only the value of the last entry will be utilized by the solver.",
                         "The value of all other entries except the last entry will be ignored."));
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Replace existing entry", QMessageBox::ActionRole);
            msgBox.addButton("Add new entry", QMessageBox::ActionRole);

            switch(msgBox.exec()) {
            case 3: // replace
                replaceExistingEntry = true;
                beginRow = overrideIdRowList.at(0);
                break;
            case 4: // add
                break;
            case QMessageBox::Abort:
                qDeleteAll(itemList);
                itemList.clear();
                return false;
            }
        } else if (multipleEntryExisted) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Multiple Option Entries exist");
            msgBox.setText(QString("%1 entries of Option '%2' already exist in your option file.").arg(overrideIdRowList.size())
                     .arg(data(index(overrideIdRowList.at(0), COLUMN_OPTION_KEY)).toString()));
            msgBox.setInformativeText("How do you want to proceed?");
            QString entryDetailedText = QString("Entries:\n");
            int i = 0;
            for (int id : overrideIdRowList)
                entryDetailedText.append(QString("   %1. '%2'\n").arg(++i).arg(getOptionTableEntry(id)));
            msgBox.setDetailedText(QString("%1Description:  %2 %3").arg(entryDetailedText)
                     .arg("When a solver option file contains multiple entries of the same options, only the value of the last entry will be utilized by the solver.",
                          "The value of all other entries except the last entry will be ignored."));
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Replace first entry and delete other entries", QMessageBox::ActionRole);
            msgBox.addButton("Add new entry", QMessageBox::ActionRole);

            switch(msgBox.exec()) {
            case 3: { // delete and replace
                int prev = -1;
                for(int i=overrideIdRowList.count()-1; i>=0; i--) {
                    int current = overrideIdRowList[i];
                    if (i==0)
                        continue;
                    if (current != prev) {
                        QString text = getOptionTableEntry(current);
                        removeRows( current, 1 );
                        mOptionTokenizer->logger()->append(QString("Option entry '%1' has been deleted").arg(text), LogMsgType::Info);
                        prev = current;
                    }
                }

                replaceExistingEntry = true;
                beginRow = overrideIdRowList.at(0);
                break;
            }
            case 4: { // add
                break;
            }
            case QMessageBox::Abort: {
                qDeleteAll(itemList);
                itemList.clear();
                return false;
            }
            }
        } // else entry not exist

        for (SolverOptionItem * item : std::as_const(itemList)) {
            if (item->disabled) {
                insertRows(beginRow, 1, QModelIndex());
                QModelIndex idx = index(beginRow, COLUMN_OPTION_KEY);
                setData(idx, item->key, Qt::EditRole);
                setHeaderData( idx.row(), Qt::Vertical, Qt::CheckState(Qt::PartiallyChecked), Qt::CheckStateRole );
            } else {
                if (!replaceExistingEntry)
                    insertRows(beginRow, 1, QModelIndex());

                QModelIndex idx = index(beginRow, COLUMN_OPTION_KEY);
                setData(idx, item->key, Qt::EditRole);
                setData( index(beginRow, COLUMN_OPTION_VALUE), item->value, Qt::EditRole);
                if (settings && settings->toBool(skSoAddEOLComment)) { //addEOLComment) {
                    setData(index(beginRow, COLUMN_EOL_COMMENT), item->text, Qt::EditRole);
                }
                setData(index(beginRow, columnEntryNumber), item->optionId, Qt::EditRole);
                setHeaderData( idx.row(), Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
                emit newTableRowDropped( idx );
            }
            beginRow++;
        }

        qDeleteAll(itemList);
        itemList.clear();

        connect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);
        updateRecurrentStatus();
        return true;
    }

    return false;
}

QList<SolverOptionItem *> SolverOptionTableModel::getCurrentListOfOptionItems() const
{
    return mOptionItem;
}

QString SolverOptionTableModel::getOptionTableEntry(int row)
{
    QModelIndex keyIndex = index(row, COLUMN_OPTION_KEY);
    QVariant optionKey = data(keyIndex, Qt::DisplayRole);
    if (Qt::CheckState(headerData(row, Qt::Vertical, Qt::CheckStateRole).toInt())==Qt::PartiallyChecked) {
        return QString("%1 %2").arg(mOptionTokenizer->getOption()->isEOLCharDefined() ? QString(mOptionTokenizer->getOption()->getEOLChars().at(0)) :"#",
                                    optionKey.toString());
    } else {
        QModelIndex valueIndex = index(row, COLUMN_OPTION_VALUE);
        QVariant optionValue = data(valueIndex, Qt::DisplayRole);
        QModelIndex commentIndex = index(row, COLUMN_EOL_COMMENT);
        QVariant optionComment = data(commentIndex, Qt::DisplayRole);
        if (mOptionTokenizer->getOption()->isEOLCharDefined() && !optionComment.toString().isEmpty()) {
            return QString("%1%2%3  %4 %5").arg(optionKey.toString(), mOptionTokenizer->getOption()->getDefaultSeparator(),
                                                optionValue.toString(), QString(mOptionTokenizer->getOption()->getEOLChars().at(0)),
                                                optionComment.toString());
        } else {
            return QString("%1%2%3").arg(optionKey.toString(), mOptionTokenizer->getOption()->getDefaultSeparator(), optionValue.toString());
        }
   }
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
    disconnect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);

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
            setData( index(i, getColumnEntryNumber()), QVariant(mOptionItem.at(i)->optionId), Qt::EditRole);
        } else {
           setData( index(i, COLUMN_OPTION_KEY), QVariant(mOptionItem.at(i)->key), Qt::EditRole);
           setData( index(i, COLUMN_OPTION_VALUE), QVariant(mOptionItem.at(i)->value), Qt::EditRole);
           if (mOption->isEOLCharDefined())
               setData( index(i, COLUMN_EOL_COMMENT), QVariant(mOptionItem.at(i)->text), Qt::EditRole);
           setData( index(i, getColumnEntryNumber()), QVariant(mOptionItem.at(i)->optionId), Qt::EditRole);
           if (mOptionItem.at(i)->error == OptionErrorType::No_Error)
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
    updateRecurrentStatus();
    endResetModel();
    connect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateSolverOptionItem);
}

void SolverOptionTableModel::on_updateSolverOptionItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QModelIndex idx = topLeft;
    int row = idx.row();
    while(row <= bottomRight.row()) {
        idx = index(row++, idx.column());
        if (roles.first()==Qt::EditRole) {
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

              if (mOptionTokenizer->getOption()->available())
                  mOptionTokenizer->updateOptionItem(key, value, text, mOptionItem.at(idx.row()));

              if (mOptionItem.at(idx.row())->error == OptionErrorType::No_Error)
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
                  emit solverOptionModelChanged(mOptionItem);
       }
    }
    updateRecurrentStatus();
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
            if (mOptionItem.at(i)->error ==OptionErrorType::No_Error)
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
    updateRecurrentStatus();
    endResetModel();
}

void SolverOptionTableModel::on_toggleRowHeader(int logicalIndex)
{
    if (mCheckState[logicalIndex].toInt() == Qt::PartiallyChecked) { // from comment
        mOptionTokenizer->getOptionItemFromStr(mOptionItem.at(logicalIndex), false, mOptionItem.at(logicalIndex)->key);
        if (mOptionItem.at(logicalIndex)->error == OptionErrorType::No_Error)
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
        if (mOptionItem.at(logicalIndex)->value.isEmpty()) {
            if (mOption->isEOLCharDefined() && !mOptionItem.at(logicalIndex)->text.isEmpty())
                key = QString("%1 %2 %3").arg(mOptionItem.at(logicalIndex)->key,
                                              mOptionTokenizer->getEOLCommentChar(),
                                              mOptionItem.at(logicalIndex)->text);
            else
                key = QString("%1").arg(mOptionItem.at(logicalIndex)->key);
        } else {
            if (mOption->isEOLCharDefined() && !mOptionItem.at(logicalIndex)->text.isEmpty())
                key = QString("%1%2%3 %4 %5").arg(mOptionItem.at(logicalIndex)->key,
                                                  mOption->getDefaultSeparator(),
                                                  mOptionItem.at(logicalIndex)->value,
                                                  mOptionTokenizer->getEOLCommentChar(),
                                                  mOptionItem.at(logicalIndex)->text);
            else
                key = QString("%1%2%3").arg(mOptionItem.at(logicalIndex)->key,
                                            mOption->getDefaultSeparator(),
                                            mOptionItem.at(logicalIndex)->value);
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

void SolverOptionTableModel::on_groupDefinitionReloaded()
{
    emit  solverOptionModelChanged(mOptionItem);
}

void SolverOptionTableModel::updateRecurrentStatus()
{
    QList<int> idList;
    for(SolverOptionItem* item : std::as_const(mOptionItem)) {
        idList << item->optionId;
    }
    for(SolverOptionItem* item : std::as_const(mOptionItem)) {
        item->recurrent = (!item->disabled && item->optionId != -1 && idList.count(item->optionId) > 1);
    }
    emit headerDataChanged(Qt::Vertical, 0, mOptionItem.size());
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
        else if (mOptionItem.at(i)->error == OptionErrorType::No_Error)
                value = QVariant(Qt::Unchecked);
        else
            value = QVariant(Qt::Checked);

        mCheckState[i] = value;
    }
}

} // namepsace option
} // namespace studio
} // namespace gams
