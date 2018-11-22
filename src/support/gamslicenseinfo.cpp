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
#include "gamslicenseinfo.h"
#include "commonpaths.h"
#include "gclgms.h"

#include <QDebug>

namespace gams {
namespace studio {
namespace support {

GamsLicenseInfo::GamsLicenseInfo()
{
    char msg[GMS_SSSIZE];
    if (!cfgCreateD(&mCFG,
                    CommonPaths::systemDir().toStdString().c_str(),
                    msg,
                    sizeof(msg)))
        qDebug() << "ERROR: " << msg; // TODO(AF): execption/syslog
    auto configFile = CommonPaths::systemDir() + "/" + mConfigFile; // TODO(AF): QDIR/CommonPath usage?
    if (cfgReadConfig(mCFG, configFile.toStdString().c_str())) {
        cfgGetMsg(mCFG, msg);
        qDebug() << "ERROR: " << msg; // TODO(AF): execption/syslog
    }
    if (!palCreateD(&mPAL,
                    CommonPaths::systemDir().toStdString().c_str(),
                    msg,
                    sizeof(msg)))
        qDebug() << "ERROR: " << msg; // TODO(AF): execption/syslog
}

GamsLicenseInfo::~GamsLicenseInfo()
{
    if (mCFG) cfgFree(&mCFG);
}

int GamsLicenseInfo::solvers() const
{
    return cfgNumAlgs(mCFG);
}

int GamsLicenseInfo::solverId(const QString &name)
{
    return cfgAlgNumber(mCFG, name.toStdString().c_str());
}

QString GamsLicenseInfo::solverName(int id) const
{
    char name[GMS_SSSIZE];
    QString result = cfgAlgName(mCFG, id, name);
    if (result == "UnknownSolver")
        return QString();
    return result;
}

QMap<int, QString> GamsLicenseInfo::solverNames()
{
    QMap<int, QString> names;
    for (int i=0; i<solvers(); ++i)
        names[i] = solverName(i);
    return names;
}

QMap<int, QString> GamsLicenseInfo::modelTypeNames()
{
    QMap<int, QString> modelTypes;
    char modelType[GMS_SSSIZE];
    for (int i=1; i<cfgProc_nrofmodeltypes; ++i) {
        cfgModelTypeName(mCFG, i, modelType);
        modelTypes[i] = modelType;
    }
    return modelTypes;
}

bool GamsLicenseInfo::solverCapability(int solver, int modelType) const
{
    return cfgAlgCapability(mCFG, solver, modelType);
}

QString GamsLicenseInfo::solverLicense(int id) const
{
    int days = 0; // TODO(AF): pal call
    if (days == 0)
        return "Full";
    if (days > 0)
        return "Evaluation";
    return "Expired";
}

}
}
}
