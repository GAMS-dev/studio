/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include <QDebug>
#include "referencetreemodel.h"
#include "referenceitemmodel.h"

namespace gams {
namespace studio {

ReferenceTreeModel::ReferenceTreeModel(Reference* ref, QObject *parent) :
    QAbstractItemModel(parent), mReference(ref), mCurrentSymbolID(-1)
{
    QList<QVariant> rootData;
    rootData << "Location" << "Line" << "Column";
    mRootItem = new ReferenceItemModel(rootData);

    connect(this, &ReferenceTreeModel::symbolSelectionChanged, this, &ReferenceTreeModel::updateSelectedSymbol);
}

ReferenceTreeModel::~ReferenceTreeModel()
{
    delete mRootItem;
}

QVariant ReferenceTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ReferenceItemModel* item = static_cast<ReferenceItemModel*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags ReferenceTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index);
}

QVariant ReferenceTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return mRootItem->data(section);

    return QVariant();
}

QModelIndex ReferenceTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ReferenceItemModel* parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<ReferenceItemModel*>(parent.internalPointer());

    ReferenceItemModel *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ReferenceTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ReferenceItemModel* childItem = static_cast<ReferenceItemModel*>(index.internalPointer());
    ReferenceItemModel* parentItem = childItem->parentItem();

    if (parentItem == mRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ReferenceTreeModel::rowCount(const QModelIndex &parent) const
{
    ReferenceItemModel* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<ReferenceItemModel*>(parent.internalPointer());

    return parentItem->childCount();
}

int ReferenceTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ReferenceItemModel*>(parent.internalPointer())->columnCount();
    else
        return mRootItem->columnCount();
}

bool ReferenceTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!mRootItem) return false;

    bool success = false;
    if (count > 0) {
        beginRemoveRows(parent, row, row + count - 1);
        success = mRootItem->removeChildren(row, count);
        endRemoveRows();
    }
    return success;
}

void ReferenceTreeModel::updateSelectedSymbol(SymbolId symbolid)
{
    mCurrentSymbolID = symbolid;

    beginResetModel();

    if (rowCount() > 0) {
        removeRows(0, rowCount(), QModelIndex());
    }

    QList<ReferenceItemModel*> parents;
    parents << mRootItem;

    SymbolReferenceItem* symbolRef = mReference->findReference(mCurrentSymbolID);
    insertSymbolReference(parents, symbolRef->declare(), "Declared");
    insertSymbolReference(parents, symbolRef->define(), "Defined");
    insertSymbolReference(parents, symbolRef->assign(), "Assigned");
    insertSymbolReference(parents, symbolRef->implicitAssign(), "Implicit Assigned");
    insertSymbolReference(parents, symbolRef->control(), "Controlled");
    insertSymbolReference(parents, symbolRef->reference(), "Referenced");
    insertSymbolReference(parents, symbolRef->index(), "Indexed");

    endResetModel();
}

void ReferenceTreeModel::insertSymbolReference(QList<ReferenceItemModel *>& parents, const QList<ReferenceItem *>& referenceItemList, const QString& referenceType)
{
    QList<QVariant> columnData;
    columnData <<  QString("(%1) %2 %3").arg(referenceItemList.size()).arg(referenceType).arg((referenceItemList.size()==0)?"":"in")
                << "" << "";
    parents.last()->appendChild(new ReferenceItemModel(columnData, parents.last()));

    parents << parents.last()->child(parents.last()->childCount()-1);
    foreach(const ReferenceItem* item, referenceItemList) {
        QList<QVariant> itemData;
        itemData << item->location;
        itemData << QString::number(item->lineNumber);
        itemData << QString::number(item->columnNumber);
        parents.last()->appendChild(new ReferenceItemModel(itemData, parents.last()));
    }
    parents.pop_back();
}


} // namespace studio
} // namespace gams
