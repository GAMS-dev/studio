/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "tableviewmodel.h"

#include "theme.h"

namespace gams {
namespace studio {
namespace gdxviewer {

TableViewModel::TableViewModel(GdxSymbol* sym, GdxSymbolTableModel* gdxSymbolTable, QObject *parent)
    : QAbstractTableModel(parent), mSym(sym), mGdxSymbolTable(gdxSymbolTable)
{
    mTvColDim = (mSym->mDim > 1 ? 1 : 0) + (isVarEqu() ? 1 : 0);
    mTvDimOrder.reserve(tvDimExt());
    for (int i = 0; i < tvDimExt(); ++i)
        mTvDimOrder << i;
}

bool TableViewModel::isVarEqu() const
{
    return mSym->mType == GMS_DT_VAR || mSym->mType == GMS_DT_EQU;
}

int TableViewModel::tvDimExt() const
{
    return mSym->mDim + (isVarEqu() ? 1 : 0);
}

TableViewModel::~TableViewModel()
{

}

QString TableViewModel::attrLabel(uint valIdx)
{
    switch(valIdx) {
    case GMS_VAL_LEVEL: return "Level";
    case GMS_VAL_MARGINAL: return "Marginal";
    case GMS_VAL_LOWER: return "Lower";
    case GMS_VAL_UPPER: return "Upper";
    case GMS_VAL_SCALE: return "Scale";
    }
    return QString();
}

QVariant TableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        QStringList header;
        int ed = tvDimExt();
        if (orientation == Qt::Horizontal) {
            if (mNeedDummyColumn) {
                if (mNeedDummyRow) {
                    if (mSym->isLoaded())
                        return "No Data";
                    else
                        return "Loading Data";
                }
                if (mSym->type() == GMS_DT_EQU || mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_PAR)
                    header << "Value";
                else
                    header << "Text";
            }
            else {
                for (int i=0; i<mTvColDim; i++) {
                    uint v = mTvColHeaders[section][i];
                    if (isVarEqu() && mTvDimOrder[ed-mTvColDim+i] == mSym->mDim)
                        header << attrLabel(v);
                    else
                        header << mGdxSymbolTable->uel2Label(int(v));
                }
            }
        }
        else {
            if (mNeedDummyRow) {
                if (mSym->type() == GMS_DT_EQU || mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_PAR)
                    header << "Value";
                else
                    header << "Text";
            }
            else {
                for (int i=0; i<ed-mTvColDim; i++) {
                    uint v = mTvRowHeaders[section][i];
                    if (isVarEqu() && mTvDimOrder[i] == mSym->mDim)
                        header << attrLabel(v);
                    else
                        header << mGdxSymbolTable->uel2Label(int(v));
                }
            }
        }
        return header;
    }
    return QVariant();
}

int TableViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (mNeedDummyRow) {
        if (mNeedDummyColumn) //table is empty
            return 0;
        return 1;
    }
    return mTvRowHeaders.size();
}

int TableViewModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (mNeedDummyColumn)
        return 1;
    return mTvColHeaders.size();
}

QVariant TableViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        QList<uint> keys;
        if (mNeedDummyRow)
            keys = mTvColHeaders[index.column()];
        else if (mNeedDummyColumn)
            keys = mTvRowHeaders[index.row()];
        else
            keys = mTvRowHeaders[index.row()] + mTvColHeaders[index.column()];
        if (mTvKeysToValIdx.contains(keys)) {
            double val = mSym->mValues[size_t(mTvKeysToValIdx[keys])];
            if (mSym->mType == GMS_DT_SET) {
                if (role == Qt::UserRole)
                    return GMS_SV_UNDEF;
                return mGdxSymbolTable->getElementText(int(val));
            }
            if (role == Qt::EditRole)  // copy action
                return mSym->formatValue(val, true);
            if (role == Qt::UserRole) { // raw value
                if (mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_EQU) {
                    int mask = 1 << mTvColHeaders[index.column()].last();
                    if ((mask & mHeatedAttributes) == 0)
                        return GMS_SV_UNDEF;
                }
                return val;
            }
            return mSym->formatValue(val);
        }
        return QVariant();
    }
    else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    }
    else if (role == Qt::BackgroundRole) {
        if (!mSym->searchRegEx().pattern().isEmpty() && mSym->searchRegEx().match(data(index).toString()).hasMatch())
            return toColor(Theme::Edit_findBg);
    }
    else if (role == Qt::ForegroundRole) {
        if (!mSym->searchRegEx().pattern().isEmpty() && mSym->searchRegEx().match(data(index).toString()).hasMatch())
            return QColor(Qt::white);
    }
    return QVariant();
}


