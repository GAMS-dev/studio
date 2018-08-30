#include "result.h"

namespace gams {
namespace studio {

Result::Result(int lineNr, int colNr, int length, QString fileLoc, QString context) :
    mLineNr(lineNr), mColNr(colNr), mLength(length), mFilepath(fileLoc), mContext(context)
{ }

int Result::lineNr() const
{
    return mLineNr;
}

int Result::colNr() const
{
    return mColNr;
}

QString Result::filepath() const
{
    return mFilepath;
}

QString Result::context() const
{
    return mContext;
}

int Result::length() const
{
    return mLength;
}

}
}
