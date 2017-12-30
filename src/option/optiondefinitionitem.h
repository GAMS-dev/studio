#ifndef OPTIONDEFINITIONITEM_H
#define OPTIONDEFINITIONITEM_H

#include <QList>
#include <QVector>

namespace gams {
namespace studio {

class OptionDefinitionItem
{
public:
    OptionDefinitionItem(const QList<QVariant>& data, OptionDefinitionItem* parentItem = 0);
    ~OptionDefinitionItem();

    void appendChild(OptionDefinitionItem *child);

    OptionDefinitionItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    OptionDefinitionItem *parentItem();

private:
    QList<OptionDefinitionItem*> mChildItems;
    QList<QVariant> mItemData;
    OptionDefinitionItem *mParentItem;

};

} // namespace studio
} // namespace gams

#endif // OPTIONDEFINITIONITEM_H
