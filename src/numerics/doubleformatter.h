#ifndef GAMS_STUDIO_NUMERICS_DOUBLEFORMATTER_H
#define GAMS_STUDIO_NUMERICS_DOUBLEFORMATTER_H

#include <QString>

namespace gams {
namespace studio {
namespace numerics {

class DoubleFormatter
{
public:

    static QString formatF (double v, int nDecimals, int squeeze);

    static QString formatE (double v, int nSigFigs, int squeeze);

    static QString formatG (double v, int nSigFigs, int squeeze);

private:
    DoubleFormatter() {};
};

} // namespace numerics
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NUMERICS_DOUBLEFORMATTER_H
