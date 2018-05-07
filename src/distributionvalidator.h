#ifndef DISTRIBUTIONVALIDATOR_H
#define DISTRIBUTIONVALIDATOR_H

#include <QString>

namespace gams {
namespace studio {

class DistributionValidator
{
public:
    DistributionValidator();

    ///
    /// \brief Verify the GAMS Distribution bitness on Windows.
    /// \remark On Windows we support both 32 Bit and 64 Bit.
    ///
    QString checkBitness();
};

}
}



#endif // DISTRIBUTIONVALIDATOR_H
