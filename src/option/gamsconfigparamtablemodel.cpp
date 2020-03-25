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
#include "gamsconfigparamtablemodel.h"
#include "scheme.h"

#include <QApplication>
#include <QMessageBox>
#include <QMimeData>

namespace gams {
namespace studio {
namespace option {

GamsConfigParamTableModel::GamsConfigParamTableModel(const QList<ParamConfigItem *> itemList, OptionTokenizer *tokenizer, QObject *parent):
    QAbstractTableModel(parent), mOptionItem(itemList), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption())
{
    mHeader << "Key"  << "Value" << "minVersion" << "maxVersion"  << "Debug Entry";  // TODO (JP)<< "Action";
   // TODO (JP) init data with itemList
}

QVariant GamsConfigParamTableModel::headerData(int index, Qt::Orientation orientation, int role) const
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
            if (mOptionItem.at(index)->recurrent)
               return QVariant::fromValue(Scheme::icon(":/img/square-red-yellow"));
            else
               return QVariant::fromValue(Scheme::icon(":/img/square-red"));
        } else if (Qt::CheckState(mCheckState[index].toUInt())==Qt::PartiallyChecked) {
                  if (mOptionItem.at(index)->recurrent)
                     return QVariant::fromValue(Scheme::icon(":/img/square-gray-yellow"));
                  else
                     return QVariant::fromValue(Scheme::icon(":/img/square-gray"));
        } else {
            if (mOptionItem.at(index)->recurrent)
                return QVariant::fromValue(Scheme::icon(":/img/square-green-yellow"));
            else
                return QVariant::fromValue(Scheme::icon(":/img/square-green"));
        }
    case Qt::ToolTipRole:
        QString tooltipText = "";
        switch(mOptionItem.at(index)->error) {
        case OptionErrorType::Invalid_Key:
            tooltipText.append( QString("Unknown parameter '%1'").arg(mOptionItem.at(index)->key) );
            break;
        case OptionErrorType::Incorrect_Value_Type:
            tooltipText.append( QString("Parameter key '%1' has a value of incorrect type").arg(mOptionItem.at(index)->key) );
            break;
        case OptionErrorType::Value_Out_Of_Range:
            tooltipText.append( QString("Value '%1' for parameter key '%2' is out of range").arg(mOptionItem.at(index)->value).arg(mOptionItem.at(index)->key) );
            break;
        case OptionErrorType::Deprecated_Option:
            tooltipText.append( QString("Parameter '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(index)->key) );
            break;
        default:
            break;
        }
        if (mOptionItem.at(index)->recurrent) {
            if (!tooltipText.isEmpty())
                tooltipText.append("\n");
            tooltipText.append( QString("Recurrent parameter '%1', only last entry of same parameters will not be ignored").arg(mOptionItem.at(index)->key));
        }
        return tooltipText;
    }
    return QVariant();
}

int GamsConfigParamTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return  mOptionItem.size();
}

int GamsConfigParamTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant GamsConfigParamTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (mOptionItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        if (col==COLUMN_PARAM_KEY) {
            return mOptionItem.at(row)->key;
        } else if (col== COLUMN_PARAM_VALUE) {
                 return mOptionItem.at(row)->value;
        } else if (col==COLUMN_ENTRY_NUMBER) {
                  return mOptionItem.at(row)->optionId;
        } else { // TODO (JP)
            break;
        }
    }
    case Qt::TextAlignmentRole: {
        return Qt::AlignLeft+ Qt::AlignVCenter;
    }
    case Qt::ToolTipRole: {
        QString tooltipText = "";
        switch(mOptionItem.at(row)->error) {
        case OptionErrorType::Invalid_Key:
            tooltipText.append( QString("Unknown parameter '%1'").arg(mOptionItem.at(row)->key));
            break;
        case OptionErrorType::Incorrect_Value_Type:
            tooltipText.append( QString("Parameter key '%1' has a value of incorrect type").arg(mOptionItem.at(row)->key) );
            break;
        case OptionErrorType::Value_Out_Of_Range:
            tooltipText.append( QString("Value '%1' for parameter key '%2' is out of range").arg(mOptionItem.at(row)->value).arg(mOptionItem.at(row)->key) );
            break;
        case OptionErrorType::Deprecated_Option:
            tooltipText.append( QString("Parameter '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(row)->key) );
            break;
        case OptionErrorType::UserDefined_Error:
            tooltipText.append( QString("Invalid parameter key or value or comment defined") );
            break;
        default:
            break;
        }
        if (mOptionItem.at(row)->recurrent) {
            if (!tooltipText.isEmpty())
                tooltipText.append("\n");
            tooltipText.append( QString("Recurrent parameter '%1', only last entry of same parameters will not be ignored").arg(mOptionItem.at(row)->key));
        }
        return tooltipText;
    }
    case Qt::TextColorRole: {
//        if (Qt::CheckState(headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toBool()))
//            return QVariant::fromValue(QColor(Qt::gray));

        if (mOptionItem[index.row()]->recurrent && index.column()==COLUMN_PARAM_KEY)
            return QVariant::fromValue(QColor(Qt::darkYellow));

        if (mOption->isDoubleDashedOption(mOptionItem.at(row)->key)) { // double dashed parameter
            if (!mOption->isDoubleDashedOptionNameValid( mOption->getOptionKey(mOptionItem.at(row)->key)) )
                return QVariant::fromValue(Scheme::color(Scheme::Normal_Red));
            else
                 return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        }
        if (mOption->isValid(mOptionItem.at(row)->key) || mOption->isASynonym(mOptionItem.at(row)->key)) { // valid option
            if (col==COLUMN_PARAM_KEY) { // key
                if (mOption->isDeprecated(mOptionItem.at(row)->key)) { // deprecated option
                    return QVariant::fromValue(QColor(Qt::gray));
                } else {
                    return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
                }
            } else { // value
                  switch (mOption->getValueErrorType(mOptionItem.at(row)->key, mOptionItem.at(row)->value)) {
                      case OptionErrorType::Incorrect_Value_Type:
                            return QVariant::fromValue(Scheme::color(Scheme::Normal_Red));
                      case OptionErrorType::Value_Out_Of_Range:
                            return QVariant::fromValue(Scheme::color(Scheme::Normal_Red));
                      case OptionErrorType::No_Error:
                            return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
                      default:
                           return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
                  }
            }
        } else { // invalid option
            if (col ==COLUMN_PARAM_KEY)
               return QVariant::fromValue(Scheme::color(Scheme::Normal_Red));
            else
                return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        }

     }
    default:
        break;
    }
    return QVariant();

}