void TableViewModel::calcDefaultSectionsTableView()
{
    mDefaultSectionsTableView.clear();
    if (!isVarEqu())
        return; // symbols other than variable and equation do not have default values

    bool attrInCol = attrOrientation() == Qt::Horizontal;
    int sectionCount = attrInCol ? columnCount() : rowCount();
    int otherCount = attrInCol ? rowCount() : columnCount();
    mDefaultSectionsTableView.resize(sectionCount);

    for (int s=0; s<sectionCount; s++) {
        int valIdx = attrValueForSection(s, attrOrientation());
        if (valIdx < 0) {
            mDefaultSectionsTableView[s] = false;
            continue;
        }
        double defVal = mSym->mType == GMS_DT_VAR ? gmsDefRecVar[mSym->mSubType][valIdx]
                                                    : gmsDefRecEqu[mSym->mSubType][valIdx];
        bool allDefault = true;
        for (int o=0; o<otherCount; o++) {
            QList<uint> keys;
            if (attrInCol)
                keys = mNeedDummyRow ? mTvColHeaders[s] : mTvRowHeaders[o] + mTvColHeaders[s];
            else
                keys = mNeedDummyColumn ? mTvRowHeaders[s] : mTvRowHeaders[s] + mTvColHeaders[o];
            double val = defVal;
            if (mTvKeysToValIdx.contains(keys))
                val = mSym->mValues[size_t(mTvKeysToValIdx[keys])];

            // We really need (defVal != val) here - but that leads to compiler-warning
            if (defVal < val || defVal > val) {
                allDefault = false;
                break;
            }
        }
        mDefaultSectionsTableView[s] = allDefault;
    }
}

void TableViewModel::calcLabelsInRows()
{
    QList<QSet<uint>> uelsInRows;

    if (mNeedDummyRow) {
        mlabelsInRows.clear();
        mlabelsInRows.resize(1);
        mlabelsInRows[0].append(this->headerData(0, Qt::Vertical).toString());
        return;
    }
    int rowDim = mTvRowHeaders[0].size();
    uelsInRows.resize(rowDim);

    mlabelsInRows.clear();
    mlabelsInRows.resize(rowDim);

    for (int r=0; r<mTvRowHeaders.size(); r++) {
        for(int c=0; c<rowDim; c++)
            uelsInRows[c].insert(mTvRowHeaders[r][c]);
    }
    for (int c=0; c<uelsInRows.size(); c++) {
        bool isAttr = attrLocalIndex(Qt::Vertical) == c;
        for(const uint &uel : std::as_const(uelsInRows[c]))
            mlabelsInRows[c].append(isAttr ? attrLabel(uel) : mGdxSymbolTable->uel2Label(int(uel)));
    }
}

QList<QList<QString> > TableViewModel::labelsInRows() const
{
    return mlabelsInRows;
}

QStringList TableViewModel::domains() const
{
    return mSym->domains();
}

void TableViewModel::scrollHTriggered()
{
    emit headerDataChanged(Qt::Horizontal, 0, 2);
}

void TableViewModel::scrollVTriggered()
{
    emit headerDataChanged(Qt::Vertical, 0, 2);
}

