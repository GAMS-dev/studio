#include "optiondefinitionitem.h"

namespace gams {
namespace studio {

OptionDefinitionItem::OptionDefinitionItem(const QList<QVariant>& data, OptionDefinitionItem* parent)
{
    mParentItem = parent;
    mItemData = data;
}

OptionDefinitionItem::~OptionDefinitionItem()
{
    qDeleteAll(mChildItems);
}

void OptionDefinitionItem::appendChild(OptionDefinitionItem *item)
{
    mChildItems.append(item);
}

OptionDefinitionItem *OptionDefinitionItem::child(int row)
{
    return mChildItems.value(row);
}

int OptionDefinitionItem::childCount() const
{
    return mChildItems.count();
}

int OptionDefinitionItem::row() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<OptionDefinitionItem*>(this));

    return 0;
}

int OptionDefinitionItem::columnCount() const
{
    return mItemData.count();
}

QVariant OptionDefinitionItem::data(int column) const
{
    return mItemData.value(column);
}

OptionDefinitionItem *OptionDefinitionItem::parentItem()
{
    return mParentItem;
}


} // namespace studio
} // namespace gams
