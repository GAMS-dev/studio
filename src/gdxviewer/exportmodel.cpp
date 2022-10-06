#include "exportmodel.h"
#include "gdxviewer.h"
#include "gdxsymboltablemodel.h"
#include "gdxsymbol.h"

#include <QDebug>

namespace gams {
namespace studio {
namespace gdxviewer {

ExportModel::ExportModel(GdxViewer* gdxViewer, GdxSymbolTableModel *symbolTableModel, QObject *parent)
    : QAbstractTableModel(parent), mGdxViewer(gdxViewer), mSymbolTableModel(symbolTableModel)
{
    mChecked.resize(mSymbolTableModel->symbolCount()+1);
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
    return mSymbolTableModel->headerData(section-1, orientation, role);
}

int ExportModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mSymbolTableModel->rowCount(parent);
}

int ExportModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mSymbolTableModel->columnCount(parent) + 1;
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
    QModelIndex idx = mSymbolTableModel->index(index.row(), index.column()-1);
    return mSymbolTableModel->data(idx, role);
    return QVariant();
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
    for (int r=1; r<mSymbolTableModel->symbolCount()+1; r++) { // r=1 to skip universe symbol
        if (mChecked[r])
            l.append(mSymbolTableModel->gdxSymbols().at(r));
    }
    return l;
}

void ExportModel::selectAll()
{
    for (int r=1; r<mSymbolTableModel->symbolCount()+1; r++) // r=1 to skip universe symbol
        mChecked[r] = true;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void ExportModel::deselectAll()
{
    for (int r=1; r<mSymbolTableModel->symbolCount()+1; r++) // r=1 to skip universe symbol
        mChecked[r] = false;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
