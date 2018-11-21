#include "solvertablemodel.h"

#include <QDebug>

namespace gams {
namespace studio {
namespace support {

SolverTableModel::SolverTableModel(QObject *parent)
    : QAbstractTableModel(parent),
      mHorizontalHeaderData(mLicenseInfo.modelTypeNames()),
      mVerticalHeaderData(mLicenseInfo.solverNames())
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

    if (orientation == Qt::Vertical && section < mVerticalHeaderData.size()) {
        return mVerticalHeaderData.value(section+RowShift);
    }

    return QVariant();
}

int SolverTableModel::columnCount(const QModelIndex &parent) const
{
    return mHorizontalHeaderData.size();
}

int SolverTableModel::rowCount(const QModelIndex &parent) const
{
    return mVerticalHeaderData.size()-RowShift;
}

QVariant SolverTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() == 0)
        return "--";

    if (mLicenseInfo.solverCapability(index.row(), index.column()))
        return "X";
    return QVariant();
}

}
}
}
