#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {


GDXSymbol::GDXSymbol(int nr, QString name, int dimension, int type, int subType, int recordCount, QString explText)
    : mNr(nr), mName(name), mDim(dimension), mType(type), mSubType(subType), mRecordCount(recordCount), mExplText(explText)
{

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

} // namespace gdxviewer
} // namespace studio
} // namespace gams
