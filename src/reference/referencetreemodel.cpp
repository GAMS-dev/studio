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
#include <QDebug>
#include "referencetreemodel.h"
#include "referenceitemmodel.h"

namespace gams {
namespace studio {
namespace reference {

ReferenceTreeModel::ReferenceTreeModel(Reference* ref, QObject *parent) :
    QAbstractItemModel(parent), mReference(ref), mCurrentSymbolID(-1)
{
    QList<QVariant> rootData;
    rootData << "Location" << "Line" << "Column" << "Type" << "Referred";
    mRootItem = new ReferenceItemModel(rootData);
}

ReferenceTreeModel::~ReferenceTreeModel()
{
    delete mRootItem;
}

QVariant ReferenceTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();


    ReferenceItemModel* item = static_cast<ReferenceItemModel*>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole: {
        if (index.column()==0) {
            QFileInfo fileInfo(item->data(index.column()).toString());
            return QString(fileInfo.fileName());
        }
        return item->data(index.column());
    }
    case Qt::ToolTipRole: {
        ReferenceItemModel* item = static_cast<ReferenceItemModel*>(index.internalPointer());
        ReferenceItemModel* parentItem = item->parent();

        if (parentItem == mRootItem) {
            QString name = item->data(ReferenceItemModel::COLUMN_REFERENCE_TYPE).toString();
            QString description = ReferenceDataType::from(name).description();
            return QString("%1 : %2").arg(name, description);
        } else {
            return QString("%1 : Line %2 : Column %3").arg(item->data(ReferenceItemModel::COLUMN_LOCATION).toString(),
                                                           item->data(ReferenceItemModel::COLUMN_LINE_NUMBER).toString(),
                                                           item->data(ReferenceItemModel::COLUMN_COLUMN_NUMBER).toString());
        }
    }
    case Qt::TextAlignmentRole: {
        Qt::AlignmentFlag aFlag;
        switch(index.column()) {
            case 0: aFlag = Qt::AlignLeft; break;
            case 1: aFlag = Qt::AlignRight; break;
            case 2: aFlag = Qt::AlignLeft; break;
            default: aFlag = Qt::AlignLeft; break;
        }
        return QVariant(aFlag | Qt::AlignVCenter);
    }
    case Qt::UserRole: {
        ReferenceItemModel* item = static_cast<ReferenceItemModel*>(index.internalPointer());
        ReferenceItemModel* parentItem = item->parent();

        if (parentItem != mRootItem)
            return item->data(index.column());
        break;
    }
    default:
        break;
    }

    return QVariant();
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
    ReferenceItemModel* parentItem = childItem->parent();

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

void ReferenceTreeModel::resetModel()
{
    beginResetModel();

    if (rowCount() > 0) {
        removeRows(0, rowCount(), QModelIndex());
    }

    endResetModel();
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

    SymbolReferenceItem* symbolRef = mReference->findReferenceFromId(mCurrentSymbolID);
    if (symbolRef) {
        insertSymbolReference(parents, symbolRef->declare(), ReferenceDataType::ReferenceType::Declare);
        insertSymbolReference(parents, symbolRef->define(), ReferenceDataType::ReferenceType::Define);
        insertSymbolReference(parents, symbolRef->assign(), ReferenceDataType::ReferenceType::Assign);
        insertSymbolReference(parents, symbolRef->implicitAssign(), ReferenceDataType::ReferenceType::ImplicitAssign);
        insertSymbolReference(parents, symbolRef->control(), ReferenceDataType::ReferenceType::Control);
        insertSymbolReference(parents, symbolRef->index(), ReferenceDataType::ReferenceType::Index);
        insertSymbolReference(parents, symbolRef->reference(), ReferenceDataType::ReferenceType::Reference);
    }
    endResetModel();
}

void ReferenceTreeModel::updateSelectedSymbol(const QString &symbolName)
{
    mCurrentSymbolID = -1;

    beginResetModel();

    if (rowCount() > 0) {
        removeRows(0, rowCount(), QModelIndex());
    }

    QList<ReferenceItemModel*> parents;
    parents << mRootItem;

    SymbolReferenceItem* symbolRef = mReference->findReferenceFromName(symbolName);
    if (symbolRef) {
        mCurrentSymbolID = symbolRef->id();
        insertSymbolReference(parents, symbolRef->declare(), ReferenceDataType::ReferenceType::Declare );
        insertSymbolReference(parents, symbolRef->define(), ReferenceDataType::ReferenceType::Define);
        insertSymbolReference(parents, symbolRef->assign(), ReferenceDataType::ReferenceType::Assign);;
        insertSymbolReference(parents, symbolRef->implicitAssign(), ReferenceDataType::ReferenceType::ImplicitAssign);
        insertSymbolReference(parents, symbolRef->control(), ReferenceDataType::ReferenceType::Control);
        insertSymbolReference(parents, symbolRef->index(), ReferenceDataType::ReferenceType::Index);
        insertSymbolReference(parents, symbolRef->reference(), ReferenceDataType::ReferenceType::Reference);
    }
    endResetModel();
}

void ReferenceTreeModel::insertSymbolReference(QList<ReferenceItemModel*>& parents, const QList<ReferenceItem *>& referenceItemList, ReferenceDataType::ReferenceType type)  //, const QString& referenceType)
{  
    QList<QVariant> columnData;
    columnData <<  QString("(%1) %2 %3").arg(referenceItemList.size()).arg(ReferenceDataType::from(type).name(), (referenceItemList.size()==0)?"":"in")
               << "" << ""  << ReferenceDataType::from(type).name() << 0;
    parents.last()->appendChild(new ReferenceItemModel(columnData, parents.last()));

    parents << parents.last()->child(parents.last()->childCount()-1);
    for (const ReferenceItem* item: referenceItemList) {
        QList<QVariant> itemData;
        itemData << QString(item->location);
        itemData << QString::number(item->lineNumber);
        itemData << QString::number(item->columnNumber);
        itemData << ReferenceDataType::from(type).name();
        itemData << 1;
        parents.last()->appendChild(new ReferenceItemModel(itemData, parents.last()));
    }
    parents.pop_back();
}

} // namespace reference
} // namespace studio
} // namespace gams
