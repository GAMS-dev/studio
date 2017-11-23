#include "filteruelmodel.h"

#include <QTime>
#include <QDebug>

namespace gams {
namespace studio {
namespace gdxviewer {

FilterUelModel::FilterUelModel(GdxSymbol *symbol, int column, QObject *parent)
    : QAbstractListModel(parent), mSymbol(symbol), mColumn(column)
{
    mfilterUels = mSymbol->filterUels().at(mColumn);
}

FilterUelModel::~FilterUelModel()
{
    if (mUels)
        delete mUels;
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
    return mSymbol->filterUels().at(mColumn)->count();

}

QVariant FilterUelModel::data(const QModelIndex &index, int role) const
{
    //qDebug() << "data: " << index.row();
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        int uel = mfilterUels->keys().at(index.row());
        return mSymbol->gdxSymbolTable()->uel2Label().at(uel);
    }
    else if(role == Qt::CheckStateRole)
    {
        if(mChanged.find(mfilterUels->keys().at(index.row())) != mChanged.end())
        {
            if(mChanged.find( mfilterUels->keys().at(index.row())).value())
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
        if (mfilterUels->values().at(index.row()))
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
        mChanged.insert(mfilterUels->keys().at(index.row()), value.toBool());

    dataChanged(index, index, QVector<int>(Qt::CheckStateRole));
    return true;
}

int *FilterUelModel::uels() const
{
    return mUels;
}

QMap<int, bool> FilterUelModel::changed() const
{
    return mChanged;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
