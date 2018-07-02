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
#include "gdxsymbol.h"
#include "exception.h"
#include "gdxsymboltable.h"

#include <QMutex>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbol::GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, int nr, GdxSymbolTable* gdxSymbolTable, QObject *parent)
    : QAbstractTableModel(parent), mGdx(gdx), mNr(nr), mGdxMutex(gdxMutex), mGdxSymbolTable(gdxSymbolTable)
{
    loadMetaData();
    loadDomains();

    mRecSortIdx.resize(mRecordCount);
    for(int i=0; i<mRecordCount; i++)
        mRecSortIdx[i] = i;

    mRecFilterIdx.resize(mRecordCount);
    for(int i=0; i<mRecordCount; i++)
        mRecFilterIdx[i] = i;

    mFilterActive.resize(mDim);
    for(int i=0; i<mDim; i++)
        mFilterActive[i] = false;

    mSpecValSortVal.push_back(5.0E300); // GMS_SV_UNDEF
    mSpecValSortVal.push_back(4.0E300); // GMS_SV_NA
    mSpecValSortVal.push_back(GMS_SV_PINF); // GMS_SV_PINF
    mSpecValSortVal.push_back(-std::numeric_limits<double>::max()); // GMS_SV_MINF
    mSpecValSortVal.push_back(4.94066E-324); // GMS_SV_EPS
    mSpecValSortVal.push_back(0);  //TODO: Acronyms
}

GdxSymbol::~GdxSymbol()
{

    for(auto v : mUelsInColumn)
        delete v;
    for(auto a: mShowUelInColumn) {
        if(a)
            delete[] a;
    }
}

QVariant GdxSymbol::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section < mDim)
                return mDomains.at(section);
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
                description += "<p><span style=\" font-weight:600;\">Sort: </span>Left click sorts the numeric values using a stable sort mechanism. Sorting direction can be changed by clicking again.</p>";
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
    return mFilterRecCount;
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

    else if (role == Qt::DisplayRole) {
        int row = mRecSortIdx[mRecFilterIdx[index.row()]];
        if (index.column() < mDim)
            return mGdxSymbolTable->uel2Label(mKeys[row*mDim + index.column()]);
        else {
            double val = 0.0;
            if (mType == GMS_DT_PAR)
                val = mValues[row];
            else if (mType == GMS_DT_SET) {
                val = mValues[row];
                return mGdxSymbolTable->getElementText((int) val);
            }
            else if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
                val = mValues[row*GMS_DT_MAX + (index.column()-mDim)];
            //apply special values:
            if (val<GMS_SV_UNDEF)
                return QString::number(val, 'g', 15);
            else {
                if (val == GMS_SV_UNDEF)
                    return "UNDEF";
                if (val == GMS_SV_NA)
                    return "NA";
                if (val == GMS_SV_PINF)
                    return "+INF";
                if (val == GMS_SV_MINF)
                    return "-INF";
                if (val == GMS_SV_EPS)
                    return "EPS";
                else if (val>=GMS_SV_ACR) {
                    char acr[GMS_SSSIZE];
                    gdxAcronymName(mGdx, val, acr);
                    return QString(acr);
                }
            }
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
    return QVariant();
}


void GdxSymbol::loadData()
{
    QMutexLocker locker(mGdxMutex);
    mMinUel.resize(mDim);
    for(int i=0; i<mDim; i++)
        mMinUel[i] = INT_MAX;
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
        int* keys = new int[GMS_MAX_INDEX_DIM];
        double* values = new double[GMS_VAL_MAX];
        if (!gdxDataReadRawStart(mGdx, mNr, &dummy)) {
            char msg[GMS_SSSIZE];
            gdxErrorStr(mGdx, gdxGetLastError(mGdx), msg);
            EXCEPT() << "Problems reading GDX file: " << msg;
        }

        //skip records that have already been loaded
        for(int i=0; i<mLoadedRecCount; i++) {
            gdxDataReadRaw(mGdx, keys, values, &dummy);
            //TODO(CW): redundant code (see below)
            if(stopLoading) {
                stopLoading = false;
                gdxDataReadDone(mGdx);
                delete[] keys;
                delete[] values;
                return;
            }
        }

        int updateCount = 1000000;
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
            mLoadedRecCount++;
            mFilterRecCount = mLoadedRecCount;
            if(i%updateCount == 0) {
                beginResetModel();
                endResetModel();
            }
            if (stopLoading) {
                stopLoading = false;
                gdxDataReadDone(mGdx);
                delete[] keys;
                delete[] values;
                return;
            }
        }
        gdxDataReadDone(mGdx);

        beginResetModel();
        endResetModel();
        calcDefaultColumns();
        calcUelsInColumn();

        mIsLoaded = true;

        delete[] keys;
        delete[] values;

        emit loadFinished();
    }
}

void GdxSymbol::stopLoadingData()
{
    stopLoading = true;
}

