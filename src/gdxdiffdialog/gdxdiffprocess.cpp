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
#include "gdxdiffprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

#include <QDir>

namespace gams {
namespace studio {
namespace gdxdiffdialog {

GdxDiffProcess::GdxDiffProcess(QObject *parent)
    : AbstractGamsProcess("gdxdiff", parent)
{
    connect(this, &AbstractProcess::newProcessCall,
            [](const QString &text, const QString &call){ SysLogLocator::systemLog()->append(text + " " + call, LogMsgType::Info); });
    connect(this, &AbstractProcess::newStdChannelData, this, &GdxDiffProcess::appendSystemLog);
}

void GdxDiffProcess::execute()
{
    mProcess.setWorkingDirectory(workingDirectory());
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start(nativeAppPath(), parameters());
}

QString GdxDiffProcess::diffFile() const
{
    return mDiffFile;
}

void GdxDiffProcess::stop(int waitMSec)
{
    mProcess.kill();
    if (waitMSec>0)
        mProcess.waitForFinished(waitMSec);
}

void GdxDiffProcess::appendSystemLog(const QString &text)
{
    SysLogLocator::systemLog()->append(text, LogMsgType::Info);
    if (text.contains("Output:")) {
        mDiffFile = text.split("Output:").last().trimmed();
        if (QFileInfo(mDiffFile).isRelative())
            mDiffFile = QDir::cleanPath(workingDirectory() + QDir::separator() + mDiffFile);
    }
}

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams
