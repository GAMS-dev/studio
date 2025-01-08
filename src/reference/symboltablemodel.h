/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
    enum FileUsedSortOrder
    {
        AscendingOrder = 0,
        DescendingOrder = 1,
        OriginalOrder = 2
    };
    Q_ENUM(FileUsedSortOrder)

    explicit SymbolTableModel(SymbolDataType::SymbolType type, QObject *parent = nullptr);


    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    void resetModel();
    void initModel(Reference* ref);

    bool isModelLoaded();

    int getSortedIndexOf(const SymbolId id) const;
    int getSortedIndexOf(const QString &name) const;
    void toggleSearchColumns(bool checked);
    int getLastSectionIndex();

    static const int COLUMN_SYMBOL_ID = 0;
    static const int COLUMN_SYMBOL_NAME = 1;
    static const int COLUMN_FILEUSED_NAME = 0;

signals:
    void symbolSelectionToBeUpdated();

public slots:
    void sortFileUsed(gams::studio::reference::SymbolTableModel::FileUsedSortOrder order =  FileUsedSortOrder::OriginalOrder);
    void setFilterPattern(const QRegularExpression &pattern);

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
    bool isFilteredActive(SymbolReferenceItem* item, int column, const QRegularExpression &regExp);
    bool isLocationFilteredActive(int idx, const QRegularExpression &regExp);
    void filterRows();
    void resetSizeAndIndices();

    SymbolDataType::SymbolType mType;

    QStringList mAllSymbolsHeader;
    QStringList mSymbolsHeader;
    QStringList mFileHeader;
    QStringList mFileUsedHeader;

    Reference* mReference = nullptr;

    int mFilteredKeyColumn = -1;
    QRegularExpression mFilteredPattern;
    int mCurrentSortedColumn = 0;
    Qt::SortOrder mCurrentAscendingSort = Qt::AscendingOrder;

    size_t mFilteredRecordSize = 0;
    std::vector<bool> mFilterActive;
    std::vector<size_t> mFilterIdxMap;
    std::vector<size_t> mSortIdxMap;
};

} // namespace reference
} // namespace studio
} // namespace gams

Q_DECLARE_METATYPE(gams::studio::reference::SymbolTableModel::FileUsedSortOrder);

#endif // SYMBOLTABLEMODEL_H
