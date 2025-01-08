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
#include "gdxsymbol.h"
#include "exception.h"
#include "gdxsymboltablemodel.h"
#include "nestedheaderview.h"
#include "columnfilter.h"
#include "valuefilter.h"
#include "gdxviewer.h"

#include <QMutex>
#include <QSet>
#include <settings.h>
#include <cmath>
#include "theme.h"

namespace gams {
namespace studio {
namespace gdxviewer {

const int GdxSymbol::MAX_DISPLAY_RECORDS = 107374182;

const QList<QString> GdxSymbol::superScript = QList<QString>({
                                         QString(u8"\u2070"),
                                         QString(u8"\u00B9"),
                                         QString(u8"\u00B2"),
                                         QString(u8"\u00B3"),
                                         QString(u8"\u2074"),
                                         QString(u8"\u2075"),
                                         QString(u8"\u2076"),
                                         QString(u8"\u2077"),
                                         QString(u8"\u2078"),
                                         QString(u8"\u2079"),
                                         QString(u8"\u00B9\u2070"),
                                         QString(u8"\u00B9\u00B9"),
                                         QString(u8"\u00B9\u00B2"),
                                         QString(u8"\u00B9\u00B3"),
                                         QString(u8"\u00B9\u2074"),
                                         QString(u8"\u00B9\u2075"),
                                         QString(u8"\u00B9\u2076"),
                                         QString(u8"\u00B9\u2077"),
                                         QString(u8"\u00B9\u2078"),
                                         QString(u8"\u00B9\u2079"),
                                         QString(u8"\u00B2\u2070"),
                                     });

GdxSymbol::GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, int nr, GdxSymbolTableModel* gdxSymbolTable, QObject *parent)
    : QAbstractTableModel(parent), mGdx(gdx), mNr(nr), mGdxMutex(gdxMutex), mGdxSymbolTable(gdxSymbolTable)
{
    decode = QStringDecoder(QStringConverter::Utf8, QStringConverter::Flag::Stateless);
    loadMetaData();
    loadDomains();

    mRecSortIdx.resize(mRecordCount);
    for(int i=0; i<mRecordCount; i++)
        mRecSortIdx[i] = i;

    mRecFilterIdx.resize(mRecordCount);
    for(int i=0; i<mRecordCount; i++)
        mRecFilterIdx[i] = i;

    mFilterActive.resize(filterColumnCount());
    for(int i=0; i<filterColumnCount(); i++)
        mFilterActive[i] = false;

    mColumnFilters.resize(mDim);
    for (int i=0; i<mDim; i++)
        mColumnFilters[i] = nullptr;
    mValueFilters.resize(mNumericalColumnCount);
    for (int i=0; i<mNumericalColumnCount; i++)
        mValueFilters[i] = nullptr;

    mSpecValSortVal.push_back(5.0E300); // GMS_SV_UNDEF
    mSpecValSortVal.push_back(4.0E300); // GMS_SV_NA
    mSpecValSortVal.push_back(GMS_SV_PINF); // GMS_SV_PINF
    mSpecValSortVal.push_back(-std::numeric_limits<double>::max()); // GMS_SV_MINF
    mSpecValSortVal.push_back(4.94066E-324); // GMS_SV_EPS
    // no entry for acronyms. They are sorted by their internal value (>10.0E300 e.g. 1.35e+303 is the smallest acronym value)
}

GdxSymbol::~GdxSymbol()
{
    for(auto v : mUelsInColumn)
        delete v;
    for(auto a: mShowUelInColumn) {
        if(a)
            delete[] a;
    }
    unregisterAllFilters();
}

QVariant GdxSymbol::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section < mDim)
                return mDomains.at(section) + " " + GdxSymbol::superScript[section+1];
            else {
                if (mType == GMS_DT_SET)
                    return "Text";
                else if (mType == GMS_DT_PAR)
                    return "Value";
                else if (mType == GMS_DT_VAR || mType == GMS_DT_EQU)
                    switch(section-mDim) {
                    case GMS_VAL_LEVEL: return "Level";
                    case GMS_VAL_MARGINAL: return "Marginal";
                    case GMS_VAL_LOWER: return "Lower";
                    case GMS_VAL_UPPER: return "Upper";
                    case GMS_VAL_SCALE: return "Scale";
                    }
            }
        }
    }
    else if (role == Qt::ToolTipRole) {
        QString description("<html><head/><body>");

        if (section < mDim)
            description += "<p><span style=\" font-weight:600;\">Sort: </span>Left click sorts the labels in the column in alphabetical order using a stable sort mechanism. Sorting direction can be changed by clicking again.</p><p><span style=\" font-weight:600;\">Filter:</span> The filter menu can be opened via right click or by clicking on the filter icon.</p>";
        else if (section >= mDim) {
            if (mType == GMS_DT_SET)
                description += "<p><span style=\" font-weight:600;\">Sort: </span>Left click sorts the explanatory text in alphabetical order using a stable sort mechanism. Sorting direction can be changed by clicking again.</p>";
            else
                description += "<p><span style=\" font-weight:600;\">Sort: </span>Left click sorts the numeric values using a stable sort mechanism. Sorting direction can be changed by clicking again.</p><p><span style=\" font-weight:600;\">Filter:</span> The filter menu can be opened via right click or by clicking on the filter icon.</p>";
        }
        description += "<p><span style=\" font-weight:600;\">Rearrange columns: </span>Drag-and-drop can be used for changing the order of columns</p>";
        description += "</body></html>";
        return description;
    }
    return QVariant();
}

