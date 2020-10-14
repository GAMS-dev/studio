/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"
#include "exception.h"
#include "common.h"
#include "palmcc.h"
#include "gclgms.h"

#include <QClipboard>
#include <QGuiApplication>

namespace gams {
namespace studio {
namespace support {

GamsLicenseInfo::GamsLicenseInfo()
{
    auto logger = SysLogLocator::systemLog();
    char msg[GMS_SSSIZE];

    palSetExitIndicator(0); // switch of exit() call
    palSetScreenIndicator(0);
    palSetErrorCallback(GamsLicenseInfo::errorCallback);

    if (!palCreateD(&mPAL,
                    CommonPaths::systemDir().toStdString().c_str(),
                    msg,
                    sizeof(msg))) {
        logger->append(msg, LogMsgType::Error);
        EXCEPT() << "Could not create PAL object. " << msg;
    }
    int rc; // additional return code, not used here
    mLicenseAvailable = palLicenseReadU(mPAL,
                                        CommonPaths::gamsLicenseFilePath().toStdString().c_str(),
                                        msg,
                                        &rc);
}

GamsLicenseInfo::~GamsLicenseInfo()
{
    if (mPAL) palFree(&mPAL);
}

int GamsLicenseInfo::solvers() const
{
    return mSolverInfo.solvers();
}

int GamsLicenseInfo::solverId(const QString &name) const
{
    return mSolverInfo.solverId(name);
}

QString GamsLicenseInfo::solverName(int id) const
{
    return mSolverInfo.solverName(id);
}

QMap<int, QString> GamsLicenseInfo::solverNames()
{
    return mSolverInfo.solverNames();
}

QMap<int, int> GamsLicenseInfo::solverIndices()
{
    return mSolverInfo.solverIndices();
}

QMap<int, QString> GamsLicenseInfo::modelTypeNames()
{
    return mSolverInfo.modelTypeNames();
}

bool GamsLicenseInfo::solverCapability(int solver, int modelType) const
{
    return mSolverInfo.solverCapability(solver,modelType);
}

QString GamsLicenseInfo::solverLicense(const QString &name, int id) const
{
    int days = -1;
    auto codes = solverCodes(id);
    if (!mLicenseAvailable)
        return "None";
    if (0 == palLicenseLevel(mPAL))
        return "Demo";
    if (5 == palLicenseLevel(mPAL))
        return "Community";
    if (palLicenseCheckSubX(mPAL,
                            name.toStdString().c_str(),
                            codes.toStdString().c_str(),
                            &days)) {
        if (palLicenseIsAcademic(mPAL))
            return "Community";
        return "Demo";
    }

    if (days == 0)
        return "Full";
    if (days > 0)
        return "Evaluation";
    return "Expired";
}

QStringList GamsLicenseInfo::licenseFromClipboard()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    QStringList licenseLines = clipboard->text().split('\n', QString::SkipEmptyParts);
    if (licenseLines.isEmpty())
        QStringList();

    for (int i=0; i<licenseLines.size(); ++i)
        licenseLines[i] = licenseLines[i].trimmed();
    return licenseLines;
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

QString GamsLicenseInfo::solverCodes(int solverId) const
{
    return mSolverInfo.solverCodes(solverId);
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
