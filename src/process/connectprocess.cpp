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
#include "connectprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

#include <QDir>
#include <commonpaths.h>

namespace gams {
namespace studio {

ConnectProcess::ConnectProcess(QObject *parent)
    : AbstractGamsProcess("GMSPython/python", parent)
{
    connect(this, &AbstractProcess::newProcessCall,
            [](const QString &text, const QString &call){ SysLogLocator::systemLog()->append(text + " " + call, LogMsgType::Info); });
    connect(this, &AbstractProcess::newStdChannelData,
            [](const QString &text){ SysLogLocator::systemLog()->append(text, LogMsgType::Info); });
}

void ConnectProcess::execute()
{
    QString app = nativeAppPath();
    if (!isAppAvailable(app))
        return;
    mProcess.setWorkingDirectory(workingDirectory());
    QStringList param;
    param << QDir::toNativeSeparators(CommonPaths::systemDir() + "/connectdriver.py");
    param << QDir::toNativeSeparators(CommonPaths::systemDir());
    param << parameters();
    setParameters(param);
    emit newProcessCall("Running:", appCall(app, parameters()));
    mProcess.start(app, parameters());
}

void ConnectProcess::stop(int waitMSec)
{
    mProcess.kill();
    if (waitMSec>0)
        mProcess.waitForFinished(waitMSec);
}

} // namespace studio
} // namespace gams
