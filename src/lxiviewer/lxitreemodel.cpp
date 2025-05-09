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
#include "lxitreemodel.h"
#include "lxitreeitem.h"

namespace gams {
namespace studio {
namespace lxiviewer {

LxiTreeModel::LxiTreeModel(LxiTreeItem *root,
                           const QVector<int> &lineNrs,
                           const QVector<LxiTreeItem*> &treeItems,
                           QObject *parent)
    : QAbstractItemModel(parent)
    , mRootItem(root)
    , mLineNrs(lineNrs)
    , mTreeItems(treeItems)
{

}

LxiTreeModel::~LxiTreeModel()
{
    delete mRootItem;
}

QModelIndex LxiTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    LxiTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<LxiTreeItem*>(parent.internalPointer());

    LxiTreeItem *childItem = parentItem->child(row);
    if (childItem) {
        QModelIndex modelIndex = createIndex(row, column, childItem);
        childItem->setModelIndex(modelIndex);
        return modelIndex;
    }
    else
        return QModelIndex();
}

QModelIndex LxiTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    LxiTreeItem *childItem = static_cast<LxiTreeItem*>(index.internalPointer());
    LxiTreeItem *parentItem = childItem->parentItem();

    if (parentItem == mRootItem)
        return QModelIndex();

    QModelIndex modelIndex = createIndex(parentItem->row(), 0, parentItem);
    parentItem->setModelIndex(modelIndex);
    return modelIndex;
}

int LxiTreeModel::rowCount(const QModelIndex &parent) const
{
    LxiTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<LxiTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int LxiTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant LxiTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    LxiTreeItem *item = static_cast<LxiTreeItem*>(index.internalPointer());

    return item->text();
}

QVector<int> LxiTreeModel::lineNrs() const
{
    return mLineNrs;
}

QVector<LxiTreeItem *> LxiTreeModel::treeItems() const
{
    return mTreeItems;
}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
