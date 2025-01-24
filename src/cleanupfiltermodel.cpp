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
#include "cleanupfiltermodel.h"

namespace gams {
namespace studio {

CleanupFilterItem::CleanupFilterItem(Qt::CheckState checked,
                                     const QString &filter,
                                     const QString &description,
                                     CleanupFilterItem *parent)
    : mParent(parent)
    , mFilter(filter)
    , mDescription(description)
    , mChecked(checked)
{

}

CleanupFilterItem::~CleanupFilterItem()
{
    qDeleteAll(mItems);
}

QString CleanupFilterItem::filter() const
{
    return mFilter;
}

void CleanupFilterItem::setFilter(const QString &filter)
{
    mFilter = filter;
}

QString CleanupFilterItem::description() const
{
    return mDescription;
}

void CleanupFilterItem::setDescription(const QString &description)
{
    mDescription = description;
}

Qt::CheckState CleanupFilterItem::checked()
{
    return mChecked;
}

void CleanupFilterItem::setChecked(Qt::CheckState state)
{
    mChecked = state;
}

void CleanupFilterItem::append(CleanupFilterItem *child)
{
    mItems.append(child);
}

void CleanupFilterItem::remove(int index)
{
    if (index < mItems.count()) {
        auto item = mItems.takeAt(index);
        delete item;
    }
}

void CleanupFilterItem::removeAllItems()
{
    qDeleteAll(mItems);
    mItems.clear();
}

CleanupFilterItem *CleanupFilterItem::item(int index)
{
    if (index < 0 || index >= mItems.size())
        return nullptr;
    return mItems.at(index);
}

QList<CleanupFilterItem *> CleanupFilterItem::items() const
{
    return mItems;
}

void CleanupFilterItem::setItems(const QList<CleanupFilterItem *> &items)
{
    for (auto item : items) {
        item->setParent(this);
        mItems.append(item);
    }
}

bool CleanupFilterItem::hasItems() const
{
    return !mItems.isEmpty();
}

int CleanupFilterItem::entries() const
{
    return mItems.size();
}

int CleanupFilterItem::row() const
{
    if (mParent)
        return mParent->mItems.indexOf(const_cast<CleanupFilterItem*>(this));
    return 0;
}

CleanupFilterItem *CleanupFilterItem::parent() const
{
    return mParent;
}

void CleanupFilterItem::setParent(CleanupFilterItem *parent)
{
    mParent = parent;
}

CleanupFilterModel::CleanupFilterModel(QObject *parent)
    : QAbstractItemModel(parent)
    , mRootItem(new CleanupFilterItem)
{
    mRootItem->append(new CleanupFilterItem(Qt::Checked, "*.bk", "Remove all autosave backup files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Checked, "*.bak", "Remove all backup files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Checked, "*.gdx", "Remove all GDX files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Checked, "*.lst", "Remove all LST files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Checked, "*.lxi", "Remove all LXI files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Checked, "*.log*", "Remove all LOG files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Checked, "*.ref", "Remove all REF files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Checked, "*.g00", "Remove all GAMS work files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Unchecked, "*.~tmp*", "Remove all temporary text files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Unchecked, "~*.gms", "Remove unsaved crash recovery files.", mRootItem));
    mRootItem->append(new CleanupFilterItem(Qt::Checked, "225*", "Remove all scratch directories.", mRootItem));
}

CleanupFilterModel::~CleanupFilterModel()
{
    delete mRootItem;
}

Qt::ItemFlags CleanupFilterModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
}

int CleanupFilterModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2; // filter and description
}

int CleanupFilterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    CleanupFilterItem *item;
    if (!parent.isValid())
        item = mRootItem;
    else
        item = static_cast<CleanupFilterItem*>(parent.internalPointer());
    return item->entries();
}

CleanupFilterItem *CleanupFilterModel::data() const
{
    return mRootItem;
}

void CleanupFilterModel::setData(const QList<CleanupFilterItem *> &entries)
{
    emit beginResetModel();
    mRootItem->removeAllItems();
    mRootItem->setItems(entries);
    emit endResetModel();
}

QVariant CleanupFilterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    auto item = static_cast<CleanupFilterItem*>(index.internalPointer());
    if (!item)
        return QVariant();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0)
            return item->filter();
        if (index.column() == 1)
            return item->description();
    }
    if (role == Qt::CheckStateRole && index.column() == 0) {
        return item->checked();
    }
    return QVariant();
}