Qt::ItemFlags GamsConfigParamTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
     if (!index.isValid())
         return Qt::NoItemFlags | Qt::ItemIsDropEnabled ;
     else
         return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
}

bool GamsConfigParamTableModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    emit headerDataChanged(orientation, index, index);

    return true;
}

bool GamsConfigParamTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole)   {
        QString dataValue = value.toString().simplified();
        if (dataValue.isEmpty())
            return false;

        if (index.row() > mOptionItem.size())
            return false;

        if (index.column() == COLUMN_PARAM_KEY) { // key
            QString from = data(index, Qt::DisplayRole).toString();
            mOptionItem[index.row()]->key = dataValue;
            if (QString::compare(from, dataValue, Qt::CaseInsensitive)!=0)
                emit optionNameChanged(from, dataValue);
        } else if (index.column() == COLUMN_PARAM_VALUE) { // value
                  mOptionItem[index.row()]->value = dataValue;
                  emit optionValueChanged(index);
        } else if (index.column() == COLUMN_ENTRY_NUMBER) {
                  mOptionItem[index.row()]->optionId = dataValue.toInt();
        }
        emit optionModelChanged(  mOptionItem );
    } else if (role == Qt::CheckStateRole) {
        if (index.row() > mOptionItem.size())
            return false;

        mOptionItem[index.row()]->disabled = value.toBool();
        emit optionModelChanged(  mOptionItem );
    }
    emit dataChanged(index, index);
    return true;
}

QModelIndex GamsConfigParamTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

bool GamsConfigParamTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if (count < 1 || row < 0 || row > mOptionItem.size())
         return false;

     beginInsertRows(QModelIndex(), row, row + count - 1);
     if (mOptionItem.size() == row)
         mOptionItem.append(new ParamConfigItem());
     else
         mOptionItem.insert(row, new ParamConfigItem());

    endInsertRows();
    emit optionModelChanged(mOptionItem);
    return true;
}

bool GamsConfigParamTableModel::removeRows(int row, int count, const QModelIndex &parent)
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

bool GamsConfigParamTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
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

QStringList GamsConfigParamTableModel::mimeTypes() const
{
    QStringList types;
    types << optionMimeType(OptionDefinitionType::ConfigOptionDefinition);
    return types;
}

QMimeData *GamsConfigParamTableModel::mimeData(const QModelIndexList &indexes) const
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

    mimeData->setData(optionMimeType(OptionDefinitionType::ConfigOptionDefinition), encodedData);
    return mimeData;
}

Qt::DropActions GamsConfigParamTableModel::supportedDragActions() const
{
    return Qt::MoveAction ;
}

Qt::DropActions GamsConfigParamTableModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction ;
}

bool GamsConfigParamTableModel::dropMimeData(const QMimeData *mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column)
    if (action == Qt::IgnoreAction)
        return true;

    if (!mimedata->hasFormat(optionMimeType(OptionDefinitionType::ConfigOptionDefinition)))
        return false;

    QByteArray encodedData = mimedata->data(optionMimeType(OptionDefinitionType::ConfigOptionDefinition));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;
    int rows = 0;

    while (!stream.atEnd()) {
       QString text;
       stream >> text;
       newItems << text;
       ++rows;
    }

    int beginRow = -1;

    if (row != -1) {
        beginRow = row;
    } else if (parent.isValid()) {
        beginRow = parent.row();
    } else {
        beginRow = rowCount(QModelIndex());
    }

