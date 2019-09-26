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
#include "locators/abstractsystemlogger.h"
#include "locators/sysloglocator.h"
#include "commonpaths.h"
#include "exception.h"
#include "common.h"
#include "cfgmcc.h"
#include "palmcc.h"
#include "gclgms.h"

namespace gams {
namespace studio {
namespace support {

GamsLicenseInfo::GamsLicenseInfo()
{
    auto logger = SysLogLocator::systemLog();

    cfgSetExitIndicator(0); // switch of exit() call
    cfgSetScreenIndicator(0);
    cfgSetErrorCallback(GamsLicenseInfo::errorCallback);

    char msg[GMS_SSSIZE];
    if (!cfgCreateD(&mCFG,
                    CommonPaths::systemDir().toStdString().c_str(),
                    msg,
                    sizeof(msg))) {
        logger->append(msg, LogMsgType::Error);
        EXCEPT() << "Could not open About GAMS dialog. " << msg;
    }
    if (cfgReadConfig(mCFG,
                      CommonPaths::configFile().toStdString().c_str())) {
        cfgGetMsg(mCFG, msg);
        logger->append(msg, LogMsgType::Error);
    }

    palSetExitIndicator(0); // switch of exit() call
    palSetScreenIndicator(0);
    palSetErrorCallback(GamsLicenseInfo::errorCallback);

    if (!palCreateD(&mPAL,
                    CommonPaths::systemDir().toStdString().c_str(),
                    msg,
                    sizeof(msg))) {
        logger->append(msg, LogMsgType::Error);
        EXCEPT() << "Could not open About GAMS dialog. " << msg;
    }
    int rc; // additional return code, not used here
    if (!palLicenseReadU(mPAL,
                         CommonPaths::licenseFile().toStdString().c_str(),
                         msg,
                         &rc))
        logger->append(msg, LogMsgType::Error);

}

GamsLicenseInfo::~GamsLicenseInfo()
{
    if (mCFG) cfgFree(&mCFG);
    if (mPAL) palFree(&mPAL);
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
    for (int i=1, j=1; i<=solvers(); ++i) {
        if (!cfgAlgHidden(mCFG, i)) {
            names[j++] = solverName(i);
        }
    }
    return names;
}

bool GamsLicenseInfo::isSolverHidden(const QString &solverName)
{
    return cfgAlgHidden(mCFG, solverId(solverName));
}

QString GamsLicenseInfo::solverOptDefFileName(const QString &solverName) const
{
#if CFGAPIVERSION > 2
    char name[GMS_SSSIZE];
    if (cfgDefFileName(mCFG, solverName.toStdString().c_str(), name))
        return QString("%1").arg(name);
    else
        return QString();
#else
        return QString();
#endif
}

QMap<QString, QString> GamsLicenseInfo::solverOptDefFileNames()
{
    QMap<QString, QString> fileNames;
    for (int i=1; i<=solvers(); ++i) {
        if (!cfgAlgHidden(mCFG, i)) {
            fileNames[solverName(i)] = solverOptDefFileName( solverName(i) );
        }
    }
    return fileNames;
}

QMap<int, int> GamsLicenseInfo::solverIndices()
{
    QMap<int, int> indices;
    for (int i=1, j=1; i<=solvers(); ++i) {
        if (!cfgAlgHidden(mCFG, i)) {
            indices[j++] = i;
        }
    }
    return indices;
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

QString GamsLicenseInfo::solverLicense(int solverId) const
{
    int days;
    char *codes = solverCodes(solverId);
    if (palLicenseCheckSubX(mPAL,
                            solverName(solverId).toStdString().c_str(),
                            codes,
                            &days))
        return "Demo";
    if (days == 0)
        return "Full";
    if (days > 0)
        return "Evaluation";
    return "Expired";
}

bool GamsLicenseInfo::isLicenseValid(const QStringList &license)
{
    int i = 1;
    for (auto line: license) {
        palLicenseRegisterGAMS(mPAL, i++, line.toStdString().c_str());
    }
    palLicenseRegisterGAMSDone(mPAL);
    return palLicenseValidation(mPAL) ? false : true;
}

char* GamsLicenseInfo::solverCodes(int solverId) const
{
    char msg[GMS_SSSIZE];
    char *codes = cfgAlgCode(mCFG, solverId, msg);
    return codes;
}

int GamsLicenseInfo::errorCallback(int count, const char *message)
{
    Q_UNUSED(count)
    auto logger = SysLogLocator::systemLog();
    logger->append(InvalidGAMS, LogMsgType::Error);
    logger->append(message, LogMsgType::Error);
    return 0;
}

}
}
}
