#include "tableviewmodel.h"

#include <QDebug>

namespace gams {
namespace studio {
namespace gdxviewer {

TableViewModel::TableViewModel(GdxSymbol* sym, GdxSymbolTable* gdxSymbolTable, QObject *parent)
    : QAbstractTableModel(parent), mSym(sym), mGdxSymbolTable(gdxSymbolTable)
{
    mTvColDim = 1;
    for(int i=0; i<mSym->mDim; i++)
        mTvDimOrder << i;

    tvLabelWidth = new QMap<QString, int>();
    tvSectionWidth = new QVector<int>();
}

TableViewModel::~TableViewModel()
{
    if (tvSectionWidth)
        delete tvSectionWidth;
    if (tvLabelWidth)
        delete tvLabelWidth;
}

QVariant TableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {        
        QStringList header;
        if (orientation == Qt::Horizontal) {
            if (mNeedDummyColumn)
                header << " ";
            else if (mSym->mType == GMS_DT_VAR || mSym->mType == GMS_DT_EQU) {
                for (int i=0; i<mTvColHeaders[section].size()-1; i++ ) {
                    uint uel = mTvColHeaders[section][i];
                    header << mGdxSymbolTable->uel2Label(uel);
                }
                switch(mTvColHeaders[section].last()) {
                case GMS_VAL_LEVEL: header << "Level"; break;
                case GMS_VAL_MARGINAL: header << "Marginal"; break;
                case GMS_VAL_LOWER: header << "Lower"; break;
                case GMS_VAL_UPPER: header << "Upper"; break;
                case GMS_VAL_SCALE: header << "Scale"; break;
                }
            }
            else {
                for (uint uel: mTvColHeaders[section])
                    header << mGdxSymbolTable->uel2Label(uel);
            }
        }
        else {
            if (mNeedDummyRow)
                header << " ";
            else {
                for (uint uel: mTvRowHeaders[section])
                    header << mGdxSymbolTable->uel2Label(uel);
            }
        }
        return header;
    }
    else if (role == Qt::SizeHintRole && orientation == Qt::Vertical) {
        if (mNeedDummyRow)
            return 10;
        int totalWidth = 0;
        for (int i=0; i<mSym->mDim-mTvColDim; i++) {
            int width;
            QString label = mGdxSymbolTable->uel2Label(mTvRowHeaders[section][i]);
            if (tvLabelWidth->contains(label))
                width = tvLabelWidth->value(label);
            else {
                QVariant var = headerData(section, orientation, Qt::FontRole);
                QFont fnt;
                if (var.isValid() && var.canConvert<QFont>())
                    fnt = qvariant_cast<QFont>(var);
                fnt.setBold(true);
                width = QFontMetrics(fnt).width(label)*1.3;
                tvLabelWidth->insert(label, width);
            }
            totalWidth += width;
            tvSectionWidth->replace(i, qMax(tvSectionWidth->at(i), width));
        }
        return totalWidth;
    }
    return QVariant();
}

int TableViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (mNeedDummyRow)
        return 1;
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

    else if (role == Qt::DisplayRole) {
        QVector<uint> keys;
        if (mNeedDummyRow)
            keys = mTvColHeaders[index.column()];
        else if (mNeedDummyColumn)
            keys = mTvRowHeaders[index.row()];
        else
            keys = mTvRowHeaders[index.row()] + mTvColHeaders[index.column()];
        if (mTvKeysToValIdx.contains(keys)) {
            double val = mSym->mValues[mTvKeysToValIdx[keys]];
            if (mSym->mType == GMS_DT_SET)
                return mGdxSymbolTable->getElementText((int) val);
            else
                return mSym->formatValue(val);
        } else
            return QVariant();
    }
    else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    }
    return QVariant();
}


void TableViewModel::calcDefaultColumnsTableView()
{
    mDefaultColumnTableView.clear();
    mDefaultColumnTableView.resize(columnCount());
    if(mSym->mType != GMS_DT_VAR && mSym->mType != GMS_DT_EQU)
        return; // symbols other than variable and equation do not have default values
    double defVal;
    for(int col=0; col<columnCount(); col++) {
        if (mSym->mType == GMS_DT_VAR)
            defVal = gmsDefRecVar[mSym->mSubType][col%GMS_VAL_MAX];
        else // mType == GMS_DT_EQU
            defVal = gmsDefRecEqu[mSym->mSubType][col%GMS_VAL_MAX];
        for(int row=0; row<rowCount(); row++) {
            QVector<uint> keys = mTvRowHeaders[row] + mTvColHeaders[col];
            double val = defVal;
            if (mTvKeysToValIdx.contains(keys))
                val = mSym->mValues[mTvKeysToValIdx[keys]];
            if(defVal != val) {
                mDefaultColumnTableView[col] = false;
                break;
            }
            mDefaultColumnTableView[col] = true;
        }
    }
}

