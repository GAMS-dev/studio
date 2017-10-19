#include "gdxsymboltablemodel.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolTableModel::GdxSymbolTableModel(QList<std::shared_ptr<GDXSymbol>> data, QObject *parent)
    : QAbstractTableModel(parent), mData(data)
{
    mHeaderText.append("Entry");
    mHeaderText.append("Name");
    mHeaderText.append("Type");
    mHeaderText.append("Dimension");
    mHeaderText.append("Nr Records");
}

QVariant GdxSymbolTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
            if (section < mHeaderText.size())
                return mHeaderText.at(section);
    }
    return QVariant();
}

int GdxSymbolTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mData.size();
}

int GdxSymbolTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeaderText.size();
}

QVariant GdxSymbolTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole)
        switch(index.column())
        {
        case 0: return mData.at(index.row())->nr(); break;
        case 1: return mData.at(index.row())->name(); break;
        case 2: return mData.at(index.row())->type(); break;
        case 3: return mData.at(index.row())->dim(); break;
        case 4: return mData.at(index.row())->recordCount(); break;
        }
    return QVariant();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