//    StudioSettings* settings = SettingsLocator::settings();
    if (action ==  Qt::CopyAction) {

        QList<ParamConfigItem *> itemList;
        QList<int> overrideIdRowList;
        for (const QString &text : newItems) {
            QStringList textList = text.split("=");
            int optionid = mOption->getOptionDefinition(textList.at(0)).number;
            itemList.append(new ParamConfigItem(optionid, textList.at( COLUMN_PARAM_KEY ), textList.at( COLUMN_PARAM_VALUE )));
            QModelIndexList indices = match(index(COLUMN_PARAM_KEY,COLUMN_ENTRY_NUMBER), Qt::DisplayRole,
                                            QVariant(optionid), Qt::MatchRecursive);
//          if (settings && settings->overridExistingOption()) {
              for(QModelIndex idx : indices) { overrideIdRowList.append(idx.row()); }
//          }
         }
         std::sort(overrideIdRowList.begin(), overrideIdRowList.end());

         bool replaceExistingEntry = false;
         bool singleEntryExisted = (overrideIdRowList.size()==1);
         bool multipleEntryExisted = (overrideIdRowList.size()>1);
         if (singleEntryExisted) {
             QMessageBox msgBox;
             msgBox.setWindowTitle("Parameter Entry exists");
             msgBox.setText("Parameter '" + data(index(overrideIdRowList.at(0), COLUMN_PARAM_KEY)).toString()+ "' already exists.");
             msgBox.setInformativeText("How do you want to proceed?");
             msgBox.setDetailedText(QString("Entry:  '%1'\nDescription:  %2 %3").arg(getParameterTableEntry(overrideIdRowList.at(0)))
                     .arg("When running GAMS with multiple entries of the same parameter, only the value of the last entry will be utilized by GAMS.")
                     .arg("The value of all other entries except the last entry will be ignored."));
             msgBox.setStandardButtons(QMessageBox::Abort);
             msgBox.addButton("Replace existing entry", QMessageBox::ActionRole);
             msgBox.addButton("Add new entry", QMessageBox::ActionRole);

             switch(msgBox.exec()) {
             case 0: // replace
                replaceExistingEntry = true;
                beginRow = overrideIdRowList.at(0);
                break;
             case 1: // add
                break;
             case QMessageBox::Abort:
                itemList.clear();
                return false;
             }
         } else if (multipleEntryExisted) {
             QMessageBox msgBox;
             msgBox.setWindowTitle("Multiple Parameter Entries exist");
             msgBox.setText(QString("%1 entries of Parmaeter '%2' already exist.").arg(overrideIdRowList.size())
                      .arg(data(index(overrideIdRowList.at(0), COLUMN_PARAM_KEY)).toString()));
             msgBox.setInformativeText("How do you want to proceed?");
             QString entryDetailedText = QString("Entries:\n");
             int i = 0;
             for (int id : overrideIdRowList)
                 entryDetailedText.append(QString("   %1. '%2'\n").arg(++i).arg(getParameterTableEntry(id)));
             msgBox.setDetailedText(QString("%1Description:  %2 %3").arg(entryDetailedText)
                      .arg("When running GAMS with multiple entries of the same parameter, only the value of the last entry will be utilized by the GAMS.")
                      .arg("The value of all other entries except the last entry will be ignored."));
             msgBox.setText("Multiple entries of Parameter '" + data(index(overrideIdRowList.at(0), COLUMN_PARAM_KEY)).toString() + "' already exist.");
             msgBox.setInformativeText("How do you want to proceed?");
             msgBox.setStandardButtons(QMessageBox::Abort);
             msgBox.addButton("Replace first entry and delete other entries", QMessageBox::ActionRole);
             msgBox.addButton("Add new entry", QMessageBox::ActionRole);

             switch(msgBox.exec()) {
             case 0: { // delete and replace
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
             case 1: { // add
                 break;
             }
             case QMessageBox::Abort: {
                 itemList.clear();
                 return false;
             }
             }
         } // else entry not exist

         for (const ParamConfigItem* item : itemList) {
             if (!replaceExistingEntry)
                 insertRows(beginRow, 1, QModelIndex());

             QModelIndex idx = index(beginRow, COLUMN_PARAM_KEY);
             setData(idx, item->key, Qt::EditRole);
             setData( index(beginRow, COLUMN_PARAM_VALUE), item->value, Qt::EditRole);
             setData( index(beginRow, COLUMN_ENTRY_NUMBER), item->optionId, Qt::EditRole);
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

QString GamsConfigParamTableModel::getParameterTableEntry(int row)
{
    QModelIndex keyIndex = index(row, COLUMN_PARAM_KEY);
    QVariant optionKey = data(keyIndex, Qt::DisplayRole);
    QModelIndex valueIndex = index(row, COLUMN_PARAM_VALUE);
    QVariant optionValue = data(valueIndex, Qt::DisplayRole);
    return QString("%1%2%3").arg(optionKey.toString()).arg(mOptionTokenizer->getOption()->getDefaultSeparator()).arg(optionValue.toString());

}


} // namepsace option
} // namespace studio
} // namespace gams

