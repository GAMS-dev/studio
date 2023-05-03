/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include "configparamtablemodel.h"
#include "theme.h"

#include <QApplication>
#include <QMessageBox>
#include <QMimeData>

namespace gams {
namespace studio {
namespace option {

ConfigParamTableModel::ConfigParamTableModel(QList<ParamConfigItem *> itemList, OptionTokenizer *tokenizer, QObject *parent):
    QAbstractTableModel(parent), mOptionItem(itemList), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption())
{
    mHeader << "Key"  << "Value" << "minVersion" << "maxVersion"  << "Debug Entry";

    for(ParamConfigItem* item : itemList) {
        QList<OptionErrorType> errorType = mOptionTokenizer->validate(item);
        item->error =  (errorType.isEmpty() ? OptionErrorType::No_Error : errorType.at(0));
    }
    updateCheckState();
}

QVariant ConfigParamTableModel::headerData(int index, Qt::Orientation orientation, int role) const
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
    case Qt::ToolTipRole:
        QString tooltipText = "";
        if (mOptionItem.at(index)->value.isEmpty()) {
            tooltipText.append(QString("Missing value for parameter key '%1'").arg(mOptionItem.at(index)->key));
        } else {
            switch(mOptionItem.at(index)->error) {
            case OptionErrorType::Invalid_Key:
                tooltipText.append( QString("Unknown parameter '%1'").arg(mOptionItem.at(index)->key) );
                break;
            case OptionErrorType::Invalid_minVersion:
                tooltipText.append( QString("Invalid minVersion '%1', must be  conformed to [x[.y[.z]]] format").arg(mOptionItem.at(index)->minVersion) );
                break;
            case OptionErrorType::Invalid_maxVersion:
                tooltipText.append( QString("Invalid maxVersion '%1', must be  conformed to [x[.y[.z]]] format").arg(mOptionItem.at(index)->minVersion) );
                break;
            case OptionErrorType::Incorrect_Value_Type:
                tooltipText.append( QString("Parameter key '%1' has a value of incorrect type").arg(mOptionItem.at(index)->key) );
                break;
            case OptionErrorType::Value_Out_Of_Range:
                tooltipText.append( QString("Value '%1' for parameter key '%2' is out of range").arg(mOptionItem.at(index)->value, mOptionItem.at(index)->key) );
                break;
            case OptionErrorType::Deprecated_Option:
                tooltipText.append( QString("Parameter '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(index)->key) );
                break;
            default:
                break;
            }
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

int ConfigParamTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return  mOptionItem.size();
}

int ConfigParamTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant ConfigParamTableModel::data(const QModelIndex &index, int role) const
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
                 return  mOptionItem.at(row)->value;
        } else if (col==COLUMN_MIN_VERSION) {
                  return mOptionItem.at(row)->minVersion;
        } else if (col==COLUMN_MAX_VERSION) {
                  return mOptionItem.at(row)->maxVersion;
        } else if (col==COLUMN_ENTRY_NUMBER) {
            return mOptionItem.at(row)->optionId;
        }
        break;
    }
    case Qt::TextAlignmentRole: {
        if (col==COLUMN_MIN_VERSION || col==COLUMN_MAX_VERSION)
            return int(Qt::AlignRight | Qt::AlignVCenter);
        else
           return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
    case Qt::ToolTipRole: {
        QString tooltipText = "";
        if (mOptionItem.at(row)->value.isEmpty()) {
            tooltipText.append( QString("Missing value for parameter key '%1'").arg(mOptionItem.at(row)->key) );
        } else {
            switch(mOptionItem.at(row)->error) {
            case OptionErrorType::Invalid_Key:
                tooltipText.append( QString("Unknown parameter '%1'").arg(mOptionItem.at(row)->key));
                break;
            case OptionErrorType::Incorrect_Value_Type:
                tooltipText.append( QString("Parameter key '%1' has a value of incorrect type").arg(mOptionItem.at(row)->key) );
                break;
            case OptionErrorType::Value_Out_Of_Range:
                tooltipText.append( QString("Value '%1' for parameter key '%2' is out of range").arg(mOptionItem.at(row)->value, mOptionItem.at(row)->key) );
                break;
            case OptionErrorType::Deprecated_Option:
                tooltipText.append( QString("Parameter '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(row)->key) );
                break;
            case OptionErrorType::UserDefined_Error:
                tooltipText.append( QString("Invalid parameter key or value or comment defined") );
                break;
            case OptionErrorType::Invalid_minVersion:
                tooltipText.append( QString("Invalid minVersion '%1', must be conformed to [x[.y[.z]]] format").arg(mOptionItem.at(row)->minVersion) );
                break;
            case OptionErrorType::Invalid_maxVersion:
                tooltipText.append( QString("Invalid maxVersion '%1', must be conformed to [x[.y[.z]]] format").arg(mOptionItem.at(row)->maxVersion) );
                break;
            default:
                break;
            }
        }
        if (mOptionItem.at(row)->recurrent) {
            if (!tooltipText.isEmpty())
                tooltipText.append("\n");
            tooltipText.append( QString("Recurrent parameter '%1', only last entry of same parameters will not be ignored").arg(mOptionItem.at(row)->key));
        }
        return tooltipText;
    }
    case Qt::ForegroundRole: {
//        if (Qt::CheckState(headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toBool()))
//            return QVariant::fromValue(QColor(Qt::gray));

        if (mOptionItem[index.row()]->recurrent && index.column()==COLUMN_PARAM_KEY)
            return QVariant::fromValue(QColor(Qt::darkYellow));

        if (index.column()==COLUMN_MIN_VERSION) {
            if (mOptionItem[index.row()]->minVersion.isEmpty())
                return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
            else if (mOption->isConformantVersion(mOptionItem[index.row()]->minVersion))
                     return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
            else
                 return QVariant::fromValue(Theme::color(Theme::Normal_Red));
        } else if (index.column()==COLUMN_MAX_VERSION) {
            if (mOptionItem[index.row()]->maxVersion.isEmpty())
                return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
            else if (mOption->isConformantVersion(mOptionItem[index.row()]->maxVersion))
                    return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
            else
                return QVariant::fromValue(Theme::color(Theme::Normal_Red));
        }
        if (mOption->isDoubleDashedOption(mOptionItem.at(row)->key)) { // double dashed parameter
            if (!mOption->isDoubleDashedOptionNameValid( mOption->getOptionKey(mOptionItem.at(row)->key)) )
                return QVariant::fromValue(Theme::color(Theme::Normal_Red));
            else
                 return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
        }
        if (mOption->isValid(mOptionItem.at(row)->key) || mOption->isASynonym(mOptionItem.at(row)->key)) { // valid option
            if (col==COLUMN_PARAM_KEY) { // key
                if (mOption->isDeprecated(mOptionItem.at(row)->key)) { // deprecated option
                    return QVariant::fromValue(QColor(Theme::Disable_Gray));
                } else {
                    return  QVariant::fromValue(QApplication::palette().color(QPalette::Text));
                }
            } else if (col==COLUMN_PARAM_VALUE) { // value
                  switch (mOption->getValueErrorType(mOptionItem.at(row)->key, mOptionItem.at(row)->value)) {
                      case OptionErrorType::Incorrect_Value_Type:
                            return QVariant::fromValue(Theme::color(Theme::Normal_Red));
                      case OptionErrorType::Value_Out_Of_Range:
                            return QVariant::fromValue(Theme::color(Theme::Normal_Red));
                      case OptionErrorType::No_Error:
                            return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
                      default:
                           return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
                  }
            } else if (col==COLUMN_MIN_VERSION && mOption->isConformantVersion(mOptionItem.at(row)->minVersion)) {
                      return QVariant::fromValue(Theme::color(Theme::Normal_Red));
            } else if (col==COLUMN_MAX_VERSION && mOption->isConformantVersion(mOptionItem.at(row)->maxVersion)) {
                       return QVariant::fromValue(Theme::color(Theme::Normal_Red));
            }
        } else { // invalid option
            if (col ==COLUMN_PARAM_KEY)
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

Qt::ItemFlags ConfigParamTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
     if (!index.isValid())
         return Qt::NoItemFlags | Qt::ItemIsDropEnabled ;
     else
         return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
}

bool ConfigParamTableModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    emit headerDataChanged(orientation, index, index);

    return true;
}

bool ConfigParamTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() > mOptionItem.size())
        return false;

    QVector<int> roles;
    if (role == Qt::EditRole)   {
        roles = { Qt::EditRole };
        QString dataValue = value.toString().simplified();
        if (index.column() != COLUMN_MIN_VERSION &&
            index.column() != COLUMN_MAX_VERSION &&
            dataValue.isEmpty())
            return false;

        if (index.row() > mOptionItem.size())
            return false;

        if (index.column() == COLUMN_PARAM_KEY) { // key
//            QString from = data(index, Qt::DisplayRole).toString();
            mOptionItem[index.row()]->key = dataValue;
        } else if (index.column() == COLUMN_PARAM_VALUE) { // value
                  mOptionItem[index.row()]->value = dataValue;
        } else if (index.column() == COLUMN_ENTRY_NUMBER) {
                  mOptionItem[index.row()]->optionId = dataValue.toInt();
        } else if (index.column() == COLUMN_MIN_VERSION) {
                   mOptionItem[index.row()]->minVersion = dataValue;
        } else if (index.column() == COLUMN_MAX_VERSION) {
            mOptionItem[index.row()]->maxVersion = dataValue;
        }
        emit dataChanged(index, index, roles);
    } else if (role == Qt::CheckStateRole) {
        roles = { Qt::CheckStateRole };
        mOptionItem[index.row()]->disabled = (Qt::CheckState(value.toUInt())==Qt::PartiallyChecked);
        mCheckState[index.row()] = value;
        mOptionItem[index.row()]->disabled = value.toBool();
        emit dataChanged(index, index, roles);
    }
    return true;
}

QModelIndex ConfigParamTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

bool ConfigParamTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if (count < 1 || row < 0 || row > mOptionItem.size())
         return false;

     beginInsertRows(QModelIndex(), row, row + count - 1);
     if (mOptionItem.size() == row)
         mOptionItem.append(new ParamConfigItem());
     else
         mOptionItem.insert(row, new ParamConfigItem());

     updateCheckState();

    endInsertRows();
    return true;
}

bool ConfigParamTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if (count < 1 || row < 0 || row > mOptionItem.size() || mOptionItem.size() ==0)
         return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i=row+count-1; i>=row; --i) {
        mOptionItem.removeAt(i);
    }
    endRemoveRows();
    emit configParamItemRemoved();
    return true;
}

