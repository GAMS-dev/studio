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
#ifndef GAMSLICENSEINFO_H
#define GAMSLICENSEINFO_H

#include <QMap>
#include <QString>
#include <QRegularExpression>

#include "solverconfiginfo.h"

struct palRec;
typedef struct palRec *palHandle_t;

namespace gams {
namespace studio {
namespace support {

///
/// \brief The GamsLicenseInfo class provides information about a
///        GAMS license as well as information about available
///        solvers.
///
class GamsLicenseInfo
{
public:
    GamsLicenseInfo();
    ~GamsLicenseInfo();

    int solvers() const;

    int solverId(const QString &name) const;

    QString solverName(int id) const;
    QMap<int, QString> solverNames();

    QMap<int, int> solverIndices();

    QMap<int, QString> modelTypeNames();

    bool solverCapability(int solver, int modelType) const;

    QString solverLicense(const QString &name, int id) const;

    QStringList licenseFromClipboard();

    QStringList licenseFromFile(const QString &fileName);

    bool isLicenseValid() const;

    bool isLicenseValid(const QStringList &license);

    QStringList gamsDataLocations();

    QStringList gamsConfigLocations();

    int localDistribVersion();

    QString localDistribVersionString();

    QString licesneFilePath() const;

    bool isAlpha() const;

    bool isBeta() const;

    bool isCurrentEvaluation(int evalDate);

    bool isCurrentMaintenance(int mainDate);

    bool isLicenseValidationSuccessful() const;

    bool isLicenseValidForPlatform() const;

    bool isGenericLicense() const;

    int evaluationLicenseData();

    int licenseData();

    int julian();

    int today();

private:
    QString solverCodes(int solverId) const;

    static int errorCallback(int count, const char *message);

    QStringList processLicenseData(const QString &data);

private:
    QString mLicenseFilePath;
    SolverConfigInfo mSolverInfo;
    palHandle_t mPAL;
    bool mLicenseAvailable = false;
    QRegularExpression mRegEx;
};

}
}
}

#endif // GAMSLICENSEINFO_H
