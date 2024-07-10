/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "process.h"

#include <QDate>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

namespace gams {
namespace studio {
namespace support {

QRegularExpression DistributionValidator::RegEx = QRegularExpression("^GAMS Release\\s*:\\s+(\\d\\d\\.\\d).*");

DistributionValidator::DistributionValidator(QObject *parent)
    : QThread(parent)
{
    CommonPaths::setSystemDir();
}

void DistributionValidator::run()
{
    checkBitness();
    checkCompatibility();
}

void DistributionValidator::checkBitness()
{
#ifdef _WIN32
    auto gamsPath = CommonPaths::systemDir();
    QFileInfo joat64(gamsPath + '/' + "joatdclib64.dll");
    bool is64 = (sizeof(void*) == 8);
    QStringList messages;
    if (gamsPath.isEmpty())
        return; //we can not check the bitness of GAMS if no GAMS was found
    if (!is64 && joat64.exists())
        messages << "GAMS Studio is 32 bit but 64 bit GAMS installation found. System directory:"
                 << gamsPath;
    if (is64 && !joat64.exists())
        messages << "GAMS Studio is 64 bit but 32 bit GAMS installation found. System directory:"
                 << gamsPath;
    emit newError(messages.join(" "));
#endif
}

void DistributionValidator::checkCompatibility()
{
    GamsProcess gp;
    QString about = gp.aboutGAMS();
    if (about.isEmpty()) {
        QString error = QString("Could not find GAMS. Please check your GAMS setup. %1\n%2")
                                .arg("The installation instructions can be found at www.gams.com/latest/docs/UG_MAIN.html#UG_INSTALL",
                                     "Current path to GAMS: " + CommonPaths::systemDir());
        emit newError(error);
        return;
    }

    auto match = RegEx.match(about);
    if (match.hasMatch() && RegEx.captureCount() == 1) {
        auto version = match.captured(RegEx.captureCount()).split('.');
        auto minVersion = QString(GAMS_DISTRIB_VERSION_SHORT).split('.');
        if (version.at(0).toInt() > minVersion.at(0).toInt())
            return;
        if (version.at(0).toInt() == minVersion.at(0).toInt() &&
            version.at(1).toInt() >= minVersion.at(1).toInt())
            return;
        QString error = QString("Found incompatible GAMS %1 but GAMS %2 or higher was expected. Please upgrade your GAMS.")
                .arg(match.captured(RegEx.captureCount()), GAMS_DISTRIB_VERSION_SHORT);
        emit newWarning(error);
    }
    else {
        emit newError("Could not validate GAMS Distribution version.");
    }
}

}
}
}
