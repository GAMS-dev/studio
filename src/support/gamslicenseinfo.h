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

#include <QMap>
#include <QString>
#include <QStringList>

struct cfgRec;
typedef struct cfgRec *cfgHandle_t;

struct palRec;
typedef struct palRec *palHandle_t;

namespace gams {
namespace studio {
namespace support {

class GamsLicenseInfo
{
public:
    GamsLicenseInfo();
    ~GamsLicenseInfo();

    int solvers() const;

    int solverId(const QString &name);

    QString solverName(int id) const;
    QMap<int, QString> solverNames();

    QMap<int, QString> modelTypeNames();

    bool solverCapability(int solver, int modelType) const;

    QString solverLicense(int id) const;

private:
#if defined(__APPLE__) || defined(__unix__)
    const QString mConfigFile = "gmscmpun.txt";
#else
    const QString mConfigFile = "gmscmpnt.txt";
#endif
    const QString mLicenseFile = "gamslice.txt";

    cfgHandle_t mCFG;
    palHandle_t mPAL;
};

}
}
}

#endif // GAMSLICENSEINFO_H
