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
#include "solverconfiginfo.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"
#include "exception.h"
#include "common.h"
#include "cfgmcc.h"
#include "gclgms.h"

namespace gams {
namespace studio {
namespace support {

SolverConfigInfo::SolverConfigInfo()
{
    auto logger = SysLogLocator::systemLog();

    cfgSetExitIndicator(0); // switch of exit() call
    cfgSetScreenIndicator(0);
    cfgSetErrorCallback(SolverConfigInfo::errorCallback);

    char msg[GMS_SSSIZE];
    if (!cfgCreateD(&mCFG,
                    CommonPaths::systemDir().toStdString().c_str(),
                    msg,
                    sizeof(msg))) {
        logger->append(msg, LogMsgType::Error);
        EXCEPT() << "Could not initialize from gams system directory. " << msg;
    }
    if (cfgReadConfig(mCFG,
                      CommonPaths::configFile().toStdString().c_str())) {
        cfgGetMsg(mCFG, msg);
        logger->append(msg, LogMsgType::Error);
    }
}

SolverConfigInfo::~SolverConfigInfo()
{
    if (mCFG) cfgFree(&mCFG);
}

int SolverConfigInfo::solvers() const
{
    return cfgNumAlgs(mCFG);
}

int SolverConfigInfo::modelTypes() const
{
    return cfgProc_nrofmodeltypes;
}

int SolverConfigInfo::solverId(const QString &name) const
{
    return cfgAlgNumber(mCFG, name.toStdString().c_str());
}

QString SolverConfigInfo::solverName(int id) const
{
    char name[GMS_SSSIZE];
    QString result = cfgAlgName(mCFG, id, name);
    if (result == "UnknownSolver")
        return QString();
    return result;
}

QMap<int, QString> SolverConfigInfo::solverNames()
{
    QMap<int, QString> names;
    for (int i=1, j=1; i<=solvers(); ++i) {
        if (!cfgAlgHidden(mCFG, i)) {
            names[j++] = solverName(i);
        }
    }
    return names;
}

bool SolverConfigInfo::isSolverHidden(const QString &solverName)
{
    return cfgAlgHidden(mCFG, solverId(solverName));
}

QString SolverConfigInfo::solverOptDefFileName(const QString &solverName) const
{
#if CFGAPIVERSION > 2
    char name[GMS_SSSIZE];
    if (cfgDefFileName(mCFG, solverName.toStdString().c_str(), name))
        return QString("%1").arg(name);
    else
        return QString();
#else
    Q_UNUSED(solverName)
    return QString();
#endif
}

QMap<QString, QString> SolverConfigInfo::solverOptDefFileNames()
{
    QMap<QString, QString> fileNames;
#if CFGAPIVERSION > 2
    for (int i=1; i<=solvers(); ++i) {
        if (!cfgAlgHidden(mCFG, i)) {
            fileNames[solverName(i)] = solverOptDefFileName( solverName(i) );
        }
    }
#endif
    return fileNames;
}

QMap<QString, QString> SolverConfigInfo::solverAliasNames()
{
    QMap<QString, QString> alias;
    // determine alais from option definition file name
    for (auto [key, value] : solverOptDefFileNames().asKeyValueRange()) {
        QString optdeffile = QString("opt%1.def").arg(key.toLower());
        if (QString::compare(optdeffile, value, Qt::CaseInsensitive) != 0) {
           alias[key.toLower()] =  value.split('.', Qt::SkipEmptyParts).first().mid(3);
        }
    }
    return alias;
}

QMap<int, int> SolverConfigInfo::solverIndices()
{
    QMap<int, int> indices;
    for (int i=1, j=1; i<=solvers(); ++i) {
        if (!cfgAlgHidden(mCFG, i)) {
            indices[j++] = i;
        }
    }
    return indices;
}

QString SolverConfigInfo::modelTypeName(int modelTypeId) const
{
    char modelType[GMS_SSSIZE];
    cfgModelTypeName(mCFG, modelTypeId, modelType);
    return QString(modelType);
}

QString SolverConfigInfo::defaultSolverFormodelTypeName(int modelTypeId) const
{
    int solverid = cfgDefaultAlg(mCFG, modelTypeId);
    return solverName(solverid);
}

QMap<int, QString> SolverConfigInfo::modelTypeNames()
{
    QMap<int, QString> modelTypes;
    char modelType[GMS_SSSIZE];
    for (int i=1; i<cfgProc_nrofmodeltypes; ++i) {
        cfgModelTypeName(mCFG, i, modelType);
        modelTypes[i] = modelType;
    }
    return modelTypes;
}

bool SolverConfigInfo::solverCapability(int solver, int modelType) const
{
    return cfgAlgCapability(mCFG, solver, modelType);
}

QStringList SolverConfigInfo::solversForModelType(int modelType) const
{
    QStringList solverList;
    for (int i=1; i<=solvers(); ++i) {
        if (cfgAlgCapability(mCFG, i, modelType) && !cfgAlgHidden(mCFG, i)) {
            solverList << solverName(i);
        }
    }
    return solverList;
}

QString SolverConfigInfo::solverCodes(int solverId) const
{
    char codes[GMS_SSSIZE];
    cfgAlgCode(mCFG, solverId, codes);
    return codes;
}

int SolverConfigInfo::errorCallback(int count, const char *message)
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
