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
namespace reference {

class SymbolTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SymbolTableModel(Reference* ref, SymbolDataType::SymbolType type, QObject *parent = nullptr);

    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    void resetModel();

    int getSortedIndexOf(const SymbolId id) const;
    void toggleSearchColumns(bool checked);
    void setFilterPattern(const QString& pattern);

    static const int COLUMN_SYMBOLID = 0;

signals:
    void symbolSelectionToBeUpdated();

private:
    enum SortType {
        sortInt = 0,
        sortString = 1,
        sortUnknown = 2
    };
    enum ColumnType {
        columnId = 0,
        columnName = 1,
        columnType = 2,
        columnDimension = 3,
        columnDomain = 4,
        columnText = 5,
        columnFileLocation = 6,
        columnUnknown = 7
    };
    SortType getSortTypeOf(int column) const;
    ColumnType getColumnTypeOf(int column) const;
    QString getDomainStr(const QList<SymbolId>& domain) const;
    bool isFilteredActive(SymbolReferenceItem* item, int column, const QString& pattern);
    bool isLocationFilteredActive(int idx, const QString& pattern);
    void filterRows();

    SymbolDataType::SymbolType mType;

    QStringList mAllSymbolsHeader;
    QStringList mSymbolsHeader;
    QStringList mFileHeader;
    QStringList mFileUsedHeader;

    Reference* mReference = nullptr;

    int mFilteredKeyColumn = -1;
    QString mFilteredPattern = "";

    size_t mFilteredRecordSize = 0;
    std::vector<bool> mFilterActive;
    std::vector<size_t> mFilterIdxMap;
    std::vector<size_t> mSortIdxMap;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // SYMBOLTABLEMODEL_H
