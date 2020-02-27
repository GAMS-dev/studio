#ifndef GAMS_STUDIO_NUMERICS_DOUBLEFORMATTER_H
#define GAMS_STUDIO_NUMERICS_DOUBLEFORMATTER_H

#include <QString>

namespace gams {
namespace studio {
namespace numerics {

class DoubleFormatter
{
public:
    enum Format {
        g = 0,
        f = 1,
        e = 2
    };
    static int gFormatFull;
    static QString format(double v, Format format, int precision, int squeeze);

private:
    DoubleFormatter() {};
};

} // namespace numerics
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NUMERICS_DOUBLEFORMATTER_H
