#include "result.h"

namespace gams {
namespace studio {

Result::Result(int locLineNr, int locCol, QString locFile, QString node) :
    mLineNr(locLineNr), mColNr(locCol), mFilepath(locFile), mContext(node)
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

}
}
