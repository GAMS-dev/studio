#include "optiondefinitionmodel.h"

namespace gams {
namespace studio {

OptionDefinitionModel::OptionDefinitionModel(Option* data, QObject* parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Option" << "Synonym" << "DefValue" << "Description";
    rootItem = new OptionDefinitionItem(rootData);

    setupTreeItemModelData(data, rootItem);
//    setupModelData(data.split(QString("\n")), rootItem);
}

OptionDefinitionModel::~OptionDefinitionModel()
{
    delete rootItem;
}

int OptionDefinitionModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return static_cast<OptionDefinitionItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant OptionDefinitionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    OptionDefinitionItem* item = static_cast<OptionDefinitionItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags OptionDefinitionModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant OptionDefinitionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex OptionDefinitionModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    OptionDefinitionItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<OptionDefinitionItem*>(parent.internalPointer());

    OptionDefinitionItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex OptionDefinitionModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    OptionDefinitionItem *childItem = static_cast<OptionDefinitionItem*>(index.internalPointer());
    OptionDefinitionItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int OptionDefinitionModel::rowCount(const QModelIndex& parent) const
{
    OptionDefinitionItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<OptionDefinitionItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void OptionDefinitionModel::setupTreeItemModelData(Option* option, OptionDefinitionItem* parent)
{
    QList<OptionDefinitionItem*> parents;
    parents << parent;

    for(auto it = option->getOption().cbegin(); it != option->getOption().cend(); ++it)  {
        OptionDefinition optdef =  it.value();
        QList<QVariant> columnData;

        QString optionStr = optdef.name
                              + (optdef.synonym.isEmpty() ? "" : QString("[%1]").arg(optdef.synonym));
        columnData.append(optdef.name);
        columnData.append(optdef.synonym);
        columnData.append(optdef.defaultValue);
        columnData.append(optdef.description);
        parents.last()->appendChild(new OptionDefinitionItem(columnData, parents.last()));
    }
}

void OptionDefinitionModel::setupModelData(const QStringList& lines, OptionDefinitionItem* parent)
{
    QList<OptionDefinitionItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].at(position) != ' ')
                break;
            position++;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QList<QVariant> columnData;
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            parents.last()->appendChild(new OptionDefinitionItem(columnData, parents.last()));
        }

        ++number;
    }
}

} // namespace studio
} // namespace gams
