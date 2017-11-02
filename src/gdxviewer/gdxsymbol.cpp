#include "gdxsymbol.h"
#include <memory>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbol::GdxSymbol(gdxHandle_t gdx, QStringList* uel2Label, QStringList* strPool, int nr, QString name, int dimension, int type, int subtype, int recordCount, QString explText, QObject *parent)
    : QAbstractTableModel(parent), mGdx(gdx), mUel2Label(uel2Label), mStrPool(strPool),  mNr(nr), mName(name), mDim(dimension), mType(type), mSubType(subtype), mRecordCount(recordCount), mExplText(explText)
{
    //loadData();
}

GdxSymbol::~GdxSymbol()
{
    delete mKeys;
    delete mValues;
}

QVariant GdxSymbol::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (section < mDim)
                return "Dim " + QString::number(section+1);
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
    if(!mIsLoaded)
    {
        int dummy;
        int* keys = new int[mDim];
        double* values = new double[GMS_VAL_MAX];
        gdxDataReadRawStart(mGdx, mNr, &dummy);
        mKeys = new int[mRecordCount*mDim];
        if (mType == GMS_DT_PAR || mType == GMS_DT_SET)
            mValues = new double[mRecordCount];
        else  if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
            mValues = new double[mRecordCount*GMS_DT_MAX];
        for(int i=0; i<mRecordCount; i++)
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
                    mValues[i*GMS_VAL_MAX+vIdx] =  values[vIdx];
            }
            mLoadedRecCount++;
        }
        gdxDataReadDone(mGdx);

        mIsLoaded = true;

        delete keys;
        delete values;
    }
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
