#include "filteruelmodel.h"


namespace gams {
namespace studio {
namespace gdxviewer {

FilterUelModel::FilterUelModel(GdxSymbol *symbol, int column, QObject *parent)
    : QAbstractListModel(parent), mSymbol(symbol), mColumn(column)
{
    QSet<int>* uelsInColumn = mSymbol->uelsInColumn().at(column);
    QSet<int>* filterUels = symbol->filterUels().at(column);

    mUels = new int[uelsInColumn->count()];
    mChecked = new bool[uelsInColumn->count()];

    for(int i=0; i<uelsInColumn->count(); i++)
    {
        int uel = uelsInColumn->values().at(i);
        mUels[i] = uel;
        if(filterUels->find(uel) == filterUels->end())
            mChecked[i] = false;
        else
            mChecked[i] = true;
    }
}

FilterUelModel::~FilterUelModel()
{
    if (mUels)
        delete mUels;
    if (mChecked)
        delete mChecked;
}

QVariant FilterUelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

int FilterUelModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return mSymbol->uelsInColumn().at(mColumn)->count();
}

QVariant FilterUelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        return mSymbol->uel2Label()->at(mUels[index.row()]);
    }
    else if(role == Qt::CheckStateRole)
    {
        if (mChecked[index.row()])
            return Qt::Checked;
        else
            return Qt::Unchecked;
    }
    return QVariant();
}

Qt::ItemFlags FilterUelModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractListModel::flags(index);
    if(index.isValid())
        f |= Qt::ItemIsUserCheckable;
    return f;
}

bool FilterUelModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role==Qt::CheckStateRole)
        mChecked[index.row()] = value.toBool();
    return true;
}

int *FilterUelModel::uels() const
{
    return mUels;
}

bool *FilterUelModel::checked() const
{
    return mChecked;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
