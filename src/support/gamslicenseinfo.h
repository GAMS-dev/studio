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
#ifndef GAMSLICENSEINFO_H
#define GAMSLICENSEINFO_H

#include "cfgmcc.h"

#include <QString>
#include <QStringList>

namespace gams {
namespace studio {
namespace support {

struct SolverInfo
{
    QString Name;
    QString Status; // TODO(AF): really a string?
    QString Capabilites; // TODO(AF): change type

    bool isValid() // TODO(AF): ...
    {
        return !Name.isEmpty();
    }
};

class GamsLicenseInfo
{
public:
    GamsLicenseInfo();
    ~GamsLicenseInfo();

    int solvers();

    QString solverName(int index);

//    SolverInfo solverInfo(int index);

    QStringList modelTypeNames();

private:
#if defined(__APPLE__) || defined(__unix__)
    const QString mConfigFile = "gmscmpun.txt";
#else
    const QString mConfigFile = "gmscmpnt.txt";
#endif
    const QString mLicenseFile = "gamslice.txt";

    cfgHandle_t mCFG;
};

}
}
}

#endif // GAMSLICENSEINFO_H
