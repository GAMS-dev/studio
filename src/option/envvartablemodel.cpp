/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "envvartablemodel.h"
#include "theme.h"

#include <QApplication>

namespace gams {
namespace studio {
namespace option {

QRegularExpression EnvVarTableModel::mRexVersion("^[1-9][0-9](\\.([0-9])(\\.([0-9]))?)?$");


EnvVarTableModel::EnvVarTableModel(const QList<EnvVarConfigItem *> &itemList, QObject *parent):
    QAbstractTableModel(parent), mEnvVarItem(itemList)
{
    mHeader << "Name"  << "Value" << "minVersion" << "maxVersion" << "pathVariable";
    updateCheckState();
}

QVariant EnvVarTableModel::headerData(int index, Qt::Orientation orientation, int role) const
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
        if (mEnvVarItem.isEmpty())
            return QVariant();
        else
            return mCheckState[index];
    case Qt::DecorationRole:
        if (Qt::CheckState(mCheckState[index].toUInt())==Qt::Checked) {
            return QVariant::fromValue(Theme::icon(":/img/square-red"));
        } else if (Qt::CheckState(mCheckState[index].toUInt())==Qt::PartiallyChecked) {
                  return QVariant::fromValue(Theme::icon(":/img/square-gray"));
        } else {
                return QVariant::fromValue(Theme::icon(":/img/square-green"));
        }
    case Qt::ToolTipRole:
        QString tooltipText = "";
        if (mEnvVarItem.at(index)->key.simplified().isEmpty())
           tooltipText.append( QString("Empty environment variable Name.") );
        if (mEnvVarItem.at(index)->value.simplified().isEmpty())
           tooltipText.append( QString("%1Empty environment variable Value.").arg( tooltipText.isEmpty() ? "" : "\n") );
        if (!mEnvVarItem.at(index)->minVersion.simplified().isEmpty() && !isConformatVersion(mEnvVarItem.at(index)->minVersion))
           tooltipText.append( QString("%1minVersion '%2' must be a version number that is conformed to [xx[.y[.z]]] format.")
                               .arg(tooltipText.isEmpty() ? "" : "\n", mEnvVarItem.at(index)->minVersion));
        if (!mEnvVarItem.at(index)->maxVersion.simplified().isEmpty() && !isConformatVersion(mEnvVarItem.at(index)->maxVersion))
           tooltipText.append( QString("%1maxVersion '%2' must be a version number that is conformed to [xx[.y[.z]]] format.")
                               .arg(tooltipText.isEmpty() ? "" : "\n", mEnvVarItem.at(index)->maxVersion));
        return tooltipText;
    }
    return QVariant();

}

int EnvVarTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return  mEnvVarItem.size();
}

int EnvVarTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant EnvVarTableModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const int col = index.column();

    if (mEnvVarItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        if (col==COLUMN_PARAM_KEY) {
            return mEnvVarItem.at(row)->key;
        } else if (col== COLUMN_PARAM_VALUE) {
                 return mEnvVarItem.at(row)->value;
        } else if (col==COLUMN_MIN_VERSION) {
                  return mEnvVarItem.at(row)->minVersion;
        } else if (col==COLUMN_MAX_VERSION) {
                  return mEnvVarItem.at(row)->maxVersion;
        } else if (col==COLUMN_PATH_VAR) {
            if (mEnvVarItem.at(row)->pathVariable == EnvVarConfigItem::pathDefinition::NONE)
                return " ";
            else if (mEnvVarItem.at(row)->pathVariable == EnvVarConfigItem::pathDefinition::PATH_DEFINED)
                    return "True";
            else if (mEnvVarItem.at(row)->pathVariable == EnvVarConfigItem::pathDefinition::NO_PATH_DEFINED)
                return "False";
            else
                return QString::number(mEnvVarItem.at(row)->pathVariable);
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
        if (mEnvVarItem.at(row)->value.isEmpty()) {
            tooltipText.append( QString("Missing value for '%1'").arg(mEnvVarItem.at(row)->key) );
        } else if (!mEnvVarItem.at(row)->minVersion.simplified().isEmpty() &&
            !isConformatVersion(mEnvVarItem.at(row)->minVersion)) {
            tooltipText = QString("Invalid minVersion '%1', must be conformed to [xx[.y[.z]]] format").arg(mEnvVarItem.at(row)->minVersion);
        } else if (!mEnvVarItem.at(row)->maxVersion.simplified().isEmpty() &&
                   !isConformatVersion(mEnvVarItem.at(row)->maxVersion)) {
                   tooltipText = QString("Invalid maxVersion '%1', must be conformed to [xx[.y[.z]]] format").arg(mEnvVarItem.at(row)->maxVersion);
        }
        return tooltipText;
    }
    case Qt::ForegroundRole: {
         return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
     }
    default:
        break;
    }
    return QVariant();

}

