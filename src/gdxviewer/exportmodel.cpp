/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "exportmodel.h"
#include "gdxviewer.h"
#include "gdxsymboltablemodel.h"
#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

ExportModel::ExportModel(GdxSymbolTableModel* gdxSymbolTableModel, QObject *parent)
    : QAbstractTableModel(parent),
      mGdxSymbolTableModel(gdxSymbolTableModel)
{
    mChecked.resize(mGdxSymbolTableModel->symbolCount()+1);
}

ExportModel::~ExportModel()
{

}

QVariant ExportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (section==0) {
            if(role == Qt::DisplayRole) {
                return "Export";
            }
        }
    }
    return mGdxSymbolTableModel->headerData(section-1, orientation, role);
}

int ExportModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mGdxSymbolTableModel->rowCount(parent);
}

int ExportModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mGdxSymbolTableModel->columnCount(parent) + 1;
}

QVariant ExportModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() == 0) {
        if(role == Qt::CheckStateRole) {
            if(mChecked[index.row()])
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
        return QVariant();
    }
    QModelIndex idx = mGdxSymbolTableModel->index(index.row(), index.column()-1);
    return mGdxSymbolTableModel->data(idx, role);
}

bool ExportModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() == 0) {
        if (role==Qt::CheckStateRole) {
            mChecked[index.row()] = value.toBool();
            emit dataChanged(index, index);
        }
    }
    return true;
}

Qt::ItemFlags ExportModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.isValid()) {
        if (index.column() == 0)
            f |= Qt::ItemIsUserCheckable;
    }
    return f;
}

QList<GdxSymbol *> ExportModel::selectedSymbols()
{
    QList<GdxSymbol*> l = QList<GdxSymbol*>();
    for (int r=1; r<mGdxSymbolTableModel->symbolCount()+1; r++) { // r=1 to skip universe symbol
        if (mChecked[r])
            l.append(mGdxSymbolTableModel->gdxSymbols().at(r));
    }
    return l;
}

void ExportModel::selectAll()
{
    for (int r=1; r<mGdxSymbolTableModel->symbolCount()+1; r++) // r=1 to skip universe symbol
        mChecked[r] = true;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void ExportModel::deselectAll()
{
    for (int r=1; r<mGdxSymbolTableModel->symbolCount()+1; r++) // r=1 to skip universe symbol
        mChecked[r] = false;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