void TableViewModel::initTableView(int nrColDim, QList<int> dimOrder)
{
    int ed = tvDimExt();

    // fall back to the default pivot for a missing or stale (e.g. pre-existing persisted) order
    if (dimOrder.size() != ed) {
        dimOrder.clear();
        for (int i=0; i<ed; i++)
            dimOrder << i;
        nrColDim = (mSym->mDim > 1 ? 1 : 0) + (isVarEqu() ? 1 : 0);
    }
    nrColDim = qBound(0, nrColDim, ed);

    mTvColDim = nrColDim;
    mTvDimOrder = dimOrder;
    QSet<QList<uint>> seenColHeaders;
    QSet<QList<uint>> seenRowHeaders;

    mTvRowHeaders.clear();
    mTvColHeaders.clear();
    mTvKeysToValIdx.clear();
    QList<uint> lastRowHeader;
    QList<uint> lastColHeader;
    size_t r;

    QList<uint> fullKey(ed);
    QList<uint> rowHeader;
    QList<uint> colHeader;
    size_t keyIdx;
    int rowDim = ed - mTvColDim;

    // position of the attribute's slot within rowHeader/colHeader, so the valIdx loop below
    // can patch that one entry in place instead of rebuilding both tuples from scratch
    int attrRowIdx = -1, attrColIdx = -1;
    if (isVarEqu()) {
        int pos = mTvDimOrder.indexOf(mSym->mDim);
        if (pos < rowDim)
            attrRowIdx = pos;
        else
            attrColIdx = pos - rowDim;
    }

    for (int rec=0; rec<mSym->mFilterRecCount; rec++) {
        r = mSym->mRecSortIdx[size_t(mSym->mRecFilterIdx[size_t(rec)])];
        keyIdx = r*mSym->mDim;

        for (int i=0; i<mSym->mDim; i++)
            fullKey[i] = mSym->mKeys[keyIdx+i];

        rowHeader.resize(0);
        colHeader.resize(0);
        for(int i=0; i<rowDim; i++)
            rowHeader.push_back(mTvDimOrder[i] == mSym->mDim ? 0 : fullKey[mTvDimOrder[i]]);
        for(int i=rowDim; i<ed; i++)
            colHeader.push_back(mTvDimOrder[i] == mSym->mDim ? 0 : fullKey[mTvDimOrder[i]]);

        if (isVarEqu()) {
            for (int valIdx=0; valIdx<GMS_VAL_MAX; valIdx++) {
                if (attrRowIdx != -1)
                    rowHeader[attrRowIdx] = uint(valIdx);
                else
                    colHeader[attrColIdx] = uint(valIdx);

                QList<uint> keys = rowHeader + colHeader;
                mTvKeysToValIdx[keys] = int(r)*GMS_VAL_MAX + valIdx;

                if (rowHeader != lastRowHeader) {
                    if (!seenRowHeaders.contains(rowHeader)) {
                        seenRowHeaders.insert(rowHeader);
                        mTvRowHeaders.push_back(rowHeader);
                    }
                    lastRowHeader = rowHeader;
                }
                if (colHeader != lastColHeader) {
                    if (!seenColHeaders.contains(colHeader)) {
                        seenColHeaders.insert(colHeader);
                        mTvColHeaders.push_back(colHeader);
                    }
                    lastColHeader = colHeader;
                }
            }
        } else {
            QList<uint> keys = rowHeader + colHeader;
            mTvKeysToValIdx[keys] = int(r);

            if (rowHeader != lastRowHeader) {
                if (!seenRowHeaders.contains(rowHeader)) {
                    seenRowHeaders.insert(rowHeader);
                    mTvRowHeaders.push_back(rowHeader);
                    lastRowHeader = rowHeader;
                }
            }
            if (colHeader != lastColHeader) {
                if (!seenColHeaders.contains(colHeader)) {
                    seenColHeaders.insert(colHeader);
                    mTvColHeaders.push_back(colHeader);
                    lastColHeader = colHeader;
                }
            }
        }
    }
    //sort the headers
    std::stable_sort(mTvRowHeaders.begin(), mTvRowHeaders.end());
    std::stable_sort(mTvColHeaders.begin(), mTvColHeaders.end());

    mNeedDummyRow = mTvRowHeaders.isEmpty();
    mNeedDummyColumn = mTvColHeaders.isEmpty();

    calcDefaultSectionsTableView();
    calcLabelsInRows();
}

