/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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

#include "option/solveroptiontablemodel.h"
#include "settings.h"
#include "theme.h"
#include "msgbox.h"

namespace gams {
namespace studio {
namespace option {

SolverOptionTableModel::SolverOptionTableModel(const QString& callstr,
                                               const QList<SolverOptionItem *> &itemList,
                                               OptionTokenizer *tokenizer,
                                               QObject *parent):
    OptionTableModel(callstr, true, tokenizer, parent),
    mOptionItem(itemList)
{
    if (mOptionTokenizer->getOption()->isEOLCharDefined()) {
        mHeader << "id" << "Key" << "Value" << "Comment" ;
    } else {
         mHeader << "id" << "Key" << "Value";
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
        return (mOptionItem.isEmpty() ? QVariant() : mCheckState[index]);
    case Qt::ToolTipRole:
        return headerTooltip(mOptionItem.at(index)->disabled,
                             (mOptionTokenizer->getOption()->isEOLCharDefined() ? QString(mOptionTokenizer->getOption()->getEOLChars().at(0))
                                                                                : QString("*")),
                             mOptionItem.at(index)->recurrent,
                             mOptionItem.at(index)->error,
                             mOptionItem.at(index)->key,
                             mOptionItem.at(index)->value );
    case Qt::DecorationRole:
        return headerDecoration(mCheckState[index].toUInt(), mOptionItem.at(index)->recurrent);
    default:
        return QVariant();
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
    const int row = index.row();
    const int col = index.column();

    if (mOptionItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        if (col==COLUMN_KEY) {
            QString text = mOptionItem.at(row)->key;
//            QString lineComment = mOptionTokenizer->getOption()->isEOLCharDefined() ? QString(mOptionTokenizer->getOption()->getEOLChars().at(0)) : QString("*");
            if (!text.isEmpty()) {
                if (mOptionTokenizer->isValidLineCommentChar(text.at(0)))
                    text = mOptionItem.at(row)->key.mid(1);
            }
            return QVariant(text);
        } else if (col==COLUMN_VALUE) {
                   return mOptionItem.at(row)->value;
        } else {
            if (mOptionTokenizer->getOption()->isEOLCharDefined()) {
                   if (col==COLUMN_EOL_COMMENT)
                       return QVariant(mOptionItem.at(row)->text);
                   else if (col==column_id())
                       return QVariant(mOptionItem.at(row)->optionId);
            } else {
                if (col==column_id())
                  return QVariant(mOptionItem.at(row)->optionId);
            }
        }
        break;
    }
    case Qt::TextAlignmentRole: {
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
    case Qt::ToolTipRole: {
        return dataTooltip(mOptionItem.at(row)->disabled, mOptionItem.at(row)->recurrent,
                           mOptionItem.at(row)->error, mOptionItem.at(row)->key, mOptionItem.at(row)->value);
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
                if (mOptionItem.at(row)->recurrent && index.column()==COLUMN_KEY)
                    return QVariant::fromValue(Theme::color(Theme::Normal_Yellow));
                else return QVariant::fromValue(Theme::color(Theme::Disable_Gray));
            case OptionErrorType::No_Error:
                if (mOptionItem.at(row)->recurrent && index.column()==COLUMN_KEY)
                    return QVariant::fromValue(Theme::color(Theme::Normal_Yellow));
                else if (mOptionTokenizer->getOption()->isEOLCharDefined() && col==COLUMN_EOL_COMMENT)
                    return QVariant::fromValue(Theme::color(Theme::Disable_Gray));
                else
                    return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
            default:
                if (mOptionItem.at(row)->recurrent && index.column()==COLUMN_KEY)
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

bool SolverOptionTableModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    mOptionItem.at(index)->disabled = (value.toInt()==Qt::CheckState(Qt::PartiallyChecked));

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
        const QString dataValue = value.toString();
        if (index.column()==COLUMN_KEY) {
            if (mOptionItem[index.row()]->disabled) {
                const QString lineComment = mOptionTokenizer->getOption()->isEOLCharDefined() ? QString(mOptionTokenizer->getOption()->getEOLChars().at(0)) : QString("*");
                if (!dataValue.startsWith(lineComment))
                    mOptionItem[index.row()]->key = QString("%1 %2").arg(lineComment, dataValue);
                else
                    mOptionItem[index.row()]->key = dataValue;
            } else {
               mOptionItem[index.row()]->key = dataValue;
            }

        } else if (index.column()==COLUMN_VALUE) {
            mOptionItem[index.row()]->value = dataValue;
        } else {
            if (mOptionTokenizer->getOption()->isEOLCharDefined()) {
                if (index.column()==COLUMN_EOL_COMMENT) {
                    mOptionItem[index.row()]->text = dataValue;
                } else {
                    mOptionItem[index.row()]->optionId = dataValue.toInt();
                }
            } else {
                if (index.column()==COLUMN_ID)
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
    if (count < 1 || row < 0 || row > mOptionItem.size() || mOptionItem.isEmpty())
         return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i=row+count-1; i>=row; --i) {
        SolverOptionItem* item = mOptionItem.at(i);
        delete item;
        mOptionItem.removeAt(i);
    }
    endRemoveRows();
    emit optionItemRemoved();
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

        disconnect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateOptionItem);

        QList<SolverOptionItem *> itemList;
        itemList.reserve(newItems.size());
        QList<int> overrideIdRowList;
        for (const QString &text : std::as_const(newItems)) {
            const QString lineComment = mOptionTokenizer->getOption()->isEOLCharDefined() ? QString(mOptionTokenizer->getOption()->getEOLChars().at(0)) : QString("*");
            if (text.startsWith(lineComment)) {
                itemList.append(new SolverOptionItem(-1, text, "", "", true));
            } else {
                const QStringList textList = text.split("=");
                const int optionid = mOptionTokenizer->getOption()->getOptionDefinition(textList.at(COLUMN_KEY)).number;
                itemList.append(new SolverOptionItem(optionid,
                                                     textList.at( COLUMN_KEY ),
                                                     textList.at( COLUMN_VALUE ),
                                                     textList.at( COLUMN_EOL_COMMENT ),
                                                     false));
                QModelIndexList indices = match(index(0, COLUMN_ID), Qt::DisplayRole, QVariant(optionid), Qt::MatchRecursive);

                if (settings && settings->toBool(skSoOverrideExisting)) {
                    for(const QModelIndex &idx : std::as_const(indices)) {
                        overrideIdRowList.append(idx.row());
                    }
                }
                if (settings && settings->toBool(skSoDeleteCommentsAbove)) {
                    for(const QModelIndex &idx : indices) {
                        for(int r=idx.row()-1; r>=0; --r) {
                            if (headerData(r, Qt::Vertical, static_cast<int>(Qt::CheckStateRole)).toInt()!=static_cast<int>(Qt::PartiallyChecked))
                                break;
                            if (!overrideIdRowList.contains(r))
                                overrideIdRowList.append(r);
                        }
                    }
                }
            }
        }
        std::sort(overrideIdRowList.begin(), overrideIdRowList.end());

        bool replaceExistingEntry = false;
        if (overrideIdRowList.size() > 0) {
            int answer = questionEntryExisted(overrideIdRowList);
            if (overrideIdRowList.size()==1) { // singleEntryExisted)
                switch(answer) {
                case 0: // replace
                    replaceExistingEntry = true;
                    beginRow = overrideIdRowList.at(0);
                    break;
                case 1: // add
                    break;
                default:
                    qDeleteAll(itemList);
                    itemList.clear();
                    return false;
                }
            } else if (overrideIdRowList.size()>1) { // multipleEntryExisted
                switch(answer) {
                case 0: { // delete and replace
                    int prev = -1;
                    for(int i=overrideIdRowList.count()-1; i>=0; i--) {
                        const int current = overrideIdRowList[i];
                        if (i==0)
                            continue;
                        if (current != prev) {
                            const QString text = getOptionTableEntry(current);
                            removeRows( current, 1 );
                            mOptionTokenizer->logger()->append(QString(mCallstr+" entry '%1' has been deleted").arg(text), LogMsgType::Info);
                            prev = current;
                        }
                    }

                    replaceExistingEntry = true;
                    beginRow = overrideIdRowList.at(0);
                    break;
                }
                case 1: { // add
                    break;
                }
                default: {
                    qDeleteAll(itemList);
                    itemList.clear();
                    return false;
                }
                }
            }
        } // else entry not exist

        for (SolverOptionItem * item : std::as_const(itemList)) {
            if (item->disabled) {
                insertRows(beginRow, 1, QModelIndex());
                QModelIndex idx = index(beginRow, COLUMN_KEY);
                setData(idx, item->key, Qt::EditRole);
                setHeaderData( idx.row(), Qt::Vertical, Qt::CheckState(Qt::PartiallyChecked), Qt::CheckStateRole );
            } else {
                if (!replaceExistingEntry)
                    insertRows(beginRow, 1, QModelIndex());

                QModelIndex idx = index(beginRow, COLUMN_KEY);
                setHeaderData( idx.row(), Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
                setData(idx, item->key, Qt::EditRole);
                setData( index(beginRow, COLUMN_VALUE), item->value, Qt::EditRole);
                if (mOptionTokenizer->getOption()->isEOLCharDefined()) {
                    if (settings && settings->toBool(skSoAddEOLComment)) { //addEOLComment) {
                        setData(index(beginRow, COLUMN_EOL_COMMENT), item->text, Qt::EditRole);
                    } else {
                        setData(index(beginRow, COLUMN_EOL_COMMENT), "", Qt::EditRole);
                    }
                }
                setData(index(beginRow, COLUMN_ID), item->optionId, Qt::EditRole);
                emit newTableRowDropped( idx );
            }
            beginRow++;
        }

        qDeleteAll(itemList);
        itemList.clear();

        connect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateOptionItem);
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
    const QModelIndex keyIndex = index(row, COLUMN_KEY);
    const QVariant optionKey = data(keyIndex, Qt::DisplayRole);
    if (Qt::CheckState(headerData(row, Qt::Vertical, Qt::CheckStateRole).toInt())==Qt::PartiallyChecked) {
        return QString("%1 %2").arg(mOptionTokenizer->getOption()->isEOLCharDefined() ? QString(mOptionTokenizer->getOption()->getEOLChars().at(0)) :"#",
                                    optionKey.toString());
    } else {
        const QModelIndex valueIndex = index(row, COLUMN_VALUE);
        const QVariant optionValue = data(valueIndex, Qt::DisplayRole);
        if (mOptionTokenizer->getOption()->isEOLCharDefined()) {
            const QModelIndex commentIndex = index(row, COLUMN_EOL_COMMENT);
            const QVariant optionComment = data(commentIndex, Qt::DisplayRole);
            if (!optionComment.toString().isEmpty()) {
                return QString("%1%2%3  %4 %5").arg(optionKey.toString(), mOptionTokenizer->getOption()->getDefaultSeparator(),
                                                optionValue.toString(), QString(mOptionTokenizer->getOption()->getEOLChars().at(0)),
                                                optionComment.toString());
            }
        }
        return QString("%1%2%3").arg(optionKey.toString(), mOptionTokenizer->getOption()->getDefaultSeparator(), optionValue.toString());
   }
}

void SolverOptionTableModel::reloadSolverOptionModel(const QList<SolverOptionItem *> &optionItem)
{
    disconnect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateOptionItem);

    beginResetModel();

    qDeleteAll(mOptionItem);
    mOptionItem.clear();

    mOptionItem = optionItem;
    mOptionTokenizer->validateOption(mOptionItem);
    updateCheckState();

    setRowCount(mOptionItem.size());

    for (int i=0; i<mOptionItem.size(); ++i) {
        if (mOptionItem.at(i)->disabled) {
            setData( index(i, COLUMN_KEY), QVariant(mOptionItem.at(i)->key), Qt::EditRole);
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
            setData( index(i, COLUMN_ID), QVariant(mOptionItem.at(i)->optionId), Qt::EditRole);
        } else {
           setData( index(i, COLUMN_KEY), QVariant(mOptionItem.at(i)->key), Qt::EditRole);
           setData( index(i, COLUMN_VALUE), QVariant(mOptionItem.at(i)->value), Qt::EditRole);
           if (mOptionTokenizer->getOption()->isEOLCharDefined())
               setData( index(i, COLUMN_EOL_COMMENT), QVariant(mOptionItem.at(i)->text), Qt::EditRole);
           setData( index(i, COLUMN_ID), QVariant(mOptionItem.at(i)->optionId), Qt::EditRole);
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
    connect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateOptionItem);
}

void SolverOptionTableModel::on_updateOptionItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
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
              const QString key = data( index(idx.row(), SolverOptionTableModel::COLUMN_KEY), Qt::DisplayRole).toString();
              const QString value = data( index(idx.row(), SolverOptionTableModel::COLUMN_VALUE), Qt::DisplayRole).toString();
              QString text = "";
              if (mOptionTokenizer->getOption()->isEOLCharDefined()) {
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

void SolverOptionTableModel::on_removeOptionItem()
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
        disconnect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateOptionItem);
        setData( index(logicalIndex, SolverOptionTableModel::COLUMN_KEY), mOptionItem.at(logicalIndex)->key, Qt::EditRole );
        setData( index(logicalIndex, SolverOptionTableModel::COLUMN_VALUE), mOptionItem.at(logicalIndex)->value, Qt::EditRole );
        if (mOptionTokenizer->getOption()->isEOLCharDefined())
            setData( index(logicalIndex, SolverOptionTableModel::COLUMN_EOL_COMMENT), mOptionItem.at(logicalIndex)->text, Qt::EditRole );
        connect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateOptionItem);
        setData( index(logicalIndex, COLUMN_ID), mOptionItem.at(logicalIndex)->optionId, Qt::EditRole );
        setHeaderData( logicalIndex, Qt::Vertical,  mCheckState[logicalIndex], Qt::CheckStateRole );
    } else {  // to comment
        QString key;
        if (mOptionItem.at(logicalIndex)->value.isEmpty()) {
            if (mOptionTokenizer->getOption()->isEOLCharDefined() && !mOptionItem.at(logicalIndex)->text.isEmpty())
                key = QString("%1 %2 %3").arg(mOptionItem.at(logicalIndex)->key,
                                              mOptionTokenizer->getEOLCommentChar(),
                                              mOptionItem.at(logicalIndex)->text);
            else
                key = QString("%1").arg(mOptionItem.at(logicalIndex)->key);
        } else {
            if (mOptionTokenizer->getOption()->isEOLCharDefined() && !mOptionItem.at(logicalIndex)->text.isEmpty())
                key = QString("%1%2%3 %4 %5").arg(mOptionItem.at(logicalIndex)->key,
                                                  mOptionTokenizer->getOption()->getDefaultSeparator(),
                                                  mOptionItem.at(logicalIndex)->value,
                                                  mOptionTokenizer->getEOLCommentChar(),
                                                  mOptionItem.at(logicalIndex)->text);
            else
                key = QString("%1%2%3").arg(mOptionItem.at(logicalIndex)->key,
                                            mOptionTokenizer->getOption()->getDefaultSeparator(),
                                            mOptionItem.at(logicalIndex)->value);
        }
        mOptionItem.at(logicalIndex)->key = key;
        mOptionItem.at(logicalIndex)->value = "";
        mOptionItem.at(logicalIndex)->text = "";
        mOptionItem.at(logicalIndex)->optionId = -1;
        mOptionItem.at(logicalIndex)->disabled = true;
        mCheckState[logicalIndex] = QVariant(Qt::PartiallyChecked);
        disconnect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateOptionItem);
        setData( index(logicalIndex, SolverOptionTableModel::COLUMN_KEY), mOptionItem.at(logicalIndex)->key, Qt::EditRole );
        setData( index(logicalIndex, SolverOptionTableModel::COLUMN_VALUE), mOptionItem.at(logicalIndex)->value, Qt::EditRole );
        if (mOptionTokenizer->getOption()->isEOLCharDefined())
            setData( index(logicalIndex, SolverOptionTableModel::COLUMN_EOL_COMMENT), mOptionItem.at(logicalIndex)->text, Qt::EditRole );
        connect(this, &QAbstractTableModel::dataChanged, this, &SolverOptionTableModel::on_updateOptionItem);
        setData( index(logicalIndex, COLUMN_ID), mOptionItem.at(logicalIndex)->optionId, Qt::EditRole );
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
    idList.reserve(mOptionItem.size());
    for (SolverOptionItem* item : std::as_const(mOptionItem)) {
        idList << item->optionId;
    }
    for (SolverOptionItem* item : std::as_const(mOptionItem)) {
        item->recurrent = (!item->disabled && item->optionId != -1 && idList.count(item->optionId) > 1);
    }
    emit headerDataChanged(Qt::Vertical, 0, mOptionItem.size());
}

void SolverOptionTableModel::setRowCount(int rows)
{
    const int rc = mOptionItem.size();
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
