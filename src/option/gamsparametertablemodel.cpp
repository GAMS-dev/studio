/*
 * This file is part of the GAMS Studio project.
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
#include <QIcon>
#include <QMessageBox>
#include <QApplication>

#include "option.h"
#include "theme.h"
#include "gamsparametertablemodel.h"

namespace gams {
namespace studio {
namespace option {

GamsParameterTableModel::GamsParameterTableModel(const QString &normalizedCommandLineStr, OptionTokenizer* tokenizer, QObject* parent):
    QAbstractTableModel(parent), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption()), mTokenizerUsed(true)
{
    mHeader << "Key"  << "Value" << "Debug Entry";

    if (!normalizedCommandLineStr.simplified().isEmpty())
        on_ParameterTableModelChanged(normalizedCommandLineStr);
}

GamsParameterTableModel::GamsParameterTableModel(const QList<OptionItem> &itemList, OptionTokenizer *tokenizer, QObject *parent):
    QAbstractTableModel(parent), mOptionItem(itemList), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption()), mTokenizerUsed(false)
{
    mHeader << "Key"  << "Value" << "Debug Entry";
}

QVariant GamsParameterTableModel::headerData(int index, Qt::Orientation orientation, int role) const
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
        if (mOptionItem.at(index).recurrent) {
            if (Qt::CheckState(mCheckState[index].toUInt())==Qt::Checked)
                return QVariant::fromValue(Theme::icon(":/img/square-red-yellow"));
            else if (Qt::CheckState(mCheckState[index].toUInt())==Qt::PartiallyChecked)
                    return QVariant::fromValue(Theme::icon(":/img/square-gray-yellow"));
            else
                return QVariant::fromValue(Theme::icon(":/img/square-green-yellow"));
        } else {
            if (Qt::CheckState(mCheckState[index].toUInt())==Qt::Checked)
                return QVariant::fromValue(Theme::icon(":/img/square-red"));
            else if (Qt::CheckState(mCheckState[index].toUInt())==Qt::PartiallyChecked)
                    return QVariant::fromValue(Theme::icon(":/img/square-gray"));
            else
                return QVariant::fromValue(Theme::icon(":/img/square-green"));
        }
    case Qt::ToolTipRole:
        QString tooltipText = "";
        switch(mOptionItem.at(index).error) {
        case OptionErrorType::Missing_Value:
            tooltipText.append( QString("Missing value for Parameter key '%1'").arg(mOptionItem.at(index).key) );
            break;
        case OptionErrorType::Invalid_Key:
            tooltipText.append( QString("Unknown parameter '%1'").arg(mOptionItem.at(index).key) );
            break;
        case OptionErrorType::Incorrect_Value_Type:
            tooltipText.append( QString("Parameter key '%1' has a value of incorrect type").arg(mOptionItem.at(index).key) );
            break;
        case OptionErrorType::Value_Out_Of_Range:
            tooltipText.append( QString("Value '%1' for parameter key '%2' is out of range").arg(mOptionItem.at(index).value, mOptionItem.at(index).key) );
            break;
        case OptionErrorType::Deprecated_Option:
            tooltipText.append( QString("Parameter '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(index).key) );
            break;
        default:
            break;
        }
        if (mOptionItem.at(index).recurrent) {
            if (!tooltipText.isEmpty())
                tooltipText.append("\n");
            tooltipText.append( QString("Recurrent parameter '%1', only last entry of same parameters will not be ignored").arg(mOptionItem.at(index).key));
        }
        return tooltipText;
    }
    return QVariant();
}

int GamsParameterTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return  mOptionItem.size();
}

int GamsParameterTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant GamsParameterTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (mOptionItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        if (col==GamsParameterTableModel::COLUMN_OPTION_KEY) {
            return mOptionItem.at(row).key;
        } else if (col== GamsParameterTableModel::COLUMN_OPTION_VALUE) {
                 return mOptionItem.at(row).value;
        } else if (col==GamsParameterTableModel::COLUMN_ENTRY_NUMBER) {
                  return mOptionItem.at(row).optionId;
        } else {
            break;
        }
    }
    case Qt::TextAlignmentRole: {
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
//    case Qt::DecorationRole
    case Qt::ToolTipRole: {
        QString tooltipText = "";
        switch(mOptionItem.at(row).error) {
        case OptionErrorType::Missing_Value:
            tooltipText.append( QString("Missing value for Parameter key '%1'").arg(mOptionItem.at(row).key) );
            break;
        case OptionErrorType::Invalid_Key:
            tooltipText.append( QString("Unknown parameter '%1'").arg(mOptionItem.at(row).key));
            break;
        case OptionErrorType::Incorrect_Value_Type:
            tooltipText.append( QString("Parameter key '%1' has a value of incorrect type").arg(mOptionItem.at(row).key) );
            break;
        case OptionErrorType::Value_Out_Of_Range:
            tooltipText.append( QString("Value '%1' for parameter key '%2' is out of range").arg(mOptionItem.at(row).value, mOptionItem.at(row).key) );
            break;
        case OptionErrorType::Deprecated_Option:
            tooltipText.append( QString("Parameter '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(row).key) );
            break;
        case OptionErrorType::UserDefined_Error:
            tooltipText.append( QString("Invalid parameter key or value or comment defined") );
            break;
        case OptionErrorType::Invalid_minVersion:
            tooltipText.append( QString("Invalid minVersion format, must be [x[.y[.z]]") );
            break;
        case OptionErrorType::Invalid_maxVersion:
            tooltipText.append( QString("Invalid maxVersion format, must be [x[.y[.z]]") );
            break;
        default:
            break;
        }
        if (mOptionItem.at(row).recurrent) {
            if (!tooltipText.isEmpty())
                tooltipText.append("\n");
            tooltipText.append( QString("Recurrent parameter '%1', only last entry of same parameters will not be ignored").arg(mOptionItem.at(row).key));
        }
        return tooltipText;
    }
    case Qt::ForegroundRole: {
//        if (Qt::CheckState(headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toBool()))
//            return QVariant::fromValue(QColor(Qt::gray));

        if (mOptionItem[index.row()].recurrent && index.column()==COLUMN_OPTION_KEY)
            return QVariant::fromValue(QColor(Qt::darkYellow));

        QString key = mOptionItem.at(row).key;
        if (mOption->isDoubleDashedOption(key)) { // double dashed parameter
            if (!mOption->isDoubleDashedOptionNameValid( mOption->getOptionKey(key)) )
                return QVariant::fromValue(Theme::color(Theme::Normal_Red));
            else
                 return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        } else {
             if (key.startsWith("-") || key.startsWith("/"))
                  key = mOptionItem.at(row).key.mid(1);
             if (mOption->isASynonym(key))
                key = mOption->getNameFromSynonym(key);
        }
        if (mOption->isValid(key) || mOption->isASynonym(key)) { // valid option
            if (col==GamsParameterTableModel::COLUMN_OPTION_KEY) { // key
                if (mOption->isDeprecated(key)) { // deprecated option
                    return QVariant::fromValue(QColor(Qt::gray));
                }  else if (mOptionItem.at(row).value.simplified().isEmpty()) {
                        return QVariant::fromValue(Theme::color(Theme::Active_Gray));
                } else {
                    return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
                }
            } else { // value
                  switch (mOption->getValueErrorType(key, mOptionItem.at(row).value)) {
                      case OptionErrorType::Missing_Value:
                      case OptionErrorType::Incorrect_Value_Type:
                      case OptionErrorType::Value_Out_Of_Range:
                            return QVariant::fromValue(Theme::color(Theme::Normal_Red));
                      case OptionErrorType::No_Error:
                            return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
                      default:
                           return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
                  }
            }
        } else { // invalid option
            if (col == GamsParameterTableModel::COLUMN_OPTION_KEY)
               return QVariant::fromValue(Theme::color(Theme::Normal_Red));
            else
                return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        }

     }
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags GamsParameterTableModel::flags(const QModelIndex &index) const
{
   Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::NoItemFlags | Qt::ItemIsDropEnabled ;
    else
        return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
}

bool GamsParameterTableModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    emit headerDataChanged(orientation, index, index);

    return true;
}

bool GamsParameterTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole)   {
        QString dataValue = value.toString().simplified();
        if (dataValue.isEmpty())
            return false;

        if (index.row() > mOptionItem.size())
            return false;

        if (index.column() == COLUMN_OPTION_KEY) { // key
            QString from = data(index, Qt::DisplayRole).toString();
            mOptionItem[index.row()].key = dataValue;
            if (QString::compare(from, dataValue, Qt::CaseInsensitive)!=0)
                emit optionNameChanged(from, dataValue);
        } else if (index.column() == COLUMN_OPTION_VALUE) { // value
                  mOptionItem[index.row()].value = dataValue;
                  emit optionValueChanged(index);
        } else if (index.column() == COLUMN_ENTRY_NUMBER) {
                  mOptionItem[index.row()].optionId = dataValue.toInt();
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

QModelIndex GamsParameterTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

bool GamsParameterTableModel::insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
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

bool GamsParameterTableModel::removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
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

bool GamsParameterTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
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

QStringList GamsParameterTableModel::mimeTypes() const
{
    QStringList types;
    types << optionMimeType(OptionDefinitionType::GamsOptionDefinition);
    return types;
}

QMimeData *GamsParameterTableModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QDataStream::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            if (index.column()>0) {
                continue;
            }

            QModelIndex valueIndex = index.sibling(index.row(), 1);
            QString text = QString("%1=%2").arg(data(index, Qt::DisplayRole).toString(), data(valueIndex, Qt::DisplayRole).toString());
            stream << text;
        }
    }

    mimeData->setData(optionMimeType(OptionDefinitionType::GamsOptionDefinition), encodedData);
    return mimeData;
}

Qt::DropActions GamsParameterTableModel::supportedDragActions() const
{
    return Qt::MoveAction ;
}

Qt::DropActions GamsParameterTableModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction ;
}

bool GamsParameterTableModel::dropMimeData(const QMimeData* mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column)
    if (action == Qt::IgnoreAction)
        return true;
    if (!mimedata->hasFormat(optionMimeType(OptionDefinitionType::GamsOptionDefinition)))
        return false;

    QByteArray encodedData = mimedata->data(optionMimeType(OptionDefinitionType::GamsOptionDefinition));
    QDataStream stream(&encodedData, QDataStream::ReadOnly);
    QStringList newItems;

    while (!stream.atEnd()) {
       QString text;
       stream >> text;
       newItems << text;
    }

    int beginRow = -1;

    if (row != -1) {
        beginRow = row;
    } else if (parent.isValid()) {
        beginRow = parent.row();
    } else {
        beginRow = rowCount(QModelIndex());
    }

//    StudioSettings* settings = Settings::settings();
    if (action ==  Qt::CopyAction) {

        QList<OptionItem> itemList;
        QList<int> overrideIdRowList;
        for (const QString &text : std::as_const(newItems)) {
            QStringList textList = text.split("=");
            int optionid = mOption->getOptionDefinition(textList.at(0)).number;
            itemList.append(OptionItem(optionid, textList.at( COLUMN_OPTION_KEY ), textList.at( COLUMN_OPTION_VALUE )));
            QModelIndexList indices = match(index(GamsParameterTableModel::COLUMN_OPTION_KEY, GamsParameterTableModel::COLUMN_ENTRY_NUMBER), Qt::DisplayRole,
                                            QVariant(optionid), Qt::MatchRecursive);
//          if (settings && settings->overridExistingOption()) {
              for(QModelIndex idx : std::as_const(indices)) { overrideIdRowList.append(idx.row()); }
//          }
         }
         std::sort(overrideIdRowList.begin(), overrideIdRowList.end());

         bool replaceExistingEntry = false;
         bool singleEntryExisted = (overrideIdRowList.size()==1);
         bool multipleEntryExisted = (overrideIdRowList.size()>1);
         if (singleEntryExisted) {
             QMessageBox msgBox;
             msgBox.setWindowTitle("Parameter Entry exists");
             msgBox.setText("Parameter '" + data(index(overrideIdRowList.at(0), COLUMN_OPTION_KEY)).toString()+ "' already exists.");
             msgBox.setInformativeText("How do you want to proceed?");
             msgBox.setDetailedText(QString("Entry:  '%1'\nDescription:  %2 %3").arg(getParameterTableEntry(overrideIdRowList.at(0)),
                    "When running GAMS with multiple entries of the same parameter, only the value of the last entry will be utilized by GAMS.",
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
                itemList.clear();
                return false;
             }
         } else if (multipleEntryExisted) {
             QMessageBox msgBox;
             msgBox.setWindowTitle("Multiple Parameter Entries exist");
             msgBox.setText(QString("%1 entries of Parmaeter '%2' already exist.").arg(overrideIdRowList.size())
                      .arg(data(index(overrideIdRowList.at(0), COLUMN_OPTION_KEY)).toString()));
             msgBox.setInformativeText("How do you want to proceed?");
             QString entryDetailedText = QString("Entries:\n");
             int i = 0;
             for (int id : overrideIdRowList)
                 entryDetailedText.append(QString("   %1. '%2'\n").arg(++i).arg(getParameterTableEntry(id)));
             msgBox.setDetailedText(QString("%1Description:  %2 %3").arg(entryDetailedText)
                      .arg("When running GAMS with multiple entries of the same parameter, only the value of the last entry will be utilized by the GAMS.",
                           "The value of all other entries except the last entry will be ignored."));
             msgBox.setText("Multiple entries of Parameter '" + data(index(overrideIdRowList.at(0), COLUMN_OPTION_KEY)).toString() + "' already exist.");
             msgBox.setInformativeText("How do you want to proceed?");
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
                         removeRows( current, 1 );
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
                 itemList.clear();
                 return false;
             }
             }
         } // else entry not exist

         for (const OptionItem &item : itemList) {
             if (!replaceExistingEntry)
                 insertRows(beginRow, 1, QModelIndex());

             QModelIndex idx = index(beginRow, COLUMN_OPTION_KEY);
             setData(idx, item.key, Qt::EditRole);
             setData( index(beginRow, COLUMN_OPTION_VALUE), item.value, Qt::EditRole);
             setData( index(beginRow, COLUMN_ENTRY_NUMBER), item.optionId, Qt::EditRole);
             setHeaderData( idx.row(), Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
             emit newTableRowDropped( idx );
             beginRow++;
         }

         itemList.clear();
         return true;
    } else {
         return false;
    }
}

QList<OptionItem> GamsParameterTableModel::getCurrentListOfOptionItems()
{
    return mOptionItem;
}

QString GamsParameterTableModel::getParameterTableEntry(int row)
{
    QModelIndex keyIndex = index(row, COLUMN_OPTION_KEY);
    QVariant optionKey = data(keyIndex, Qt::DisplayRole);
    QModelIndex valueIndex = index(row, COLUMN_OPTION_VALUE);
    QVariant optionValue = data(valueIndex, Qt::DisplayRole);
    return QString("%1%2%3").arg(optionKey.toString(), mOptionTokenizer->getOption()->getDefaultSeparator(), optionValue.toString());
}

void GamsParameterTableModel::toggleActiveOptionItem(int index)
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

void GamsParameterTableModel::on_ParameterTableModelChanged(const QString &text)
{
    beginResetModel();
    itemizeOptionFromCommandLineStr(text);
    mOptionTokenizer->validateOption(mOptionItem);

    setRowCount(mOptionItem.size());

    for (int i=0; i<mOptionItem.size(); ++i) {
        setData(QAbstractTableModel::createIndex(i, GamsParameterTableModel::COLUMN_OPTION_KEY), QVariant(mOptionItem.at(i).key), Qt::EditRole);
        setData(QAbstractTableModel::createIndex(i, GamsParameterTableModel::COLUMN_OPTION_VALUE), QVariant(mOptionItem.at(i).value), Qt::EditRole);
        setData(QAbstractTableModel::createIndex(i, GamsParameterTableModel::COLUMN_ENTRY_NUMBER), QVariant(mOptionItem.at(i).optionId), Qt::EditRole);
        if (mOptionItem.at(i).error == OptionErrorType::No_Error)
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Unchecked),
                              Qt::CheckStateRole );
        else if (mOptionItem.at(i).error == OptionErrorType::Deprecated_Option)
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


void GamsParameterTableModel::setRowCount(int rows)
{
   int rc = mOptionItem.size();
   if (rows < 0 ||  rc == rows)
      return;

   if (rc < rows)
      insertRows(qMax(rc, 0), rows - rc);
   else
      removeRows(qMax(rows, 0), rc - rows);
}

void GamsParameterTableModel::itemizeOptionFromCommandLineStr(const QString &text)
{
//    QMap<int, QVariant> previousCheckState = mCheckState;
    mOptionItem.clear();
    mOptionItem = mOptionTokenizer->tokenize(text);
    for(int idx = 0; idx<mOptionItem.size(); ++idx) {
       mCheckState[idx] = QVariant();
    }

}

} // namepsace option
} // namespace studio
} // namespace gams
