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
#include "miroprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"
#include "mirocommon.h"

#include <QDir>

namespace gams {
namespace studio {
namespace miro {

MiroProcess::MiroProcess(QObject *parent)
    : AbstractMiroProcess("gams", parent)
{
}

void MiroProcess::setMiroMode(MiroMode mode)
{
    mMiroMode = mode;
}

void MiroProcess::execute()
{
    setupMiroEnvironment();
    if (mSkipModelExecution)
        emit executeMiro();
    else
        AbstractMiroProcess::execute();
}

QStringList MiroProcess::defaultParameters() const
{
    return { QString("IDCGenerateJSON=\"%1/%2_io.json\"")
                .arg(MiroCommon::confDirectory(modelName()),
                     modelName().toLower()),
             QString("IDCGenerateGDX=\"%1/default.gdx\"")
                .arg(MiroCommon::dataDirectory(modelName())) };
}

QProcessEnvironment MiroProcess::miroProcessEnvironment()
{
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert("PATH", QDir::toNativeSeparators(CommonPaths::systemDir()) +
                               ":" + environment.value("PATH"));

    switch (mMiroMode) {
    case MiroMode::Base:
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "false");
        environment.insert("MIRO_MODE", "base");
        break;
    case MiroMode::Configuration:
        environment.insert("MIRO_DEV_MODE", "true");
        environment.insert("MIRO_MODEL_PATH", modelPath());
        environment.insert("MIRO_USE_TMP", "false");
        environment.insert("MIRO_MODE", "config");
        break;
    }
    return environment;
}

void MiroProcess::setupMiroEnvironment()
{
    QDir confDir(workingDirectory() + "/" + MiroCommon::confDirectory(modelName()));
    if (!confDir.exists() && !confDir.mkpath(confDir.path())) {
        SysLogLocator::systemLog()->append(QString("Could not create the configuration folder: %1")
                                           .arg(confDir.path()), LogMsgType::Error);
    }

    QDir dataDir(workingDirectory() + "/" + MiroCommon::dataDirectory(modelName()));
    if (!dataDir.exists() && !dataDir.mkpath(dataDir.path())) {
        SysLogLocator::systemLog()->append(QString("Could not create the data folder: %1")
                                           .arg(dataDir.path()), LogMsgType::Error);
    }
}

}
}
}