bool ConfigParamTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
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
               ParamConfigItem* item = mOptionItem.at(sourceRow+i);
               mOptionItem.removeAt(sourceRow+i);
               mOptionItem.insert(destinationChild+i, item);
           }
    }
    updateCheckState();
    endMoveRows();
    return true;
}

QStringList ConfigParamTableModel::mimeTypes() const
{
    QStringList types;
    types << optionMimeType(OptionDefinitionType::ConfigOptionDefinition);
    return types;
}

QMimeData *ConfigParamTableModel::mimeData(const QModelIndexList &indexes) const
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

    mimeData->setData(optionMimeType(OptionDefinitionType::ConfigOptionDefinition), encodedData);
    return mimeData;
}

Qt::DropActions ConfigParamTableModel::supportedDragActions() const
{
    return Qt::MoveAction ;
}

Qt::DropActions ConfigParamTableModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction ;
}

bool ConfigParamTableModel::dropMimeData(const QMimeData *mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column)
    if (action == Qt::IgnoreAction)
        return true;

    if (!mimedata->hasFormat(optionMimeType(OptionDefinitionType::ConfigOptionDefinition)))
        return false;

    QByteArray encodedData = mimedata->data(optionMimeType(OptionDefinitionType::ConfigOptionDefinition));
    QDataStream stream(&encodedData, QDataStream::ReadOnly);
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
        for (const QString &text : qAsConst(newItems)) {
            QStringList textList = text.split("=");
            int optionid = mOption->getOptionDefinition(textList.at(0)).number;
            itemList.append(new ParamConfigItem(optionid, textList.at( COLUMN_PARAM_KEY ), textList.at( COLUMN_PARAM_VALUE )));
            QModelIndexList indices = match(index(COLUMN_PARAM_KEY,COLUMN_ENTRY_NUMBER), Qt::DisplayRole,
                                            QVariant(optionid), Qt::MatchRecursive);
//          if (settings && settings->overridExistingOption()) {
              for(QModelIndex idx : qAsConst(indices)) { overrideIdRowList.append(idx.row()); }
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
             msgBox.setDetailedText(QString("Entry:  '%1'\nDescription:  %2 %3").arg(getParameterTableEntry(overrideIdRowList.at(0)),
                     "When running GAMS with multiple entries of the same parameter, only the value of the last entry will be utilized by GAMS.",
                     "The value of all other entries except the last entry will be ignored."));
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
             msgBox.setDetailedText(QString("%1Description:  %2 %3").arg(entryDetailedText,
                      "When running GAMS with multiple entries of the same parameter, only the value of the last entry will be utilized by the GAMS.",
                      "The value of all other entries except the last entry will be ignored."));
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
             setData( index(beginRow, COLUMN_MIN_VERSION), item->minVersion, Qt::EditRole);
             setData( index(beginRow, COLUMN_MAX_VERSION), item->maxVersion, Qt::EditRole);
             if (item->key.isEmpty() || item->value.isEmpty())
                 setHeaderData( idx.row(), Qt::Vertical, Qt::CheckState(Qt::Checked), Qt::CheckStateRole );
             else
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

const QList<ParamConfigItem *> ConfigParamTableModel::parameterConfigItems()
{
    return mOptionItem;
}

void ConfigParamTableModel::on_groupDefinitionReloaded()
{
    emit configParamModelChanged(mOptionItem);
}

void ConfigParamTableModel::on_reloadConfigParamModel(const QList<ParamConfigItem *> &optionItem)
{
    disconnect(this, &QAbstractTableModel::dataChanged, this, &ConfigParamTableModel::on_updateConfigParamItem);

    beginResetModel();

    qDeleteAll(mOptionItem);
    mOptionItem.clear();

    mOptionItem = optionItem;
    for(ParamConfigItem* item : optionItem) {
        QList<OptionErrorType> errorType = mOptionTokenizer->validate(item);
        item->error =  (errorType.isEmpty() ? OptionErrorType::No_Error : errorType.at(0));
    }
    updateCheckState();

    setRowCount(mOptionItem.size());

    for (int i=0; i<mOptionItem.size(); ++i) {
        if (mOptionItem.at(i)->disabled) {
            setData( index(i, COLUMN_PARAM_KEY), QVariant(mOptionItem.at(i)->key), Qt::EditRole);
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
            setData( index(i, COLUMN_ENTRY_NUMBER), QVariant(mOptionItem.at(i)->optionId), Qt::EditRole);
        } else {
           setData( index(i, COLUMN_PARAM_KEY), QVariant(mOptionItem.at(i)->key), Qt::EditRole);
           setData( index(i, COLUMN_PARAM_VALUE), QVariant(mOptionItem.at(i)->value), Qt::EditRole);
           setData( index(i, COLUMN_MIN_VERSION), mOptionItem.at(i)->minVersion, Qt::EditRole);
           setData( index(i, COLUMN_MAX_VERSION), mOptionItem.at(i)->maxVersion, Qt::EditRole);
           setData( index(i, COLUMN_ENTRY_NUMBER), QVariant(mOptionItem.at(i)->optionId), Qt::EditRole);
           if (mOptionItem.at(i)->error == OptionErrorType::No_Error)
               setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Unchecked),
                              Qt::CheckStateRole );
           else if (mOptionItem.at(i)->error == OptionErrorType::Deprecated_Option)
               setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::PartiallyChecked),
                              Qt::CheckStateRole );
           else
               setHeaderData( i, Qt::Vertical,
                          Qt::CheckState(Qt::Checked),
                          Qt::CheckStateRole );
        }
    }
    emit configParamModelChanged(mOptionItem);
    updateRecurrentStatus();
    endResetModel();
    connect(this, &QAbstractTableModel::dataChanged, this, &ConfigParamTableModel::on_updateConfigParamItem, Qt::UniqueConnection);
}

