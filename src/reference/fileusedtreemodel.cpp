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
#include "fileusedtreemodel.h"
#include "filereferenceitem.h"

namespace gams {
namespace studio {
namespace reference {

FileUsedTreeModel::FileUsedTreeModel(QObject *parent) :
    QAbstractItemModel(parent), mReference(nullptr), mRootItem(nullptr)
{
    setupTreeItemModelData();
}

FileUsedTreeModel::~FileUsedTreeModel()
{
}

QVariant FileUsedTreeModel::data(const QModelIndex &index, int role) const
{
    if (!mReference)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::TextAlignmentRole: {
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    case Qt::DisplayRole: {
        FileReferenceItem* item = static_cast<FileReferenceItem*>(index.internalPointer());

        return item->data(index.column());
    }
    default:
         break;
    }
    return QVariant();
}

Qt::ItemFlags FileUsedTreeModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}

QVariant FileUsedTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return header().at(section);
    }

    return QVariant();
}

QModelIndex FileUsedTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FileReferenceItem *parentItem = getItem(parent);
    FileReferenceItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FileUsedTreeModel::indexForTreeItem(FileReferenceItem *item)
{
    return createIndex(item->childNumber(), 0, item);
}

QModelIndex FileUsedTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    FileReferenceItem *childItem = static_cast<FileReferenceItem*>(index.internalPointer());
    FileReferenceItem *parentItem = childItem->parent();

    if (parentItem == mRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FileUsedTreeModel::rowCount(const QModelIndex &parent) const
{
    FileReferenceItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<FileReferenceItem*>(parent.internalPointer());

    return parentItem ? parentItem->childCount() : 0;
}

int FileUsedTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<FileReferenceItem*>(parent.internalPointer())->columnCount();
    else
        return header().size();
}

FileReferenceItem *FileUsedTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        FileReferenceItem* item = static_cast<FileReferenceItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return mRootItem;
}

void FileUsedTreeModel::resetModel()
{
    beginResetModel();
    if (rowCount() > 0) {
        removeRows(0, rowCount(), QModelIndex());
    }
    endResetModel();
}

void FileUsedTreeModel::initModel(Reference *ref)
{
    mReference = ref;
    setupTreeItemModelData();
    resetModel();
}

bool FileUsedTreeModel::isModelLoaded()
{
    return (mReference!=nullptr);
}

void FileUsedTreeModel::setupTreeItemModelData()
{
    if (!isModelLoaded())
        return;

    if (mReference->getNumberOfFileUsed() > 0) {
        mRootItem = mReference->getFileUsedReference(0);
    }

}

} // namespace reference
} // namespace studio
} // namespace gams
