#include "lxitreemodel.h"
#include <QDebug>

namespace gams {
namespace studio {
namespace lxiviewer {

LxiTreeModel::LxiTreeModel(LxiTreeItem *root, QObject *parent)
    : mRootItem(root)
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
    if (childItem)
        return createIndex(row, column, childItem);
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

    return createIndex(parentItem->row(), 0, parentItem);
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

} // namespace lxiviewer
} // namespace studio
} // namespace gams
