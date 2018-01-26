#include "filteruelmodel.h"

#include <QTime>
#include <QDebug>

namespace gams {
namespace studio {
namespace gdxviewer {

FilterUelModel::FilterUelModel(GdxSymbol *symbol, int column, QObject *parent)
    : QAbstractListModel(parent), mSymbol(symbol), mColumn(column)
{
    mUels = mSymbol->uelsInColumn().at(mColumn);
    mChecked = new bool[mUels->size()];
    bool* showUelInColumn = mSymbol->showUelInColumn().at(column);
    for(int idx=0; idx<mUels->size(); idx++)
    {
        mChecked[idx] = showUelInColumn[mUels->at(idx)];
    }
}

FilterUelModel::~FilterUelModel()
{
    if (mChecked)
        delete[] mChecked;
}

QVariant FilterUelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

int FilterUelModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return mUels->size();
}

QVariant FilterUelModel::data(const QModelIndex &index, int role) const
{
    //qDebug() << "data: " << index.row();
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        int uel = mUels->at(index.row());
        return mSymbol->gdxSymbolTable()->uel2Label().at(uel);
    }
    else if(role == Qt::CheckStateRole)
    {
        if(mChecked[index.row()])
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

    dataChanged(index, index);
    return true;
}

bool *FilterUelModel::checked() const
{
    return mChecked;
}

void FilterUelModel::filterLabels(QString filterString)
{
    QTime t;
    t.start();
    bool checkedOld, checkedNew;
    QRegExp regExp(filterString);
    regExp.setCaseSensitivity(Qt::CaseInsensitive);
    regExp.setPatternSyntax(QRegExp::Wildcard);
    for(int idx=0; idx<mUels->size(); idx++)
    {
        int uel = mUels->at(idx);
        checkedOld = mChecked[idx];
        if(regExp.exactMatch(mSymbol->gdxSymbolTable()->uel2Label().at(uel)))
            checkedNew = true;
        else
            checkedNew = false;
        if(checkedNew != checkedOld)
        {
            mChecked[idx] = checkedNew;
            dataChanged(index(idx,0), index(idx,0));
        }
    }
    qDebug() << "fitler elapsed: " << t.elapsed();
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
