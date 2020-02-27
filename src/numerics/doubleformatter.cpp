#include "doubleformatter.h"
#include "doubleFormat.h"

namespace gams {
namespace studio {
namespace numerics {

int DoubleFormatter::gFormatFull = 0;

QString DoubleFormatter::format(double v, DoubleFormatter::Format format, int precision, int squeeze)
{
    char outBuf[32];
    int outLen;
    char* p = nullptr;
    if (format == Format::g)
        p = x2gfmt(v, precision, squeeze, outBuf, &outLen);
    else if (format == Format::f)
        p = x2fixed(v, precision, squeeze, outBuf, &outLen);
    else if (format == Format::e)
        p = x2efmt(v, precision, squeeze, outBuf, &outLen);
    if (!p)
        return "FORMAT_ERROR";
    else
        return QString(p);
}

} // namespace numerics
} // namespace studio
} // namespace gams