void ConfigParamTableModel::on_updateConfigParamItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QModelIndex idx = topLeft;
    int row = idx.row();
    while(row <= bottomRight.row()) {
        idx = index(row++, idx.column());
        if (roles.first()==Qt::EditRole) {
              QList<OptionErrorType> errorList = mOptionTokenizer->validate(mOptionItem.at(idx.row()));
              if (errorList.isEmpty()) {
                  mOptionItem[idx.row()]->error = OptionErrorType::No_Error;
              } else {
                  mOptionItem[idx.row()]->error = errorList.at(0);
                  if (mOptionItem[idx.row()]->error == OptionErrorType::Invalid_Key)
                      mOptionItem[idx.row()]->optionId = -1;
              }
              mOptionItem.at(idx.row())->disabled = (mOptionItem[idx.row()]->error == OptionErrorType::Deprecated_Option);
              if (mOptionItem.at(idx.row())->error==OptionErrorType::Deprecated_Option) {
                  setHeaderData( idx.row(), Qt::Vertical,
                                 Qt::CheckState(Qt::PartiallyChecked),
                                 Qt::CheckStateRole );
              } else if (mOptionItem.at(idx.row())->error==OptionErrorType::No_Error) {
                      setHeaderData( idx.row(), Qt::Vertical,
                                     Qt::CheckState(Qt::Unchecked),
                                     Qt::CheckStateRole );
              } else {
                   setHeaderData( idx.row(), Qt::Vertical,
                      Qt::CheckState(Qt::Checked),
                      Qt::CheckStateRole );
              }
              emit configParamModelChanged(mOptionItem);
       } else if (roles.first()==Qt::CheckStateRole) {
                  emit configParamModelChanged(mOptionItem);
       }
    }
    updateRecurrentStatus();
}

