#include "gdxsymbol.h"
#include <memory>
#include <QThread>
#include <QtConcurrent>
#include <QTime>
#include <QIcon>
#include <QVarLengthArray>
#include <algorithm>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbol::GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, int nr, GdxSymbolTable* gdxSymbolTable, QObject *parent)
    : QAbstractTableModel(parent), mGdx(gdx), mGdxMutex(gdxMutex), mNr(nr), mGdxSymbolTable(gdxSymbolTable)
{
    loadMetaData();
    loadDomains();

    mRecSortIdx = new int[mRecordCount];
    for(int i=0; i<mRecordCount; i++)
        mRecSortIdx[i] = i;

    mRecFilterIdx = new int[mRecordCount];
    for(int i=0; i<mRecordCount; i++)
        mRecFilterIdx[i] = i;

    mFilterActive = new bool[mRecordCount] {false};
}

GdxSymbol::~GdxSymbol()
{
    if(mKeys)
        delete mKeys;
    if (mValues)
        delete mValues;
    if (mRecSortIdx)
        delete mRecSortIdx;
    if (mRecFilterIdx)
        delete mRecFilterIdx;
    if (mMinUel)
        delete mMinUel;
    if (mMaxUel)
        delete mMaxUel;
    if(mFilterActive)
        delete mFilterActive;
    for(auto v : mUelsInColumn)
        delete v;
    for(auto a: mShowUelInColumn)
    {
        if(a)
            delete a;
    }
}

