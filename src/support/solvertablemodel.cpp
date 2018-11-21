#include "solvertablemodel.h"

namespace gams {
namespace studio {
namespace support {

SolverTableModel::SolverTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    mHorizontalHeaderData << "Sovler" << "License" << mLicenseInfo.modelTypeNames();
}

QVariant SolverTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal && section < mHorizontalHeaderData.size()) {
        return mHorizontalHeaderData.at(section);
    }

    return QVariant();
}

int SolverTableModel::columnCount(const QModelIndex &parent) const
{
    return mHorizontalHeaderData.size();
}

int SolverTableModel::rowCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant SolverTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    return "...";
}

}
}
}
