#include "result.h"

namespace gams {
namespace studio {

Result::Result(int locLineNr, int locCol, QString locFile, QString node) :
    mLocLineNr(locLineNr), mLocCol(locCol), mLocFile(locFile), mNode(node)
{ }

int Result::locLineNr() const
{
    return mLocLineNr;
}

int Result::locCol() const
{
    return mLocCol;
}

QString Result::locFile() const
{
    return mLocFile;
}

QString Result::node() const
{
    return mNode;
}

}
}