void GdxSymbol::calcDefaultColumns()
{
    if(mType != GMS_DT_VAR && mType != GMS_DT_EQU)
        return; // symbols other than variable and equation do not have default values
    double defVal;
    for(int valColIdx=0; valColIdx<GMS_VAL_MAX; valColIdx++) {
        if (mType == GMS_DT_VAR)
            defVal = gmsDefRecVar[mSubType][valColIdx];
        else if (mType == GMS_DT_EQU)
            defVal = gmsDefRecEqu[mSubType][valColIdx];
        for(int i=0; i<mRecordCount; i++) {
            // TODO(AF) fix uninizalized defVal
            if(defVal != mValues[i*GMS_VAL_MAX + valColIdx]) {
                mDefaultColumn[valColIdx] = false;
                break;
            }
            mDefaultColumn[valColIdx] = true;
        }
    }
}

//TODO(CW): refactoring for better performance
void GdxSymbol::calcUelsInColumn()
{
    for(int dim=0; dim<mDim; dim++) {
        std::vector<int>* uels = new std::vector<int>();
        bool* sawUel = new bool[qMax(mMaxUel[dim]+1,1)] {false}; //TODO(CW): squeeze using mMinUel

        int lastUel = -1;
        int currentUel = - 1;
        for(int rec=0; rec<mRecordCount; rec++) {
            currentUel = mKeys[rec*mDim + dim];
            if(lastUel != currentUel) {
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

GdxSymbolTable *GdxSymbol::gdxSymbolTable() const
{
    return mGdxSymbolTable;
}

void GdxSymbol::loadMetaData()
{
    char symName[GMS_UEL_IDENT_SIZE];
    char explText[GMS_SSSIZE];
    gdxSymbolInfo(mGdx, mNr, symName, &mDim, &mType);
    mName = symName;
    gdxSymbolInfoX (mGdx, mNr, &mRecordCount, &mSubType, explText);
    mExplText = explText;
    if(mType == GMS_DT_EQU)
        mSubType = gmsFixEquType(mSubType);
    if(mType == GMS_DT_VAR)
        mSubType = gmsFixVarType(mSubType);
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
            mDomains.append(domX[i]);
    }
}

double GdxSymbol::specVal2SortVal(double val)
{
    if (val == GMS_SV_UNDEF)
        return mSpecValSortVal[GMS_SVIDX_UNDEF];
    else if (val == GMS_SV_NA)
        return  mSpecValSortVal[GMS_SVIDX_NA];
    else if (val == GMS_SV_MINF)
        return  mSpecValSortVal[GMS_SVIDX_MINF];
    else if (val == GMS_SV_EPS)
        return  mSpecValSortVal[GMS_SVIDX_EPS];
    else
        return val;
}

std::vector<bool> GdxSymbol::filterActive() const
{
    return mFilterActive;
}

void GdxSymbol::setFilterActive(const std::vector<bool> &filterActive)
{
    mFilterActive = filterActive;
}

void GdxSymbol::setShowUelInColumn(const std::vector<bool *> &showUelInColumn)
{
    mShowUelInColumn = showUelInColumn;
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
        mFilterActive[dim] = false;
        for(int uel : *mUelsInColumn.at(dim))
            mShowUelInColumn.at(dim)[uel] = true;
    }
    mFilterRecCount = mLoadedRecCount; //TODO(CW): use mRecordCount ?
    layoutChanged();
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
    //TODO(CW): This is a workaround for not sorting if the selcted symbol is updated and column and order haven't changed
    //if(column == mSortColumn && order == mSortOrder)
    //    return;

    // sort by key column
    if(column<mDim) {
        std::vector<int> labelCompIdx = mGdxSymbolTable->labelCompIdx();
        QList<QPair<int, int>> l;
        int uel = -1;
        for(int rec=0; rec<mRecordCount; rec++) {
            uel = mKeys[mRecSortIdx[rec]*mDim + column];
            // TODO (AF) fix comparision warning
            if (uel >= labelCompIdx.size())  //TODO: workaround for bad UELS. Bad uels are sorted by their internal number separately from normal UELS
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

    //TODO(CW): make string pool sorting index like for uels for increasing sort speed on explanatory text
    //sort set and alias by explanatory text
    else if (mType == GMS_DT_SET || mType == GMS_DT_ALIAS) {
        QList<QPair<QString, int>> l;
        for(int rec=0; rec<mRecordCount; rec++)
            l.append(QPair<QString, int>(mGdxSymbolTable->getElementText(mValues[mRecSortIdx[rec]]), mRecSortIdx[rec]));

        if (order == Qt::SortOrder::AscendingOrder)
            std::stable_sort(l.begin(), l.end(), [](QPair<QString, int> a, QPair<QString, int> b) { return a.first < b.first; });
        else
            std::stable_sort(l.begin(), l.end(), [](QPair<QString, int> a, QPair<QString, int> b) { return a.first > b.first; });

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
    layoutChanged();
    filterRows();
}

void GdxSymbol::filterRows()
{
    for (int i=0; i<mRecordCount; i++)
        mRecFilterIdx[i] = i;

    int removedCount = 0;

    mFilterRecCount = mLoadedRecCount;
    for(int row=0; row<mRecordCount; row++) {
        int recIdx = mRecSortIdx[row];
        mRecFilterIdx[row-removedCount] = row;
        for(int dim=0; dim<mDim; dim++) {
            if(!mShowUelInColumn.at(dim)[mKeys[recIdx*mDim + dim]]) { //filter record
                mFilterRecCount--;
                removedCount++;
                break;
            }
        }
    }
    beginResetModel();
    endResetModel();
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
