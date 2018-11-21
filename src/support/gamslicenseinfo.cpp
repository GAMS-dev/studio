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
}

GamsLicenseInfo::~GamsLicenseInfo()
{
    if (mCFG) cfgFree(&mCFG);
}

int GamsLicenseInfo::solvers() const
{
    return cfgNumAlgs(mCFG);
}

QString GamsLicenseInfo::solverName(int index) const
{
    char name[GMS_SSSIZE];
    QString result = cfgAlgName(mCFG, index, name);
    if (result == "UnknownSolver")
        return QString();
    return result;
}

SolverInfo GamsLicenseInfo::solverInfo(int index)
{
    SolverInfo si;
    si.Id = index;
    si.Name = solverName(index);
    //si.Status = ""; // TODO(AF): ...
    //si.Capabilites = ""; // TODO(AF): ...
    return si;
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

}
}
}
