/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "gamsprocess.h"
#include "locators/sysloglocator.h"
#include "locators/abstractsystemlogger.h"

#include <QStringList>
#include <QFileInfo>
#include <QDir>

namespace gams {
namespace studio {

DistributionValidator::DistributionValidator(QObject *parent)
    : QThread(parent)
{

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
    QFileInfo joat64(gamsPath + QDir::separator() + "joatdclib64.dll");
    bool is64 = (sizeof(void*) == 8);
    QStringList messages;
    if (!is64 && joat64.exists())
        messages << "GAMS Studio is 32 bit but 64 bit GAMS installation found. System directory:"
                 << gamsPath;
    if (is64 && !joat64.exists())
        messages << "GAMS Studio is 64 bit but 32 bit GAMS installation found. System directory:"
                 << gamsPath;
    SysLogLocator::systemLog()->appendLog(messages.join(" "), LogMsgType::Error);
#endif
}

void DistributionValidator::checkCompatibility()
{
    GamsProcess gp;
    QString about = gp.aboutGAMS();
    if (about.isEmpty()) {
        QString error = QString("Could not find GAMS. Please check our GAMS setup. %1\n%2")
                                .arg("The installation instructions can be found at www.gams.com/latest/docs/UG_MAIN.html#UG_INSTALL")
                                .arg("Current path to GAMS: " + CommonPaths::systemDir());
        SysLogLocator::systemLog()->appendLog(error, LogMsgType::Error);
        return;
    }
    else if (about.contains(GAMS_DISTRIB_VERSION_SHORT)) {
        return;
    }
    else if (about.contains(GAMS_DISTRIB_VERSION_NEXT_SHORT)) {
        return;
    }

    QRegExp regex(".*GAMS Release\\s+:\\s+(\\d\\d\\.\\d).*");
    if (regex.exactMatch(about) && regex.captureCount() == 1) {
        QString error = QString("Found incompatible GAMS %1 but GAMS %2 or %3 was expected.")
                .arg(regex.cap(regex.captureCount()))
                .arg(GAMS_DISTRIB_VERSION_SHORT).arg(GAMS_DISTRIB_VERSION_NEXT_SHORT);
        SysLogLocator::systemLog()->appendLog(error, LogMsgType::Error);
    } else
        SysLogLocator::systemLog()->appendLog("Could not validate GAMS Distribution version.", LogMsgType::Error);
}

}
}