bool CleanupFilterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (role == Qt::CheckStateRole && index.column() == 0) {
        auto item = static_cast<CleanupFilterItem*>(index.internalPointer());
        item->setChecked(value.toBool() ? Qt::Checked : Qt::Unchecked);
        emit dataChanged(index, index);
        return true;
    }
    if (role == Qt::EditRole) {
        auto data = value.toString();
        if (data.isEmpty())
            return false;
        auto item = static_cast<CleanupFilterItem*>(index.internalPointer());
        if (index.column() == 0) {
            item->setFilter(data);
            emit dataChanged(index, index);
            return true;
        }
        if (index.column() == 1) {
            item->setDescription(data);
            emit dataChanged(index, index);
            return true;
        }
    }
    return QAbstractItemModel::setData(index, value, role);
}

QVariant CleanupFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section == 0)
            return "Filter";
        else if (section == 1)
            return "Description";
    }
    return QVariant();
}

QModelIndex CleanupFilterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    CleanupFilterItem *item;
    if (!parent.isValid())
        item = mRootItem;
    else
        item = static_cast<CleanupFilterItem*>(parent.internalPointer());
    auto child = item->item(row);
    if (child)
        return createIndex(row, column, child);
    return QModelIndex();
}

QModelIndex CleanupFilterModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    auto child = static_cast<CleanupFilterItem*>(index.internalPointer());
    auto item = child->parent();
    if (item == mRootItem)
        return QModelIndex();
    return createIndex(item->row(), 0, item);
}

void CleanupFilterModel::setSelection(Qt::CheckState checkState)
{
    emit beginResetModel();
    for (int i=0; i<mRootItem->entries(); ++i) {
        mRootItem->item(i)->setChecked(checkState);
    }
    emit endResetModel();
    emit dataChanged(QModelIndex(), QModelIndex());
}

bool CleanupFilterModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    emit beginInsertRows(QModelIndex(), position, position+rows-1);
    mRootItem->append(new CleanupFilterItem(Qt::Unchecked, "t.b.d.", QString(), mRootItem));
    emit endInsertRows();
    return true;
}

bool CleanupFilterModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    emit beginRemoveRows(QModelIndex(), position, position+rows-1);
    mRootItem->remove(position);
    emit endRemoveRows();
    return true;
}

QStringList CleanupFilterModel::activeFilters() const
{
    QStringList filters;
    for (auto item : mRootItem->items()) {
        if (item->checked() == Qt::Checked)
            filters << item->filter();
    }
    return filters;
}

CleanupWorkspaceModel::CleanupWorkspaceModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

Qt::ItemFlags CleanupWorkspaceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

int CleanupWorkspaceModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1; // Workspace path
}

int CleanupWorkspaceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mWorkspaces.count();
}

QVariant CleanupWorkspaceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mWorkspaces.count())
        return QVariant();
    if (role == Qt::DisplayRole) {
        return mWorkspaces.at(index.row()).Workspace;
    }
    if (role == Qt::CheckStateRole) {
        return mWorkspaces.at(index.row()).CheckState;
    }
    return QVariant();
}

QVariant CleanupWorkspaceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    if (role != Qt::DisplayRole)
        return QVariant();
    return orientation == Qt::Horizontal ? "Workspace" : QVariant();
}

bool CleanupWorkspaceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= mWorkspaces.count())
        return false;
    if (role == Qt::CheckStateRole && index.column() == 0) {
        mWorkspaces[index.row()].CheckState = (value.toBool() ? Qt::Checked : Qt::Unchecked);
        emit dataChanged(index, index);
        return true;
    }
    return QAbstractTableModel::setData(index, value, role);
}

const QList<CleanupWorkspaceItem> &CleanupWorkspaceModel::workspaces() const
{
    return mWorkspaces;
}

void CleanupWorkspaceModel::setWorkspaces(const QList<CleanupWorkspaceItem> &workspaces)
{
    emit beginResetModel();
    mWorkspaces = workspaces;
    emit endResetModel();
}

void CleanupWorkspaceModel::setSelection(Qt::CheckState checkState)
{
    emit beginResetModel();
    for (int i=0; i<mWorkspaces.count(); ++i) {
        mWorkspaces[i].CheckState = checkState;
    }
    emit endResetModel();
    emit dataChanged(QModelIndex(), QModelIndex());
}

QStringList CleanupWorkspaceModel::activeWorkspaces() const
{
    QStringList workspaces;
    for (auto ws : mWorkspaces) {
        if (ws.CheckState == Qt::Checked)
            workspaces << ws.Workspace;
    }
    return workspaces;
}

}
}
