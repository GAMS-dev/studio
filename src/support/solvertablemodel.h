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
#ifndef SOLVERTABLEMODEL_H
#define SOLVERTABLEMODEL_H

#include <QAbstractTableModel>

#include "gamslicenseinfo.h"

namespace gams {
namespace studio {
namespace support {

class SolverTableModel
        : public QAbstractTableModel
{
    Q_OBJECT

public:
    SolverTableModel(QObject *parent = nullptr);

    virtual QVariant headerData(int section, Qt::Orientation oriantation, int role = Qt::DisplayRole) const override;

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual int rowCount(const QModelIndex &parennt = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    GamsLicenseInfo mLicenseInfo;
    QMap<int, QString> mHorizontalHeaderData;
    QMap<int, QString> mVerticalHeaderData;
    QMap<int, int> mVerticalHeaderIndices;

    const int RowShift = 1;
};

}
}
}

#endif // SOLVERTABLEMODEL_H
