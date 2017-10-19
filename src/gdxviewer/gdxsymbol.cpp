#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {


GDXSymbol::GDXSymbol(int nr, QString name, int dimension, int type, int subType, int recordCount, QString explText, gdxHandle_t gdx)
    : mNr(nr), mName(name), mDim(dimension), mType(type), mSubType(subType), mRecordCount(recordCount), mExplText(explText), mGdx(gdx)
{

}

GDXSymbol::~GDXSymbol()
{
    delete mKeys;
    delete mValues;
}

int GDXSymbol::nr() const
{
    return mNr;
}

QString GDXSymbol::name() const
{
    return mName;
}

int GDXSymbol::dim() const
{
    return mDim;
}

int GDXSymbol::type() const
{
    return mType;
}

int GDXSymbol::subType() const
{
    return mSubType;
}

int GDXSymbol::recordCount() const
{
    return mRecordCount;
}

QString GDXSymbol::explText() const
{
    return mExplText;
}

void GDXSymbol::loadData()
{
    int dummy;
    int dimFirst;
    int* keys = new int[mDim];
    double* values = new double[GMS_VAL_MAX];
    gdxDataReadRawStart(mGdx, mNr, &dummy);
    mKeys = new int[mRecordCount*mDim];
    if (mType == GMS_DT_PAR)
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
        if (mType == GMS_DT_PAR)
            mValues[i] = values[0];
        else if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
        {
            for(int vIdx=0; vIdx<GMS_VAL_MAX; vIdx++)
                mValues[i*GMS_VAL_MAX+vIdx] =  values[vIdx];
        }
    }
    gdxDataReadDone(mGdx);

    delete keys;
    delete values;
}

int GDXSymbol::key(int rowIdx, int colIdx) const
{
    return mKeys[rowIdx*mDim + colIdx];
}

double GDXSymbol::value(int rowIdx, int colIdx) const
{
    if (mType == GMS_DT_PAR)
        return mValues[rowIdx];
    else if (mType == GMS_DT_EQU || mType == GMS_DT_VAR)
        return mValues[rowIdx*GMS_DT_MAX + colIdx];
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
