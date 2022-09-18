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
    qDebug() << "~ExportModel";
}

QVariant ExportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section==0) {
        if(role == Qt::DisplayRole) {
            return "Export";
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
        if (role==Qt::CheckStateRole)
            mChecked[index.row()] = value.toBool();
    }
    return true;
}

Qt::ItemFlags ExportModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.isValid())
        f |= Qt::ItemIsUserCheckable;
    return f;
}

QList<GdxSymbol *> ExportModel::selectedSymbols()
{
    QList<GdxSymbol*> l = QList<GdxSymbol*>();
    for (int r=0; r<mSymbolTableModel->symbolCount()+1; r++) {
        if (mChecked[r])
            l.append(mSymbolTableModel->gdxSymbols().at(r));
    }
    return l;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
