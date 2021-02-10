#include "tableviewdomainmodel.h"

namespace gams {
namespace studio {
namespace gdxviewer {

TableViewDomainModel::TableViewDomainModel(TableViewModel* tvModel, QObject *parent)
    : QAbstractTableModel(parent), mTvModel(tvModel)
{
    connect(mTvModel, &TableViewModel::modelReset, [this]() { emit beginResetModel(); emit endResetModel();});
}

QVariant TableViewDomainModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (section < mTvModel->dim())
            return mTvModel->domains().at(mTvModel->tvDimOrder().at(section));
        else {
            if (mTvModel->type() == GMS_DT_PAR)
                return QVariant("Value");
            else if (mTvModel->type() == GMS_DT_VAR || mTvModel->type() == GMS_DT_EQU) {
                switch(section-mTvModel->dim()) {
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

int TableViewDomainModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 0;
}

int TableViewDomainModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (mTvModel->type() == GMS_DT_PAR)
        return mTvModel->dim() + 1;
    if (mTvModel->type() == GMS_DT_EQU || mTvModel->type() == GMS_DT_VAR)
        return mTvModel->dim() + GMS_VAL_MAX;
    return mTvModel->dim();
}

QVariant TableViewDomainModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    return QVariant();
}

TableViewModel *TableViewDomainModel::tvModel() const
{
    return mTvModel;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
