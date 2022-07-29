#include "schemadefinitionitem.h"

namespace gams {
namespace studio {
namespace connect {

SchemaDefinitionItem::SchemaDefinitionItem(const QString& name, const QList<QVariant>& data, SchemaDefinitionItem* parentItem):
    mSchemaName(name),
    mItemData(data),
    mParentItem(parentItem)
{
}

SchemaDefinitionItem::~SchemaDefinitionItem()
{
    qDeleteAll(mChildItems);
}

void SchemaDefinitionItem::appendChild(SchemaDefinitionItem *child)
{
    mChildItems.append(child);
}

SchemaDefinitionItem *SchemaDefinitionItem::child(int row)
{
    return mChildItems.value(row);
}

SchemaDefinitionItem *SchemaDefinitionItem::parent()
{
     return mParentItem;
}

int SchemaDefinitionItem::childCount() const
{
    return mChildItems.count();
}

int SchemaDefinitionItem::columnCount() const
{
    return mItemData.count();
}

QVariant SchemaDefinitionItem::data(int column) const
{
    return mItemData.value(column);
}

int SchemaDefinitionItem::row() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<SchemaDefinitionItem*>(this));

    return 0;
}

SchemaDefinitionItem *SchemaDefinitionItem::parentItem()
{
    return mParentItem;
}

bool SchemaDefinitionItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= mItemData.size())
        return false;

    mItemData[column] = value;
    return true;
}

void SchemaDefinitionItem::setParent(SchemaDefinitionItem *parent)
{
    mParentItem = parent;
}

void SchemaDefinitionItem::insertChild(int row, SchemaDefinitionItem *item)
{
    item->setParent(this);
    mChildItems.insert(row, item);
}

bool SchemaDefinitionItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > mChildItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete mChildItems.takeAt(position);

    return true;
}

} // namespace connect
} // namespace studio
} // namespace gams
