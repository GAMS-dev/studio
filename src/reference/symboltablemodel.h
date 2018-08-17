/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef SYMBOLTABLEMODEL_H
#define SYMBOLTABLEMODEL_H

#include <QAbstractTableModel>
#include "reference.h"

namespace gams {
namespace studio {

class SymbolTableModel : public QAbstractTableModel
{
public:
    SymbolTableModel(Reference* ref, SymbolDataType::SymbolType type, QObject *parent = nullptr);

    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

private:
    SymbolDataType::SymbolType mType;

    QStringList mAllSymbolsHeader;
    QStringList mSymbolsHeader;
    QStringList mFileHeader;

    Reference* mReference = nullptr;
};

} // namespace studio
} // namespace gams

#endif // SYMBOLTABLEMODEL_H
