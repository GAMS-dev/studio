#include "distributionvalidator.h"
#include "commonpaths.h"

#include <QStringList>
#include <QFileInfo>
#include <QDir>

namespace gams {
namespace studio {

DistributionValidator::DistributionValidator()
{

}

QString DistributionValidator::checkBitness()
{
#ifdef _WIN32
    auto gamsPath = CommonPaths::systemDir();
    QFileInfo joat64(gamsPath + QDir::separator() + "joatdclib64.dll");
    bool is64 = (sizeof(int*) == 8) ? true : false;
    QStringList messages;
    if (!is64 && joat64.exists())
        messages << "ERROR: GAMS Studio is 32 bit but 64 bit GAMS installation found. System directory:"
                 << gamsPath;
    if (is64 && !joat64.exists())
        messages << "ERROR: GAMS Studio is 64 bit but 32 bit GAMS installation found. System directory:"
                 << gamsPath;
    return messages.join(" ");
#else
    return QString();
#endif
}

}
}