void TableViewModel::setHeatedAttributes(int heatedAttribute)
{
    if (mHeatedAttributes != heatedAttribute) {
        int oldAttrib = mHeatedAttributes;
        mHeatedAttributes = heatedAttribute;
        emit headerDataChanged(Qt::Horizontal, qMin(oldAttrib, heatedAttribute), qMax(oldAttrib, heatedAttribute));
    }
}

GdxSymbol *TableViewModel::sym() const
{
    return mSym;
}

bool TableViewModel::needDummyColumn() const
{
    return mNeedDummyColumn;
}

bool TableViewModel::needDummyRow() const
{
    return  mNeedDummyRow;
}

int TableViewModel::dim()
{
    return mSym->mDim;
}

QList<int> TableViewModel::tvDimOrder() const
{
    return mTvDimOrder;
}

int TableViewModel::attrPos() const
{
    if (!isVarEqu())
        return -1;
    return mTvDimOrder.indexOf(mSym->mDim);
}

int TableViewModel::attrLocalIndex(Qt::Orientation orientation) const
{
    int pos = attrPos();
    if (pos == -1)
        return -1;
    if (attrOrientation() != orientation)
        return -1;
    if (orientation == Qt::Horizontal)
        return pos - (tvDimExt()-mTvColDim);
    return pos;
}

Qt::Orientation TableViewModel::attrOrientation() const
{
    int pos = attrPos();
    if (pos == -1)
        return Qt::Horizontal;
    return pos >= tvDimExt()-mTvColDim ? Qt::Horizontal : Qt::Vertical;
}

int TableViewModel::rowColDivider() const
{
    int ed = tvDimExt();
    int rowDim = ed - mTvColDim;
    if (rowDim <= 0 || rowDim >= ed)
        return -1;

    int stripPos = 0;
    for (int i=0; i<rowDim; i++)
        stripPos += mTvDimOrder[i] == mSym->mDim ? GMS_VAL_MAX : 1;
    return stripPos - 1;
}

int TableViewModel::attrValueForSection(int section, Qt::Orientation orientation) const
{
    if (!isVarEqu())
        return -1;
    int ed = tvDimExt();
    if (orientation == Qt::Horizontal) {
        if (section < 0 || section >= mTvColHeaders.size())
            return -1;
        for (int i=0; i<mTvColDim; i++)
            if (mTvDimOrder[ed-mTvColDim+i] == mSym->mDim)
                return int(mTvColHeaders[section][i]);
    } else {
        if (section < 0 || section >= mTvRowHeaders.size())
            return -1;
        int rowDim = ed-mTvColDim;
        for (int i=0; i<rowDim; i++)
            if (mTvDimOrder[i] == mSym->mDim)
                return int(mTvRowHeaders[section][i]);
    }
    return -1;
}

bool TableViewModel::isAllDefault(int section, Qt::Orientation orientation) const
{
    if (!isVarEqu() || attrOrientation() != orientation)
        return false;
    return mDefaultSectionsTableView.value(section, false);
}

int TableViewModel::tvColDim() const
{
    return mTvColDim;
}

int TableViewModel::type()
{
    return mSym->type();
}

void TableViewModel::setTableView(int colDim, const QList<int> &tvDims)
{
    beginResetModel();
    if (colDim!=-1) {
        mTvColDim = colDim;
        mTvDimOrder = tvDims;
    }
    initTableView(mTvColDim, mTvDimOrder);
    endResetModel();
    emit initFinished();
}

void TableViewModel::setTableViewNoArgs()
{
    if (mSym->isLoaded())
        setTableView();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