Qt::ItemFlags EnvVarTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
     if (!index.isValid())
         return Qt::NoItemFlags ;
     else
         return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
}

bool EnvVarTableModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    emit headerDataChanged(orientation, index, index);

    return true;
}

bool EnvVarTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() > mEnvVarItem.size())
        return false;

    QVector<int> roles;
    if (role == Qt::EditRole)   {
        roles = { Qt::EditRole };
        if (index.row() > mEnvVarItem.size())
            return false;
        QString dataValue = value.toString();
        if (index.column() != COLUMN_MIN_VERSION &&
            index.column() != COLUMN_MAX_VERSION &&
            dataValue.simplified().isEmpty())
            return false;
        if (index.column() == COLUMN_PARAM_KEY) { // key
//            QString from = data(index, Qt::DisplayRole).toString();
            mEnvVarItem[index.row()]->key = dataValue;
        } else if (index.column() == COLUMN_PARAM_VALUE) { // value
                  mEnvVarItem[index.row()]->value = dataValue;
        } else if (index.column() == COLUMN_PATH_VAR) {
                  if (QString::compare(dataValue, "True", Qt::CaseInsensitive) == 0)
                          mEnvVarItem[index.row()]->pathVariable = EnvVarConfigItem::pathDefinition::PATH_DEFINED;
                  else if (QString::compare(dataValue, "False", Qt::CaseInsensitive) == 0)
                          mEnvVarItem[index.row()]->pathVariable = EnvVarConfigItem::pathDefinition::NO_PATH_DEFINED;
                  else
                      mEnvVarItem[index.row()]->pathVariable = EnvVarConfigItem::pathDefinition::NONE;
        } else if (index.column() == COLUMN_MIN_VERSION) {
                   mEnvVarItem[index.row()]->minVersion = dataValue;
        } else if (index.column() == COLUMN_MAX_VERSION) {
            mEnvVarItem[index.row()]->maxVersion = dataValue;
        }
        emit dataChanged(index, index, roles);
    } else if (role == Qt::CheckStateRole) {
        roles = { Qt::CheckStateRole };
        mCheckState[index.row()] = value;
        emit dataChanged(index, index, roles);
    }
    return true;

}

QModelIndex EnvVarTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

bool EnvVarTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if (count < 1 || row < 0 || row > mEnvVarItem.size())
         return false;

     beginInsertRows(QModelIndex(), row, row + count - 1);
     if (mEnvVarItem.size() == row)
         mEnvVarItem.append(new EnvVarConfigItem());
     else
         mEnvVarItem.insert(row, new EnvVarConfigItem());

     updateCheckState();

     endInsertRows();
     return true;
}

bool EnvVarTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if (count < 1 || row < 0 || row > mEnvVarItem.size() || mEnvVarItem.isEmpty())
         return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i=row+count-1; i>=row; --i) {
        mEnvVarItem.removeAt(i);
    }
    endRemoveRows();
    emit envVarItemRemoved();
    return true;
}

bool EnvVarTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (mEnvVarItem.size() == 0 || count < 1 || destinationChild < 0 ||  destinationChild > mEnvVarItem.size())
         return false;

    Q_UNUSED(sourceParent)
    Q_UNUSED(destinationParent)
    beginMoveRows(QModelIndex(), sourceRow, sourceRow  + count -1 , QModelIndex(), destinationChild);
//    mEnvVarItem.insert(destinationChild, mEnvVarItem.at(sourceRow));
//    int removeIndex = destinationChild > sourceRow ? sourceRow : sourceRow+1;
//    mEnvVarItem.removeAt(removeIndex);
    if (destinationChild > sourceRow) { // move down
       for(int i=0; i<count; ++i) {
           mEnvVarItem.insert(destinationChild, mEnvVarItem.at(sourceRow));
           mEnvVarItem.removeAt(sourceRow);
       }
    } else { // move up
           for(int i=0; i<count; ++i) {
               EnvVarConfigItem* item = mEnvVarItem.at(sourceRow+i);
               mEnvVarItem.removeAt(sourceRow+i);
               mEnvVarItem.insert(destinationChild+i, item);
           }
    }
    updateCheckState();
    endMoveRows();
    return true;
}

