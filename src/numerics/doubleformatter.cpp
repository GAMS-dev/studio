#include "doubleformatter.h"
#include "doubleFormat.h"

namespace gams {
namespace studio {
namespace numerics {

//TODO(CW): create one function for formatting for reducing redundant code

QString DoubleFormatter::formatF(double v, int nDecimals, int squeeze)
{
    char outBuf[32];
    int outLen;
    char* p = x2fixed(v, nDecimals, squeeze, outBuf, &outLen);
    if (p)
        return QString(outBuf);
    else
        return "ERROR"; //TODO(CW): how to handle this? return NA?
}

QString DoubleFormatter::formatE(double v, int nSigFigs, int squeeze)
{
    char outBuf[32];
    int outLen;
    char* p = x2efmt(v, nSigFigs, squeeze, outBuf, &outLen);
    if (p)
        return QString(outBuf);
    else
        return "ERROR";
}

QString DoubleFormatter::formatG(double v, int nSigFigs, int squeeze)
{
    char outBuf[32];
    int outLen;
    char* p = x2gfmt(v, nSigFigs, squeeze, outBuf, &outLen);
    if (p)
        return QString(outBuf);
    else
        return "ERROR";
}

} // namespace numerics
} // namespace studio
} // namespace gams
