#include "solvertablemodel.h"

#include <QDebug>

namespace gams {
namespace studio {
namespace support {

SolverTableModel::SolverTableModel(QObject *parent)
    : QAbstractTableModel(parent),
      mHorizontalHeaderData(mLicenseInfo.modelTypeNames())
{
    mHorizontalHeaderData[0] = "License";
}

QVariant SolverTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal && section < mHorizontalHeaderData.size()) {
        return mHorizontalHeaderData.value(section);
    }

    if (orientation == Qt::Vertical && section < mLicenseInfo.solvers()) {
        return mLicenseInfo.solverName(section);
    }

    return QVariant();
}

int SolverTableModel::columnCount(const QModelIndex &parent) const
{
    return mHorizontalHeaderData.size();
}

int SolverTableModel::rowCount(const QModelIndex &parent) const
{
    return mLicenseInfo.solvers();
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