QList<EnvVarConfigItem *> EnvVarTableModel::envVarConfigItems()
{
    return mEnvVarItem;
}

void EnvVarTableModel::on_updateEnvVarItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QModelIndex idx = topLeft;
    int row = idx.row();
    while(row <= bottomRight.row()) {
        idx = index(row++, idx.column());
        if (roles.first()==Qt::EditRole) {
              if (isThereAnError( mEnvVarItem.at(idx.row())) ) {
                   setHeaderData( idx.row(), Qt::Vertical,
                          Qt::CheckState(Qt::Checked),
                          Qt::CheckStateRole );
               } else {
                   setHeaderData( idx.row(), Qt::Vertical,
                      Qt::CheckState(Qt::Unchecked),
                      Qt::CheckStateRole );
              }
       }
    }
}


void EnvVarTableModel::on_removeEnvVarItem()
{
    beginResetModel();
    setRowCount(mEnvVarItem.size());

    for(int i = 0; i<mEnvVarItem.size(); ++i) {
        if (isThereAnError(mEnvVarItem.at(i))) {
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Checked),
                              Qt::CheckStateRole );
         } else {
                setHeaderData( i, Qt::Vertical,
                          Qt::CheckState(Qt::Unchecked),
                          Qt::CheckStateRole );
         }
    }
    endResetModel();
}

void EnvVarTableModel::on_reloadEnvVarModel(const QList<EnvVarConfigItem *> &configItem)
{
    beginResetModel();

    qDeleteAll(mEnvVarItem);
    mEnvVarItem.clear();

    mEnvVarItem = configItem;
    updateCheckState();

    setRowCount(mEnvVarItem.size());

    for (int i=0; i<mEnvVarItem.size(); ++i) {
        setData( index(i, COLUMN_PARAM_KEY), QVariant(mEnvVarItem.at(i)->key), Qt::EditRole);
        setData( index(i, COLUMN_PARAM_VALUE), QVariant(mEnvVarItem.at(i)->value), Qt::EditRole);
        setData( index(i, COLUMN_MIN_VERSION), QVariant(mEnvVarItem.at(i)->minVersion), Qt::EditRole);
        setData( index(i, COLUMN_MAX_VERSION), QVariant(mEnvVarItem.at(i)->maxVersion), Qt::EditRole);
        setData( index(i, COLUMN_PATH_VAR), QVariant(mEnvVarItem.at(i)->PATH_DEFINED), Qt::EditRole);
        if (isThereAnError( mEnvVarItem.at(i)) )
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Checked),
                              Qt::CheckStateRole );
        else
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Unchecked),
                              Qt::CheckStateRole );
    }
    endResetModel();
}

void EnvVarTableModel::setRowCount(int rows)
{
    const int rc = mEnvVarItem.size();
    if (rows < 0 ||  rc == rows)
       return;

    if (rc < rows)
       insertRows(qMax(rc, 0), rows - rc);
    else
        removeRows(qMax(rows, 0), rc - rows);
}

void EnvVarTableModel::updateCheckState()
{
    for(int i = 0; i<mEnvVarItem.size(); ++i) {
         if (isThereAnError( mEnvVarItem.at(i)) ) {
             EnvVarTableModel::setHeaderData(i, Qt::Vertical,
                                                Qt::CheckState(Qt::Checked),
                                                Qt::CheckStateRole);
         } else {
             EnvVarTableModel::setHeaderData(i, Qt::Vertical,
                                                Qt::CheckState(Qt::Unchecked),
                                                Qt::CheckStateRole);
         }
    }
}

bool EnvVarTableModel::isThereAnError(EnvVarConfigItem* item) const
{
    if (item->key.simplified().isEmpty())
        return true;
    else if (item->value.simplified().isEmpty())
        return true;
    else if (!item->maxVersion.simplified().isEmpty() && !isConformatVersion(item->maxVersion))
        return true;
    else  if (!item->minVersion.simplified().isEmpty() && !isConformatVersion(item->minVersion))
        return true;
    return false;
}

bool EnvVarTableModel::isConformatVersion(const QString &version) const
{
    return mRexVersion.match(version).hasMatch();
}

} // namepsace option
} // namespace studio
} // namespace gams