QVariant GdxSymbol::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (section < mDim)
            {
                return mDomains.at(section);
            }
            else
            {
                if (mType == GMS_DT_SET)
                    return "Text";
                else if (mType == GMS_DT_PAR)
                    return "Value";
                else if (mType == GMS_DT_VAR || mType == GMS_DT_EQU)
                switch(section-mDim)
                {
                case GMS_VAL_LEVEL: return "Level";
                case GMS_VAL_MARGINAL: return "Marginal";
                case GMS_VAL_LOWER: return "Lower";
                case GMS_VAL_UPPER: return "Upper";
                case GMS_VAL_SCALE: return "Scale";
                }
            }
        }
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

    else if (role == Qt::DisplayRole)
    {
        int row = mRecSortIdx[mRecFilterIdx[index.row()]];
        if (index.column() < mDim)
            return mGdxSymbolTable->uel2Label().at(mKeys[row*mDim + index.column()]);
        else
        {
            double val;
            if (mType == GMS_DT_PAR)
                val = mValues[row];
            else if (mType == GMS_DT_SET)
            {
                val = mValues[row];
                return gdxSymbolTable()->strPool().at((int) val);
            }
            else if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
                val = mValues[row*GMS_DT_MAX + (index.column()-mDim)];
            //apply special values:
            if (val<GMS_SV_UNDEF)
            {
                return val;
            }
            else
            {
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
                //TODO(CW): check special values
            }
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if (index.column() >= mDim)
        {
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
    QTime t;
    t.start();
    QMutexLocker locker(mGdxMutex);
    mMinUel = new int[mDim] {INT_MAX};
    mMaxUel = new int[mDim] {INT_MIN};

    if(!mIsLoaded)
    {
        beginResetModel();
        endResetModel();

        if(!mKeys)
            mKeys = new int[mRecordCount*mDim];
        if(!mValues)
        {
            if (mType == GMS_DT_PAR || mType == GMS_DT_SET)
                mValues = new double[mRecordCount];
            else  if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
                mValues = new double[mRecordCount*GMS_DT_MAX];
        }

        int dummy;
        int* keys = new int[mDim];
        double* values = new double[GMS_VAL_MAX];
        gdxDataReadRawStart(mGdx, mNr, &dummy);

        for(int i=0; i<mLoadedRecCount; i++) //skip records that has already been loaded
        {
            gdxDataReadRaw(mGdx, keys, values, &dummy);
            if(stopLoading) //TODO(CW): redundant code (see below)
            {
                stopLoading = false;
                gdxDataReadDone(mGdx);
                delete keys;
                delete values;
                return;
            }
        }

        int updateCount = 1000000;
        int keyOffset;
        int valOffset;
        int k;
        for(int i=mLoadedRecCount; i<mRecordCount; i++)
        {
            keyOffset = i*mDim;
            gdxDataReadRaw(mGdx, keys, values, &dummy);

            for(int j=0; j<mDim; j++)
            {
                k = keys[j];
                mKeys[keyOffset+j] = k;
                mMinUel[j] = qMin(mMinUel[j], k);
                mMaxUel[j] = qMax(mMaxUel[j], k);
            }
            if (mType == GMS_DT_PAR || mType == GMS_DT_SET)
                mValues[i] = values[0];
            else if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
            {
                valOffset = i*GMS_VAL_MAX;
                for(int vIdx=0; vIdx<GMS_VAL_MAX; vIdx++)
                {
                    mValues[valOffset+vIdx] =  values[vIdx];
                }

            }
            mLoadedRecCount++;
            mFilterRecCount = mLoadedRecCount;
            if(i%updateCount == 0)
            {
                beginResetModel();
                endResetModel();
            }
            if(stopLoading)
            {
                stopLoading = false;
                gdxDataReadDone(mGdx);
                delete keys;
                delete values;
                return;
            }
        }
        gdxDataReadDone(mGdx);

        beginResetModel();
        endResetModel();
        calcDefaultColumns();
        calcUelsInColumn();
        mIsLoaded = true;

        delete keys;
        delete values;

        qDebug() << "loadData: " << t.elapsed();
    }
}

void GdxSymbol::stopLoadingData()
{
    stopLoading = true;
}

bool GdxSymbol::squeezeDefaults() const
{
    return mSqueezeDefaults;
}

void GdxSymbol::setSqueezeDefaults(bool squeezeDefaults)
{
    mSqueezeDefaults = squeezeDefaults;
}

void GdxSymbol::calcDefaultColumns()
{
    if(mType != GMS_DT_VAR && mType != GMS_DT_EQU)
        return; // symbols other than variable and equation do not have default values
    double defVal;
    for(int valColIdx=0; valColIdx<GMS_VAL_MAX; valColIdx++)
    {
        if (mType == GMS_DT_VAR)
            defVal = gmsDefRecVar[mSubType][valColIdx];
        else if (mType == GMS_DT_EQU)
            defVal = gmsDefRecEqu[mSubType][valColIdx];
        for(int i=0; i<mRecordCount; i++)
        {
            if(defVal != mValues[i*GMS_VAL_MAX + valColIdx])
            {
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
    for(int dim=0; dim<mDim; dim++)
    {
        QVector<int>* uels = new QVector<int>();
        bool* sawUel = new bool[mMaxUel[dim]+1] {false}; //TODO(CW): squeeze using mMinUel

        int lastUel = -1;
        int currentUel = - 1;
        for(int rec=0; rec<mRecordCount; rec++)
        {
            currentUel = mKeys[rec*mDim + dim];
            if(lastUel != currentUel)
            {
                lastUel = currentUel;
                if(!sawUel[currentUel])
                {
                    sawUel[currentUel] = true;
                    uels->append(currentUel);
                }
            }
        }
        mUelsInColumn.append(uels);
        mShowUelInColumn.append(sawUel);
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
    if(mType == GMS_DT_EQU)
        mSubType = gmsFixEquType(mSubType);
    if(mType == GMS_DT_VAR)
        mSubType = gmsFixVarType(mSubType);
}

void GdxSymbol::loadDomains()
{
    gdxStrIndexPtrs_t domX;
    gdxStrIndex_t     domXXX;
    GDXSTRINDEXPTRS_INIT(domXXX,domX);
    gdxSymbolGetDomainX(mGdx, mNr, domX);
    for(int i=0; i<mDim; i++)
        mDomains.append(domX[i]);
}

bool *GdxSymbol::filterActive() const
{
    return mFilterActive;
}

void GdxSymbol::setShowUelInColumn(const QVector<bool *> &showUelInColumn)
{
    mShowUelInColumn = showUelInColumn;
}

QVector<bool *> GdxSymbol::showUelInColumn() const
{
    return mShowUelInColumn;
}

QVector<QVector<int> *> GdxSymbol::uelsInColumn() const
{
    return mUelsInColumn;
}

Qt::SortOrder GdxSymbol::sortOrder() const
{
    return mSortOrder;
}

void GdxSymbol::resetSortFilter()
{
    for(int i=0; i<mRecordCount; i++)
    {
        mRecSortIdx[i] = i;
        mRecFilterIdx[i] = i;
        mFilterActive[i] = false;
    }
    for(int dim=0; dim<mDim; dim++)
    {
        for(int uel : *mUelsInColumn.at(dim))
        {
            mShowUelInColumn.at(dim)[uel] = true;
        }
    }

    mFilterRecCount = mLoadedRecCount; //TODO(CW): use mRecordCount ?
    mSortColumn = -1;
    layoutChanged();
}

int GdxSymbol::sortColumn() const
{
    return mSortColumn;
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
    if(column == mSortColumn && order == mSortOrder)
        return;

    QTime t;
    t.start();

    int* labelCompIdx = mGdxSymbolTable->labelCompIdx();

    // sort by key column
    if(column<mDim)
    {
        QList<QPair<int, int>> l;
        for(int rec=0; rec<mRecordCount; rec++)
            l.append(QPair<int, int>(labelCompIdx[mKeys[mRecSortIdx[rec]*mDim + column]], mRecSortIdx[rec]));

        if(order == Qt::SortOrder::AscendingOrder)
            std::stable_sort(l.begin(), l.end(), [](QPair<int, int> a, QPair<int, int> b) { return a.first < b.first; });
        else
            std::stable_sort(l.begin(), l.end(), [](QPair<int, int> a, QPair<int, int> b) { return a.first > b.first; });

        for(int rec=0; rec< mRecordCount; rec++)
            mRecSortIdx[rec] = l.at(rec).second;
    }

    //TODO(CW): make string pool sorting index like for uels for increasing sort speed on explanatory text
    //sort set and alias by explanatory text
    else if(mType == GMS_DT_SET || mType == GMS_DT_ALIAS)
    {
        QList<QPair<QString, int>> l;
        for(int rec=0; rec<mRecordCount; rec++)
            l.append(QPair<QString, int>(mGdxSymbolTable->strPool().at(mValues[mRecSortIdx[rec]]), mRecSortIdx[rec]));

        if(order == Qt::SortOrder::AscendingOrder)
            std::stable_sort(l.begin(), l.end(), [](QPair<QString, int> a, QPair<QString, int> b) { return a.first < b.first; });
        else
            std::stable_sort(l.begin(), l.end(), [](QPair<QString, int> a, QPair<QString, int> b) { return a.first > b.first; });

        for(int rec=0; rec< mRecordCount; rec++)
            mRecSortIdx[rec] = l.at(rec).second;
    }

    // sort parameter, variable and equation by value columns
    else
    {
        QList<QPair<double, int>> l;
        if(mType == GMS_DT_PAR)
        {
            for(int rec=0; rec<mRecordCount; rec++)
                l.append(QPair<double, int>(mValues[mRecSortIdx[rec]], mRecSortIdx[rec]));
        }
        else if (mType == GMS_DT_VAR || mType == GMS_DT_EQU)
        {
            for(int rec=0; rec<mRecordCount; rec++)
                l.append(QPair<double, int>(mValues[mRecSortIdx[rec]*GMS_VAL_MAX + (column-mDim)], mRecSortIdx[rec]));
        }

        if(order == Qt::SortOrder::AscendingOrder)
            std::stable_sort(l.begin(), l.end(), [](QPair<double, int> a, QPair<double, int> b) { return a.first < b.first; });
        else
            std::stable_sort(l.begin(), l.end(), [](QPair<double, int> a, QPair<double, int> b) { return a.first > b.first; });

        for(int rec=0; rec< mRecordCount; rec++)
            mRecSortIdx[rec] = l.at(rec).second;
    }

    mSortColumn = column;
    mSortOrder = order;
    layoutChanged();
    filterRows();
}

void GdxSymbol::filterRows()
{
    qDebug() << "filterRows";
    QTime t;
    t.start();

    for(int i=0; i<mRecordCount; i++)
        mRecFilterIdx[i] = i;

    int removedCount = 0;

    mFilterRecCount = mLoadedRecCount;
    for(int row=0; row<mRecordCount; row++)
    {
        int recIdx = mRecSortIdx[row];
        mRecFilterIdx[row-removedCount] = row;
        for(int dim=0; dim<mDim; dim++)
        {
            if(!mShowUelInColumn.at(dim)[mKeys[recIdx*mDim + dim]]) //filter record
            {
                mFilterRecCount--;
                removedCount++;
                break;
            }
        }
    }
    beginResetModel();
    endResetModel();
    qDebug() << "filterRows: " << t.elapsed();
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
