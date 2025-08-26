/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "distributionvalidator.h"

#include "commonpaths.h"
#include "gamslicenseinfo.h"

namespace gams {
namespace studio {
namespace support {

DistributionValidator::DistributionValidator(QObject *parent)
    : QThread(parent)
{
    CommonPaths::setSystemDir();
}

void DistributionValidator::run()
{
    checkCompatibility();
}

void DistributionValidator::checkCompatibility()
{
    GamsLicenseInfo gi;
    QString localVersion = gi.localDistribVersionStringShort();
    if (localVersion.isEmpty()) {
        QString error = QString("Could not find GAMS. Please check your GAMS setup. %1\n%2")
                                .arg("The installation instructions can be found at www.gams.com/latest/docs/UG_MAIN.html#UG_INSTALL",
                                     "Current path to GAMS: " + CommonPaths::systemDir());
        emit newError(error);
        return;
    }
    auto locVersion = localVersion.split('.');
    auto minVersion = QString(GAMS_DISTRIB_VERSION_SHORT).split('.');
    if (locVersion.at(0).toInt() > minVersion.at(0).toInt())
        return;
    if (locVersion.at(0).toInt() == minVersion.at(0).toInt() &&
        locVersion.at(1).toInt() >= minVersion.at(1).toInt())
        return;
    QString error = QString("Found incompatible GAMS %1 but GAMS %2 or higher was expected. Please upgrade your GAMS.")
            .arg(localVersion, GAMS_DISTRIB_VERSION_SHORT);
    emit newWarning(error);
}

}
}
}
