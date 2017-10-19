#include "gdxsymboldatatablemodel.h"
#include <memory>

namespace gams {
namespace studio {
namespace gdxviewer {

GDXSymbolDataTableModel::GDXSymbolDataTableModel(std::shared_ptr<GDXSymbol> gdxSymbol, QStringList *uel2Label, QObject *parent)
    : QAbstractTableModel(parent), mGdxSymbol(gdxSymbol), mUel2Label(uel2Label)
{

}

QVariant GDXSymbolDataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (section<mGdxSymbol->dim())
                return "Dim " + QString::number(section+1);
            else
            {
                if (mGdxSymbol->type() == GMS_DT_PAR)
                    return "Value";
                else if (mGdxSymbol->type() == GMS_DT_VAR || GMS_DT_EQU)
                switch(section-mGdxSymbol->dim())
                {
                case GMS_VAL_LEVEL: return "Level";
                case GMS_VAL_MARGINAL: return "Marginal";
                case GMS_VAL_LOWER: return "Lower";
                case GMS_VAL_UPPER: return "Upper";
                case GMS_VAL_SCALE: return "Scale";
                }
            }

        }


    }
    return QVariant();
}

int GDXSymbolDataTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mGdxSymbol->recordCount();
}

int GDXSymbolDataTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (mGdxSymbol->type() == GMS_DT_SET)
        return mGdxSymbol->dim();
    else if (mGdxSymbol->type() == GMS_DT_PAR)
        return mGdxSymbol->dim() + 1;
    else if (mGdxSymbol->type() == GMS_DT_VAR || GMS_DT_EQU)
        return mGdxSymbol->dim() + 5;
}

QVariant GDXSymbolDataTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    else if (role == Qt::DisplayRole)
    {
        if (index.column()< mGdxSymbol->dim())
            return mUel2Label->at(mGdxSymbol->key(index.row(), index.column()));
        else
            return mGdxSymbol->value(index.row(), index.column()-mGdxSymbol->dim());
    }


    return QVariant();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
