#include "lxitreemodel.h"
#include <QDebug>

namespace gams {
namespace studio {
namespace lxiviewer {

LxiTreeModel::LxiTreeModel(LxiTreeItem *root, QVector<int> lineNrs, QVector<LxiTreeItem*> treeItems, QObject *parent)
    : QAbstractItemModel(parent), mRootItem(root), mLineNrs(lineNrs), mTreeItems(treeItems)
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
