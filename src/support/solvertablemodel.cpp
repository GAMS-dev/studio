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
#include "solvertablemodel.h"

namespace gams {
namespace studio {
namespace support {

SolverTableModel::SolverTableModel(QObject *parent)
    : QAbstractTableModel(parent),
      mHorizontalHeaderData(mLicenseInfo.modelTypeNames()),
      mVerticalHeaderData(mLicenseInfo.solverNames()),
      mVerticalHeaderIndices(mLicenseInfo.solverIndices())
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
    Q_UNUSED(parent)
    return mHorizontalHeaderData.size();
}

int SolverTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mVerticalHeaderData.size();
}

QVariant SolverTableModel::data(const QModelIndex &index, int role) const
{
    if (Qt::TextAlignmentRole == role) {
        return Qt::AlignCenter;
    }
    if (Qt::DisplayRole != role)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    if (index.column() == 0) {
        auto solverName = headerData(index.row(), Qt::Vertical).toString();
        auto solverId = mLicenseInfo.solverId(solverName);
        return mLicenseInfo.solverLicense(solverName, solverId);
    }

    if (mLicenseInfo.solverCapability(mVerticalHeaderIndices.value(index.row()+RowShift), index.column()))
        return "X";
    return QVariant();
}

}
}
}
