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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <QTableView>
#include <QStringConverter>

#include "gdxcc.h"
#include "numerics/doubleformatter.h"

class QMutex;

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolTableModel;
class TableViewModel;
class ColumnFilter;
class ValueFilter;

class GdxSymbol : public QAbstractTableModel
{
    Q_OBJECT

    friend class TableViewModel;

public:
    enum DecimalSeparator {studio, system, custom};

    explicit GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, int nr,
                       GdxSymbolTableModel* gdxSymbolTable, QObject *parent = nullptr);
    ~GdxSymbol() override;

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
    bool loadData();
    void stopLoadingData();
    bool isAllDefault(int valColIdx);
    bool hasInvalidUel() const;
    int subType() const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void filterRows();
    int sortColumn() const;
    Qt::SortOrder sortOrder() const;
    void resetSortFilter();
    GdxSymbolTableModel *gdxSymbolTable() const;
    std::vector<std::vector<int> *> uelsInColumn() const;
    std::vector<bool *> showUelInColumn() const;
    void setShowUelInColumn(const std::vector<bool *> &showUelInColumn);

    bool filterActive(int column) const;
    void setFilterActive(int column, bool active=true);

    int tvColDim() const;

    void setNumericalPrecision(int numericalPrecision, bool squeezeTrailingZeroes);

    double minDouble(int valCol=0);
    double maxDouble(int valCol=0);

    void registerColumnFilter(int column, ColumnFilter *columnFilter);
    void registerValueFilter(int valueColumn, ValueFilter *valueFilter);
    void unregisterColumnFilter(int column);
    void unregisterValueFilter(int valueColumn);
    void unregisterAllFilters();
    ValueFilter* valueFilter(int valueColumn);
    ColumnFilter* columnFilter(int column);

    int filterColumnCount();

    void setNumericalFormat(const numerics::DoubleFormatter::Format &numericalFormat);

    QStringList domains() const;

    static const QList<QString> superScript;

    int numericalColumnCount() const;

    GdxSymbol *aliasedSymbol();

    void updateDecSepCopy();


    void setSearchRegEx(const QRegularExpression &searchRegEx);

    QRegularExpression searchRegEx() const;

    bool isDataTruncated() const;

signals:
    void loadFinished();
    void loadPaused();
    void triggerListViewAutoResize();
    void truncatedData(bool isTruncated) const;
    void filterChanged();

private:
    void calcDefaultColumns();
    void calcDefaultColumnsTableView();
    void calcUelsInColumn();
    void loadMetaData();
    void loadDomains();
    double specVal2SortVal(double val);
    QVariant formatValue(double val, bool dynamicDecSep=false) const;
    QRegularExpression mSearchRegEx;

private:
    static const int MAX_DISPLAY_RECORDS;

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
    bool mHasInvalidUel = false;
    QStringDecoder decode;

    std::vector<int> mMinUel;
    std::vector<int> mMaxUel;

    std::vector<double> mMinDouble;
    std::vector<double> mMaxDouble;

    GdxSymbolTableModel* mGdxSymbolTable = nullptr;

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

    int mNumericalPrecision = 6;
    numerics::DoubleFormatter::Format mNumericalFormat = numerics::DoubleFormatter::g;
    bool mSqueezeTrailingZeroes = true;

    std::vector<ColumnFilter*> mColumnFilters;
    std::vector<ValueFilter*> mValueFilters;
    int mNumericalColumnCount;

    QChar mDecSepCopy;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
