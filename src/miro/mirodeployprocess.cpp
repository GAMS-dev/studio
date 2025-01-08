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
#include "mirodeployprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"
#include "mirocommon.h"

#include <QDir>

namespace gams {
namespace studio {
namespace miro {

MiroDeployProcess::MiroDeployProcess(QObject *parent)
    : AbstractMiroProcess("gams", parent)
{
}

QStringList MiroDeployProcess::defaultParameters() const
{
    return { QString("IDCJSON=\"%1/%2_io.json\"")
                .arg(MiroCommon::confDirectory(modelName()),
                     modelName().toLower()),
             "a=c" };
}

void MiroDeployProcess::setBaseMode(bool baseMode)
{
    mBaseMode = baseMode;
}

void MiroDeployProcess::setHypercubeMode(bool hcubeMode)
{
    mHypercubeMode = hcubeMode;
}

void MiroDeployProcess::setTestDeployment(bool testDeploy)
{
    mTestDeployment = testDeploy;
}

void MiroDeployProcess::setTargetEnvironment(MiroTargetEnvironment targetEnvironment)
{
    mTargetEnvironment = targetEnvironment;
}

void MiroDeployProcess::completed(int exitCode)
{
    QString msg;
    if (exitCode)
        msg = QString("Error deploying %1 (MIRO exit code %2) to location \n--- %3[DIR:\"%3\"]")
                .arg(MiroCommon::deployFileName(modelName()))
                .arg(exitCode).arg(workingDirectory());
    else if (!mTestDeployment)
        msg = QString("\n%1 successfully deployed to location \n--- %2[DIR:\"%2\"]")
                .arg(MiroCommon::deployFileName(modelName()), workingDirectory());
    emit newStdChannelData(msg.toLatin1());

    AbstractProcess::completed(exitCode);
}

QProcessEnvironment MiroDeployProcess::miroProcessEnvironment()
{
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert("PATH", QDir::toNativeSeparators(CommonPaths::systemDir()) +
                               ":" + environment.value("PATH"));
    auto miroBuild = mTestDeployment ? "false" : "true";
    auto miroTestDeploy = mTestDeployment ? "true" : "false";

    switch (mTargetEnvironment) {
    case MiroTargetEnvironment::SingleUser:
        environment.insert("MIRO_MODE", deployMode());
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "false");
        environment.insert("MIRO_BUILD_ARCHIVE", "false");
        environment.insert("MIRO_BUILD",miroBuild);
        environment.insert("MIRO_TEST_DEPLOY", miroTestDeploy);
        break;
    case MiroTargetEnvironment::LocalMultiUser:
        environment.insert("MIRO_MODE", deployMode());
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "true");
        environment.insert("MIRO_BUILD_ARCHIVE", "false");
        environment.insert("MIRO_BUILD", miroBuild);
        environment.insert("MIRO_TEST_DEPLOY", miroTestDeploy);
        break;
    case MiroTargetEnvironment::MultiUser:
        environment.insert("MIRO_MODE", deployMode());
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "true");
        environment.insert("MIRO_BUILD_ARCHIVE", "true");
        environment.insert("MIRO_BUILD", miroBuild);
        environment.insert("MIRO_TEST_DEPLOY", miroTestDeploy);
        break;
    }
    return environment;
}

QString MiroDeployProcess::deployMode()
{
    if (mBaseMode && mHypercubeMode)
        return "full";
    if (mBaseMode)
        return "base";
    if (mHypercubeMode)
        return "hcube";
    return QString();
}

}
}
}