int GdxSymbol::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (isDataTruncated()) {
        emit truncatedData(true);
        return GdxSymbol::MAX_DISPLAY_RECORDS;
    }
    else {
        emit truncatedData(false);
        return mFilterRecCount;
    }
}

int GdxSymbol::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (mType == GMS_DT_PAR || mType == GMS_DT_SET )
        return mDim + 1;
    else if (mType == GMS_DT_VAR || mType == GMS_DT_EQU)
        return mDim + 5;
    return 0;
}

QVariant GdxSymbol::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    else if (role == Qt::DisplayRole || role == Qt::EditRole) {
        int row = mRecSortIdx[mRecFilterIdx[index.row()]];
        if (index.column() < mDim)
            return mGdxSymbolTable->uel2Label(mKeys[row*mDim + index.column()]);
        else {
            double val = 0.0;
            if (mType <= GMS_DT_PAR) // Set, Parameter
                val = mValues[row];
            else // Variable, Equation
                val = mValues[row*GMS_DT_MAX + (index.column()-mDim)];
            if (mType == GMS_DT_SET)
                return mGdxSymbolTable->getElementText((int) val);
            if (role == Qt::EditRole)  // copy action
                return formatValue(val, true);
            return formatValue(val);
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        if (index.column() >= mDim) {
            if (mType == GMS_DT_PAR || mType == GMS_DT_VAR ||  mType == GMS_DT_EQU)
                return QVariant(Qt::AlignRight | Qt::AlignVCenter);
            else
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
    else if (role == Qt::BackgroundRole) {
        if (!mSearchRegEx.pattern().isEmpty() && mSearchRegEx.match(data(index).toString()).hasMatch())
            return toColor(Theme::Edit_matchesBg);
    }
    else if (role == Qt::ForegroundRole) {
        if (!mSearchRegEx.pattern().isEmpty() && mSearchRegEx.match(data(index).toString()).hasMatch())
            return QColor(Qt::white);
    }
    return QVariant();
}


bool GdxSymbol::loadData()
{
    mHasInvalidUel = false;
    QMutexLocker locker(mGdxMutex);
    mMinUel.resize(mDim);
    for(int i=0; i<mDim; i++)
        mMinUel[i] = INT_MAX;
    initNumericalBounds();
    mMaxUel.resize(mDim);
    for(int i=0; i<mDim; i++)
        mMaxUel[i] = INT_MIN;
    if(!mIsLoaded) {
        beginResetModel();
        endResetModel();

        if(mKeys.empty())
            mKeys.resize(mRecordCount*mDim);
        if(mValues.empty()) {
            if (mType == GMS_DT_PAR || mType == GMS_DT_SET)
                mValues.resize(mRecordCount);
            else  if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
                 mValues.resize(mRecordCount*GMS_DT_MAX);
        }

        int dummy;
        int keys[GMS_MAX_INDEX_DIM];
        double values[GMS_VAL_MAX];
        if (!gdxDataReadRawStart(mGdx, mNr, &dummy)) {
            char msg[GMS_SSSIZE];
            gdxErrorStr(mGdx, gdxGetLastError(mGdx), msg);
            EXCEPT() << "Problems reading GDX file: " << msg;
        }

        //skip records that have already been loaded
        for(int i=0; i<mLoadedRecCount; i++) {
            gdxDataReadRaw(mGdx, keys, values, &dummy);
            if(stopLoading) {
                stopLoading = false;
                gdxDataReadDone(mGdx);
                emit loadPaused();
                return false;
            }
        }

        int updateCount = 1000000;
        int triggerAutoResizeListViewCount = 1000;
        int keyOffset;
        int valOffset;
        int k;
        for(int i=mLoadedRecCount; i<mRecordCount; i++) {
            keyOffset = i*mDim;
            gdxDataReadRaw(mGdx, keys, values, &dummy);

            for(int j=0; j<mDim; j++) {
                k = keys[j];
                mKeys[keyOffset+j] = k;
                mMinUel[j] = qMin(mMinUel[j], k);
                mMaxUel[j] = qMax(mMaxUel[j], k);
            }
            if (mType == GMS_DT_PAR || mType == GMS_DT_SET)
                mValues[i] = values[0];
            else if (mType == GMS_DT_EQU || mType == GMS_DT_VAR) {
                valOffset = i*GMS_VAL_MAX;
                for(int vIdx=0; vIdx<GMS_VAL_MAX; vIdx++)
                    mValues[valOffset+vIdx] =  values[vIdx];
            }
            for(int vIdx=0; vIdx<mNumericalColumnCount; vIdx++) {
                if (values[vIdx] < GMS_SV_UNDEF) {
                    mMinDouble[vIdx] = qMin(mMinDouble[vIdx], values[vIdx]);
                    mMaxDouble[vIdx] = qMax(mMaxDouble[vIdx], values[vIdx]);
                }
            }
            mLoadedRecCount++;
            if (mLoadedRecCount == triggerAutoResizeListViewCount || mLoadedRecCount == mRecordCount) {
                beginResetModel();
                endResetModel();
                emit triggerListViewAutoResize();
            }
            mFilterRecCount = mLoadedRecCount;
            if(i%updateCount == 0) {
                beginResetModel();
                endResetModel();
            }
            if(stopLoading) {
                stopLoading = false;
                gdxDataReadDone(mGdx);
                emit loadPaused();
                return false;
            }
        }
        gdxDataReadDone(mGdx);

        beginResetModel();
        endResetModel();
        calcDefaultColumns();
        calcUelsInColumn();

        mIsLoaded = true;
        emit loadFinished();
    }
    return true;
}

void GdxSymbol::stopLoadingData()
{
    stopLoading = true;
}

void GdxSymbol::calcDefaultColumns()
{
    if(mType != GMS_DT_VAR && mType != GMS_DT_EQU)
        return; // symbols other than variable and equation do not have default values
    double defVal = 0.0;
    for(int valColIdx=0; valColIdx<GMS_VAL_MAX; valColIdx++) {
        if (mType == GMS_DT_VAR)
            defVal = gmsDefRecVar[mSubType][valColIdx];
        else if (mType == GMS_DT_EQU)
            defVal = gmsDefRecEqu[mSubType][valColIdx];
        for(int i=0; i<mRecordCount; i++) {
            if(defVal != mValues[i*GMS_VAL_MAX + valColIdx]) {
                mDefaultColumn[valColIdx] = false;
                break;
            }
            mDefaultColumn[valColIdx] = true;
        }
    }
}

void GdxSymbol::calcUelsInColumn()
{
    for(int dim=0; dim<mDim; dim++) {
        std::vector<int>* uels = new std::vector<int>();
        bool* sawUel = new bool[qMax(mMaxUel[dim]+1,1)] {false};
        int lastUel = -1;
        int currentUel = - 1;
        for(int rec=0; rec<mRecordCount; rec++) {
            currentUel = mKeys[rec*mDim + dim];
            if (currentUel < 0)
                mHasInvalidUel = true;
            else if(lastUel != currentUel) {
                lastUel = currentUel;
                if(!sawUel[currentUel]) {
                    sawUel[currentUel] = true;
                    uels->push_back(currentUel);
                }
            }
        }
        mUelsInColumn.push_back(uels);
        mShowUelInColumn.push_back(sawUel);
    }
}

GdxSymbolTableModel *GdxSymbol::gdxSymbolTable() const
{
    return mGdxSymbolTable;
}

void GdxSymbol::loadMetaData()
{
    char symName[GMS_UEL_IDENT_SIZE];
    char explText[GMS_SSSIZE];
    gdxSymbolInfo(mGdx, mNr, symName, &mDim, &mType);
    mName = mGdxSymbolTable->decoder().decode(symName);
    gdxSymbolInfoX (mGdx, mNr, &mRecordCount, &mSubType, explText);
    mExplText =  mGdxSymbolTable->decoder().decode(explText);
    if(mType == GMS_DT_EQU)
        mSubType = gmsFixEquType(mSubType);
    if(mType == GMS_DT_VAR)
        mSubType = gmsFixVarType(mSubType);

    if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
        mNumericalColumnCount = GMS_VAL_MAX;
    else if (mType == GMS_DT_PAR)
        mNumericalColumnCount = 1;
    else
        mNumericalColumnCount = 0;
}

void GdxSymbol::loadDomains()
{
    if (mNr == 0) //universe
        mDomains.append("*");
    else {
        gdxStrIndexPtrs_t domX;
        gdxStrIndex_t     domXXX;
        GDXSTRINDEXPTRS_INIT(domXXX,domX);
        gdxSymbolGetDomainX(mGdx, mNr, domX);
        for(int i=0; i<mDim; i++)
            mDomains.append(mGdxSymbolTable->decoder().decode(domX[i]));
    }
}

double GdxSymbol::specVal2SortVal(double val)
{
    if (val == GMS_SV_UNDEF)
        return mSpecValSortVal[GMS_SVIDX_UNDEF];
    else if (val == GMS_SV_NA)
        return  mSpecValSortVal[GMS_SVIDX_NA];
    else if (val == GMS_SV_PINF)
        return  mSpecValSortVal[GMS_SVIDX_PINF];
    else if (val == GMS_SV_MINF)
        return  mSpecValSortVal[GMS_SVIDX_MINF];
    else if (val == GMS_SV_EPS)
        return  mSpecValSortVal[GMS_SVIDX_EPS];
    else
        return val; // should be an acronym
}

QVariant GdxSymbol::formatValue(double val, bool dynamicDecSep) const
{
    if (val<GMS_SV_UNDEF) {
        if (dynamicDecSep)
            return numerics::DoubleFormatter::format(val, mNumericalFormat, mNumericalPrecision, mSqueezeTrailingZeroes, mDecSepCopy);
        return numerics::DoubleFormatter::format(val, mNumericalFormat, mNumericalPrecision, mSqueezeTrailingZeroes);
    }
    if (val == GMS_SV_UNDEF)
        return "UNDF";
    if (val == GMS_SV_NA)
        return "NA";
    if (val == GMS_SV_PINF)
        return "+INF";
    if (val == GMS_SV_MINF)
        return "-INF";
    if (val == GMS_SV_EPS)
        return "EPS";
    if (val>=GMS_SV_ACR) {
        char acr[GMS_SSSIZE];
        gdxAcronymName(mGdx, val, acr);
        return QString(acr);
    }
    return QVariant();
}

QRegularExpression GdxSymbol::searchRegEx() const
{
    return mSearchRegEx;
}

void GdxSymbol::setSearchRegEx(const QRegularExpression &searchRegEx)
{
    mSearchRegEx = searchRegEx;
}


void GdxSymbol::initNumericalBounds()
{
    if(mType == GMS_DT_PAR) {
        mMinDouble.resize(1);
        mMaxDouble.resize(1);
        mMinDouble[0] = INT_MAX;
        mMaxDouble[0] = INT_MIN;
    } else if (mType == GMS_DT_EQU || mType == GMS_DT_VAR) {
        mMinDouble.resize(GMS_VAL_MAX);
        mMaxDouble.resize(GMS_VAL_MAX);
        for (int i=0; i<GMS_VAL_MAX; i++) {
            mMinDouble[i] = INT_MAX;
            mMaxDouble[i] = INT_MIN;
        }
    }
}

bool GdxSymbol::isDataTruncated() const
{
    return mFilterRecCount > GdxSymbol::MAX_DISPLAY_RECORDS;
}

int GdxSymbol::numericalColumnCount() const
{
    return mNumericalColumnCount;
}

GdxSymbol *GdxSymbol::aliasedSymbol()
{
    if (mType == GMS_DT_ALIAS) {
        GdxSymbol *sym = mGdxSymbolTable->gdxSymbols().at(mSubType);
        return sym;
    } else
        return this;
}

void GdxSymbol::updateDecSepCopy()
{
    mDecSepCopy = '.';
    switch (Settings::settings()->toInt(SettingsKey::skGdxDecSepCopy)) {
    case DecimalSeparator::studio:
        break;
    case DecimalSeparator::system:
        mDecSepCopy = QString(QLocale::system().decimalPoint()).front();
        break;
    case DecimalSeparator::custom:
        if (!Settings::settings()->toString(skGdxCustomDecSepCopy).isEmpty())
            mDecSepCopy = Settings::settings()->toString(skGdxCustomDecSepCopy).front();
        break;
    default: break;
    }
}

bool GdxSymbol::hasInvalidUel() const
{
    return mHasInvalidUel;
}

QStringList GdxSymbol::domains() const
{
    return mDomains;
}

void GdxSymbol::setNumericalFormat(const numerics::DoubleFormatter::Format &numericalFormat)
{
    mNumericalFormat = numericalFormat;
}

int GdxSymbol::filterColumnCount()
{
    return mDim+mNumericalColumnCount;
}

void GdxSymbol::setNumericalPrecision(int numericalPrecision, bool squeezeTrailingZeroes)
{
    beginResetModel();
    mNumericalPrecision = numericalPrecision;
    mSqueezeTrailingZeroes = squeezeTrailingZeroes;
    endResetModel();
}

double GdxSymbol::minDouble(int valCol)
{
    return mMinDouble[valCol];
}

double GdxSymbol::maxDouble(int valCol)
{
    return mMaxDouble[valCol];
}

void GdxSymbol::registerColumnFilter(int column, ColumnFilter *columnFilter)
{
    mColumnFilters.at(column) = columnFilter;
    mFilterActive.at(column) = true;
}

void GdxSymbol::registerValueFilter(int valueColumn, ValueFilter *valueFilter)
{
    mValueFilters.at(valueColumn) = valueFilter;
    mFilterActive.at(mDim+valueColumn) = true;
}

void GdxSymbol::unregisterColumnFilter(int column)
{
    if (mColumnFilters[column] != nullptr) {
        mColumnFilters[column]->deleteLater();
        mColumnFilters[column] = nullptr;
    }
    mFilterActive.at(column) = false;
}

void GdxSymbol::unregisterValueFilter(int valueColumn)
{
    if (mValueFilters[valueColumn] != nullptr) {
        mValueFilters[valueColumn]->deleteLater();
        mValueFilters[valueColumn] = nullptr;
    }
    mFilterActive.at(mDim+valueColumn) = false;
}

void GdxSymbol::unregisterAllFilters()
{
    for(int i=0; i<mDim; i++)
        unregisterColumnFilter(i);
    for(int i=0; i<mNumericalColumnCount; i++)
        unregisterValueFilter(i);
}

ColumnFilter *GdxSymbol::columnFilter(int column)
{
    if (mColumnFilters[column] == nullptr)
        mColumnFilters[column] = new ColumnFilter(this, column);
    return mColumnFilters[column];
}

ValueFilter *GdxSymbol::valueFilter(int valueColumn)
{
    if (mValueFilters[valueColumn] == nullptr)
        mValueFilters[valueColumn] = new ValueFilter(this, valueColumn);
    return mValueFilters[valueColumn];
}

void GdxSymbol::setShowUelInColumn(const std::vector<bool *> &showUelInColumn)
{
    mShowUelInColumn = showUelInColumn;
}

bool GdxSymbol::filterActive(int column) const
{
    return mFilterActive.at(column);
}

void GdxSymbol::setFilterActive(int column, bool active)
{
    mFilterActive[column] = active;
}

std::vector<bool *> GdxSymbol::showUelInColumn() const
{
    return mShowUelInColumn;
}

std::vector<std::vector<int> *> GdxSymbol::uelsInColumn() const
{
    return mUelsInColumn;
}

void GdxSymbol::resetSortFilter()
{
    for(int i=0; i<mRecordCount; i++) {
        mRecSortIdx[i] = i;
        mRecFilterIdx[i] = i;
    }
    for(int dim=0; dim<mDim; dim++) {
        for(int uel : *mUelsInColumn.at(dim))
            mShowUelInColumn.at(dim)[uel] = true;
    }
    unregisterAllFilters();
    mFilterRecCount = mLoadedRecCount;
    emit layoutChanged();
}

bool GdxSymbol::isAllDefault(int valColIdx)
{
    if(mType == GMS_DT_VAR || mType == GMS_DT_EQU)
        return mDefaultColumn[valColIdx];
    else
        return false;
}

int GdxSymbol::subType() const
{
    return mSubType;
}

/*
 * Custom sorting algorithm that sorts by column using a stable sorting algorithm (std::stable_sort)
 *
 * mRecSortIdx maps a row index in the view to a row index in the data. This way the sorting is implemented
 * without actually changing the order of the data itself but storing a mapping of row indexes
 *
 * mLabelCompIdx is used to map a UEL (int) to a specific number (int) which refelects the lexicographical
 * order of label. This allows for better sorting performance since the compare functions only need to compare int
 * instead of QString
 */
void GdxSymbol::sort(int column, Qt::SortOrder order)
{
    if (column < 0 || mHasInvalidUel) return;
    // sort by key column
    if(column < mDim) {
        std::vector<int> labelCompIdx = mGdxSymbolTable->labelCompIdx();
        QList<QPair<int, int>> l;
        uint uel;
        for(int rec=0; rec<mRecordCount; rec++) {
            uel = mKeys[mRecSortIdx[rec]*mDim + column];
            // bad uels are sorted by their internal number separately from normal UELS
            if (uel >= labelCompIdx.size())
                l.append(QPair<int, int>(uel, mRecSortIdx[rec]));
            else
                l.append(QPair<int, int>(labelCompIdx[uel], mRecSortIdx[rec]));
        }
        if(order == Qt::SortOrder::AscendingOrder)
            std::stable_sort(l.begin(), l.end(), [](QPair<int, int> a, QPair<int, int> b) { return a.first < b.first; });
        else
            std::stable_sort(l.begin(), l.end(), [](QPair<int, int> a, QPair<int, int> b) { return a.first > b.first; });

        for(int rec=0; rec< mRecordCount; rec++)
            mRecSortIdx[rec] = l.at(rec).second;
    }

    //sort set and alias by explanatory text
    else if (mType == GMS_DT_SET || mType == GMS_DT_ALIAS) {
        QList<QPair<QString, int>> l;
        for(int rec=0; rec<mRecordCount; rec++)
            l.append(QPair<QString, int>(mGdxSymbolTable->getElementText(mValues[mRecSortIdx[rec]]), mRecSortIdx[rec]));

        if (order == Qt::SortOrder::AscendingOrder)
            std::stable_sort(l.begin(), l.end(), [](const QPair<QString, int> &a, const QPair<QString, int> &b) { return a.first < b.first; });
        else
            std::stable_sort(l.begin(), l.end(), [](const QPair<QString, int> &a, const QPair<QString, int> &b) { return a.first > b.first; });

        for(int rec=0; rec< mRecordCount; rec++)
            mRecSortIdx[rec] = l.at(rec).second;
    }
    // sort parameter, variable and equation by value columns
    else {
        QList<QPair<double, int>> l;
        double val=0;
        if (mType == GMS_DT_PAR) {
            for(int rec=0; rec<mRecordCount; rec++) {
                val = mValues[mRecSortIdx[rec]];
                if (val>=GMS_SV_UNDEF)
                    val = specVal2SortVal(val);
                l.append(QPair<double, int>(val, mRecSortIdx[rec]));
            }
        }
        else if (mType == GMS_DT_VAR || mType == GMS_DT_EQU) {
            for(int rec=0; rec<mRecordCount; rec++) {
                val = mValues[mRecSortIdx[rec]*GMS_VAL_MAX + (column-mDim)];
                if (val>=GMS_SV_UNDEF)
                    val = specVal2SortVal(val);
                l.append(QPair<double, int>(val, mRecSortIdx[rec]));
            }
        }

        if (order == Qt::SortOrder::AscendingOrder)
            std::stable_sort(l.begin(), l.end(), [](QPair<double, int> a, QPair<double, int> b) { return a.first < b.first; });
        else
            std::stable_sort(l.begin(), l.end(), [](QPair<double, int> a, QPair<double, int> b) { return a.first > b.first; });

        for (int rec=0; rec< mRecordCount; rec++)
            mRecSortIdx[rec] = l.at(rec).second;
    }
    emit layoutChanged();
    filterRows();
}

void GdxSymbol::filterRows()
{
    if (mHasInvalidUel) return;
    beginResetModel();
    for (int i=0; i<mRecordCount; i++)
        mRecFilterIdx[i] = i;

    int removedCount = 0;

    mFilterRecCount = mLoadedRecCount;
    for(int row=0; row<mRecordCount; row++) {
        int recIdx = mRecSortIdx[row];
        mRecFilterIdx[row-removedCount] = row;
        for(int dim=0; dim<filterColumnCount(); dim++) {
            if (dim<mDim) { // filter by key column
                if(!mShowUelInColumn.at(dim)[mKeys[recIdx*mDim + dim]]) { //filter record
                    mFilterRecCount--;
                    removedCount++;
                    break;
                }
            } else { // filter by numerical column
                bool alreadyRemoved=false;
                for(int i=0; i<mNumericalColumnCount; i++) {
                    if (mFilterActive[mDim+i]) {
                        double val = mValues[recIdx*mNumericalColumnCount+i];
                        ValueFilter* vf = mValueFilters[i];
                        if (val < GMS_SV_UNDEF) {
                            if ( (!vf->exclude() && (val <  vf->currentMin() || val >  vf->currentMax())) ||
                                 ( vf->exclude() && (val >= vf->currentMin() && val <= vf->currentMax())) ) {
                                mFilterRecCount--;
                                removedCount++;
                                alreadyRemoved=true;
                                break;
                            }
                        } else if (    (!vf->showUndef()   && val == GMS_SV_UNDEF)
                                    || (!vf->showNA()      && val == GMS_SV_NA   )
                                    || (!vf->showPInf()    && val == GMS_SV_PINF )
                                    || (!vf->showMInf()    && val == GMS_SV_MINF )
                                    || (!vf->showEps()     && val == GMS_SV_EPS  )
                                    || (!vf->showAcronym() && val >= GMS_SV_ACR  ) ) {
                            mFilterRecCount--;
                            removedCount++;
                            alreadyRemoved=true;
                            break;
                        }
                    }
                }
                if (alreadyRemoved)
                    break;
            }
        }
    }
    endResetModel();
    filterChanged();
}

bool GdxSymbol::isLoaded() const
{
    return mIsLoaded;
}

QString GdxSymbol::explText() const
{
    return mExplText;
}

int GdxSymbol::recordCount() const
{
    return mRecordCount;
}

int GdxSymbol::type() const
{
    return mType;
}

int GdxSymbol::dim() const
{
    return mDim;
}

QString GdxSymbol::name() const
{
    return mName;
}

int GdxSymbol::nr() const
{
    return mNr;
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