void ConfigParamTableModel::on_removeConfigParamItem()
{
    beginResetModel();
    mOptionTokenizer->validateOption(mOptionItem);

    setRowCount(mOptionItem.size());

    for (int i=0; i<mOptionItem.size(); ++i) {
        if (mOptionItem.at(i)->disabled || mOptionItem.at(i)->error ==OptionErrorType::Deprecated_Option) {
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
    emit configParamModelChanged(mOptionItem);
    updateRecurrentStatus();
    endResetModel();
}

void ConfigParamTableModel::updateRecurrentStatus()
{
    QList<int> idList;
    for(ParamConfigItem* item : qAsConst(mOptionItem)) {
        idList << item->optionId;
    }
    for(ParamConfigItem* item : qAsConst(mOptionItem)) {
        item->recurrent = (!item->disabled && item->optionId != -1 && idList.count(item->optionId) > 1);
    }
    emit headerDataChanged(Qt::Vertical, 0, mOptionItem.size());
}

void ConfigParamTableModel::updateCheckState()
{
    for(int i = 0; i<mOptionItem.size(); ++i) {
        QVariant value =  QVariant(Qt::Unchecked);
        if (mOptionItem.at(i)->disabled || mOptionItem.at(i)->error == OptionErrorType::Deprecated_Option)
            value = QVariant(Qt::PartiallyChecked);
        else if (mOptionItem.at(i)->error == OptionErrorType::No_Error)
                value = QVariant(Qt::Unchecked);
        else
            value = QVariant(Qt::Checked);

        mCheckState[i] = value;
    }
}

void ConfigParamTableModel::setRowCount(int rows)
{
    int rc = mOptionItem.size();
    if (rows < 0 ||  rc == rows)
       return;

    if (rc < rows)
       insertRows(qMax(rc, 0), rows - rc);
    else
        removeRows(qMax(rows, 0), rc - rows);
}

QString ConfigParamTableModel::getParameterTableEntry(int row)
{
    QModelIndex keyIndex = index(row, COLUMN_PARAM_KEY);
    QVariant optionKey = data(keyIndex, Qt::DisplayRole);
    QModelIndex valueIndex = index(row, COLUMN_PARAM_VALUE);
    QVariant optionValue = data(valueIndex, Qt::DisplayRole);
    return QString("%1%2%3").arg(optionKey.toString(), mOptionTokenizer->getOption()->getDefaultSeparator(), optionValue.toString());

}


} // namepsace option
} // namespace studio
} // namespace gams

