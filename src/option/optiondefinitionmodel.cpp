#include "optiondefinitionmodel.h"

namespace gams {
namespace studio {

OptionDefinitionModel::OptionDefinitionModel(Option* data, QObject* parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Option" << "Synonym" << "DefValue" // << "Range"
             << "Type" << "Description";
    rootItem = new OptionDefinitionItem(rootData);

    setupTreeItemModelData(data, rootItem);
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
        if (optdef.deprecated)
            continue;
        QList<QVariant> columnData;

        QString optionStr = optdef.name
                              + (optdef.synonym.isEmpty() ? "" : QString("[%1]").arg(optdef.synonym));
        columnData.append(optdef.name);
        columnData.append(optdef.synonym);
        columnData.append(optdef.defaultValue);
        switch(optdef.type){
        case optTypeInteger :
//            columnData.append( QString("[%1,%2]").arg( optdef.lowerBound.canConvert<int>() ? optdef.lowerBound.toInt() : optdef.lowerBound.toDouble() )
//                                                 .arg( optdef.upperBound.canConvert<int>() ? optdef.upperBound.toInt() : optdef.upperBound.toDouble() ) );
            columnData.append("Integer");
            break;
        case optTypeDouble :
//            columnData.append( QString("[%1,%2]").arg( optdef.lowerBound.canConvert<int>() ? optdef.lowerBound.toInt() : optdef.lowerBound.toDouble() )
//                                                 .arg( optdef.upperBound.canConvert<int>() ? optdef.upperBound.toInt() : optdef.upperBound.toDouble() ) );
            columnData.append("Double");
            break;
        case optTypeString :
//            columnData.append("");
            columnData.append("String");
            break;
        case optTypeBoolean :
//            columnData.append("");
            columnData.append("Boolean");
            break;
        case optTypeEnumStr :
//            columnData.append("");
            columnData.append("EnumStr");
            break;
        case optTypeEnumInt :
//            columnData.append("");
            columnData.append("EnumInt");
            break;
        case optTypeMultiList :
//            columnData.append("");
            columnData.append("MultiList");
            break;
        case optTypeStrList   :
//            columnData.append("");
            columnData.append("StrList");
            break;
        case optTypeMacro     :
//            columnData.append("");
            columnData.append("Macro");
            break;
        case optTypeImmediate :
//            columnData.append("");
            columnData.append("Immediate");
            break;
        default:
//            columnData.append("");
            columnData.append("");
            break;
        }
        columnData.append(optdef.description);
        parents.last()->appendChild(new OptionDefinitionItem(columnData, parents.last()));

        if (optdef.valueList.size() > 0) {
            parents << parents.last()->child(parents.last()->childCount()-1);
            for(int j=0; j<optdef.valueList.size(); ++j) {
                OptionValue enumValue = optdef.valueList.at(j);
                if (enumValue.hidden)
                    continue;
                QList<QVariant> enumData;
                enumData << enumValue.value;
                enumData << "";
                enumData << "";
                enumData << "";
//                enumData << "";
                enumData << enumValue.description;
                parents.last()->appendChild(new OptionDefinitionItem(enumData, parents.last()));
            }
            parents.pop_back();
        }
    }
}

} // namespace studio
} // namespace gams
