/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <QTableView>

#include "gdxcc.h"

class QMutex;

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolTable;
class TableViewModel;
class ValueFilter;

class GdxSymbol : public QAbstractTableModel
{
    Q_OBJECT

    friend class TableViewModel;

public:
    explicit GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, int nr,
                       GdxSymbolTable* gdxSymbolTable, QObject *parent = nullptr);
    ~GdxSymbol() override;
    static int maxPrecision;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int nr() const;
    QString name() const;
    int dim() const;
    int type() const;
    int recordCount() const;
    QString explText() const;
    bool isLoaded() const;
    void loadData();
    void stopLoadingData();
    bool isAllDefault(int valColIdx);
    int subType() const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void filterRows();
    int sortColumn() const;
    Qt::SortOrder sortOrder() const;
    void resetSortFilter();
    GdxSymbolTable *gdxSymbolTable() const;
    std::vector<std::vector<int> *> uelsInColumn() const;
    std::vector<bool *> showUelInColumn() const;
    void setShowUelInColumn(const std::vector<bool *> &showUelInColumn);

    bool filterActive(int column) const;
    void setFilterActive(int column, bool active=true);

    int tvColDim() const;

    bool filterHasChanged() const;
    void setFilterHasChanged(bool filterHasChanged);

    void setNumericalPrecision(int numericalPrecision, bool squeezeTrailingZeroes);

    double minDouble(int valCol=0);
    double maxDouble(int valCol=0);

    void registerValueFilter(int valueColumn, ValueFilter *valueFilter);
    void unregisterValueFilter(int valueColumn);
    ValueFilter* valueFilter(int valueColumn);

    int filterColumnCount();

    static QString formatNumericalValue(double val, int precision, bool squeezeTrailingZeroes);

signals:
    void loadFinished();
    void triggerListViewAutoResize();

private:
    void calcDefaultColumns();
    void calcDefaultColumnsTableView();
    void calcUelsInColumn();
    void loadMetaData();
    void loadDomains();
    double specVal2SortVal(double val);
    QVariant formatValue(double val) const;

private:
    void initNumericalBounds();
    gdxHandle_t mGdx = nullptr;
    int mNr;
    QMutex* mGdxMutex = nullptr;
    int mDim;
    int mType;
    int mSubType;
    int mRecordCount;
    QString mExplText;
    QString mName;

    std::vector<int> mMinUel;
    std::vector<int> mMaxUel;

    std::vector<double> mMinDouble;
    std::vector<double> mMaxDouble;

    GdxSymbolTable* mGdxSymbolTable = nullptr;

    bool mIsLoaded = false;
    int mLoadedRecCount = 0;
    int mFilterRecCount = 0;

    bool stopLoading = false;

    std::vector<uint> mKeys;
    std::vector<double> mValues;

    QStringList mDomains;

    bool mDefaultColumn[GMS_VAL_MAX] {false};

    std::vector<double> mSpecValSortVal;

    std::vector<std::vector<int>*> mUelsInColumn;
    std::vector<bool*> mShowUelInColumn;
    std::vector<bool> mFilterActive;

    std::vector<int> mRecSortIdx;
    std::vector<int> mRecFilterIdx;

    bool mFilterHasChanged = false;

    int mNumericalPrecision = 6;
    bool mSqueezeTrailingZeroes = true;

    std::vector<ValueFilter*> mValueFilters;
    int mNumericalColumnCount;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
