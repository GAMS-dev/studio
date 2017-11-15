#include "gdxsymbol.h"
#include <memory>
#include <QThread>
#include <QtConcurrent>
#include <QTime>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbol::GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, QStringList* uel2Label, QStringList* strPool, int nr, QString name, int dimension, int type, int subtype, int recordCount, QString explText, QObject *parent)
    : QAbstractTableModel(parent), mGdx(gdx), mGdxMutex(gdxMutex), mUel2Label(uel2Label), mStrPool(strPool),  mNr(nr), mName(name), mDim(dimension), mType(type), mSubType(subtype), mRecordCount(recordCount), mExplText(explText)
{
    // read domains
    mDomains.clear();
    gdxStrIndexPtrs_t Indx;
    gdxStrIndex_t     IndxXXX;
    GDXSTRINDEXPTRS_INIT(IndxXXX,Indx);
    gdxSymbolGetDomainX(mGdx, mNr, Indx);
    for(int i=0; i<mDim; i++)
        mDomains.append(Indx[i]);
}

GdxSymbol::~GdxSymbol()
{
    if(mKeys)
        delete mKeys;
    if (mValues)
        delete mValues;
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
    return mLoadedRecCount;
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
        if (index.column() < mDim)
            return mUel2Label->at(mKeys[index.row()*mDim + index.column()]);
        else
        {
            double val;
            if (mType == GMS_DT_PAR)
                val = mValues[index.row()];
            else if (mType == GMS_DT_SET)
            {
                val = mValues[index.row()];
                return mStrPool->at((int) val);
            }
            else if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
                val = mValues[index.row()*GMS_DT_MAX + (index.column()-mDim)];
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
        for(int i=mLoadedRecCount; i<mRecordCount; i++)
        {
            gdxDataReadRaw(mGdx, keys, values, &dummy);
            for(int j=0; j<mDim; j++)
            {
                mKeys[i*mDim+j] = keys[j];
            }
            if (mType == GMS_DT_PAR || mType == GMS_DT_SET)
                mValues[i] = values[0];
            else if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
            {
                for(int vIdx=0; vIdx<GMS_VAL_MAX; vIdx++)
                {
                    mValues[i*GMS_VAL_MAX+vIdx] =  values[vIdx];
                }

            }
            mLoadedRecCount++;
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
        mIsLoaded = true;

        delete keys;
        delete values;
        qDebug() << t.elapsed();
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
        else if (mType == GMS_DT_VAR)
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

bool GdxSymbol::isAllDefault(int valColIdx)
{
    if(mType == GMS_DT_VAR || mType == GMS_DT_EQU)
        return mDefaultColumn[valColIdx];
    else
        return false;
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