void TableViewModel::initTableView(int nrColDim, QVector<int> dimOrder)
{
    if (dimOrder.isEmpty()) {
        for(int i=0; i<mSym->mDim; i++)
            dimOrder << i;
    }

    mTvColDim = nrColDim;
    mTvDimOrder = dimOrder;
    QSet<QVector<uint>> seenColHeaders;
    QSet<QVector<uint>> seenRowHeaders;

    mTvRowHeaders.clear();
    mTvColHeaders.clear();
    mTvKeysToValIdx.clear();
    QVector<uint> lastRowHeader(mSym->mDim-mTvColDim);
    for (int i=0; i<lastRowHeader.size(); i++)
        lastRowHeader[i] = 0;
    QVector<uint> lastColHeader(mTvColDim);
    for (int i=0; i<lastColHeader.size(); i++)
        lastColHeader[i] = 0;
    int r;
    for (int rec=0; rec<mSym->mFilterRecCount; rec++) {
        r = mSym->mRecSortIdx[mSym->mRecFilterIdx[rec]];
        int keyIdx = r*mSym->mDim;
        QVector<uint> rowHeader;
        QVector<uint> colHeader;

        for(int i=0; i<mSym->mDim-mTvColDim; i++)
            rowHeader.push_back(mSym->mKeys[keyIdx+mTvDimOrder[i]]);
        for(int i=mSym->mDim-mTvColDim; i<mSym->mDim; i++)
            colHeader.push_back(mSym->mKeys[keyIdx+mTvDimOrder[i]]);

        if (mSym->mType == GMS_DT_VAR || mSym->mType == GMS_DT_EQU) {
            colHeader.push_back(0);
            for (int valIdx=0; valIdx<GMS_VAL_MAX; valIdx++) {
                colHeader.pop_back();
                colHeader.push_back(valIdx);

                QVector<uint> keys = rowHeader + colHeader;
                mTvKeysToValIdx[keys] = r*GMS_VAL_MAX + valIdx;

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
            QVector<uint> keys = rowHeader + colHeader;
            mTvKeysToValIdx[keys] = r;

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

    calcDefaultColumnsTableView();

    tvSectionWidth->clear();
    tvSectionWidth->resize(mSym->mDim-mTvColDim);

    if (tvSectionWidth->isEmpty()) {
        tvSectionWidth->push_back(10);
    }

    if (mTvRowHeaders.isEmpty())
        mNeedDummyRow = true;
    else
        mNeedDummyRow = false;
    if (mTvColHeaders.isEmpty())
        mNeedDummyColumn = true;
    else
        mNeedDummyColumn = false;
}

bool TableViewModel::needDummyColumn() const
{
    return mNeedDummyColumn;
}

bool TableViewModel::needDummyRow() const
{
    return mNeedDummyRow;
}

QVector<int> *TableViewModel::getTvSectionWidth() const
{
    return tvSectionWidth;
}

int TableViewModel::dim()
{
    return mSym->mDim;
}

QVector<int> TableViewModel::tvDimOrder() const
{
    return mTvDimOrder;
}

QVector<bool> TableViewModel::defaultColumnTableView() const
{
    return mDefaultColumnTableView;
}

int TableViewModel::tvColDim() const
{
    return mTvColDim;
}

int TableViewModel::type()
{
    return mSym->type();
}

void TableViewModel::setTableView(bool tableView, int colDim, QVector<int> tvDims)
{
    beginResetModel();
    if (colDim!=-1) {
        mTvColDim = colDim;
        mTvDimOrder = tvDims;
    }
    initTableView(mTvColDim, mTvDimOrder);
    endResetModel();
}

bool TableViewModel::isAllDefault(int valColIdx)
{
    if(type() == GMS_DT_VAR || type() == GMS_DT_EQU)
        return mDefaultColumnTableView[valColIdx];
    else
        return false;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
