/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "filteruelmodel.h"
#include "gdxsymbol.h"
#include "gdxsymboltablemodel.h"

#include <QTime>

namespace gams {
namespace studio {
namespace gdxviewer {

FilterUelModel::FilterUelModel(GdxSymbol *symbol, int column, QObject *parent)
    : QAbstractListModel(parent), mSymbol(symbol), mColumn(column)
{
    mUels = mSymbol->uelsInColumn().at(mColumn);
    mChecked = new bool[mUels->size()];
    bool* showUelInColumn = mSymbol->showUelInColumn().at(column);
    for(size_t idx=0; idx<mUels->size(); idx++)
        mChecked[idx] = showUelInColumn[mUels->at(idx)];
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
    if (parent.isValid())
        return 0;
    return int(mUels->size());
}

QVariant FilterUelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole) {
        int uel = mUels->at(index.row());
        return mSymbol->gdxSymbolTable()->uel2Label(uel);
    }
    else if(role == Qt::CheckStateRole) {
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
    if (index.isValid())
        f |= Qt::ItemIsUserCheckable;
    return f;
}

bool FilterUelModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role==Qt::CheckStateRole)
        mChecked[index.row()] = value.toBool();

    emit dataChanged(index, index);
    return true;
}

bool *FilterUelModel::checked() const
{
    return mChecked;
}

void FilterUelModel::filterLabels(const QRegularExpression &regExp)
{
    bool checkedOld, checkedNew;
    for(size_t idx=0; idx<mUels->size(); idx++) {
        int uel = mUels->at(idx);
        checkedOld = mChecked[idx];
        if(regExp.match(mSymbol->gdxSymbolTable()->uel2Label(uel)).hasMatch())
            checkedNew = true;
        else
            checkedNew = false;
        if(checkedNew != checkedOld) {
            mChecked[idx] = checkedNew;
            emit dataChanged(index(int(idx),0), index(int(idx),0));
        }
    }
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
