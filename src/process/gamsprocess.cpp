/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include "gamsprocess.h"

#include <QStandardPaths>
#include <QDir>

namespace gams {
namespace studio {

GamsProcess::GamsProcess(QObject *parent)
    : AbstractGamsProcess("gams", parent)
{
}

void GamsProcess::execute()
{
    mProcess.setWorkingDirectory(workingDirectory());
#if defined(__unix__) || defined(__APPLE__)
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start(nativeAppPath(), parameters());
#else
    mProcess.setNativeArguments(parameters().join(" "));
    mProcess.setProgram(nativeAppPath());
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start();
#endif
}

void GamsProcess::interrupt()
{
    interruptIntern();
}

QString GamsProcess::aboutGAMS()
{
    QProcess process;
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QStringList args({"/??", "lo=3", "procTreeMemMonitor=0", "curdir=\"" + tempDir + "\""});
    QString appPath = nativeAppPath();
    if (appPath.isEmpty())
        return QString();

#if defined(__unix__) || defined(__APPLE__)
    process.start(nativeAppPath(), args);
#else
    process.setNativeArguments(args.join(" "));
    process.setProgram(nativeAppPath());
    process.start();
#endif

    QString about;
    if (process.waitForFinished()) {
        about = process.readAllStandardOutput();
    }
    QStringList lines = about.split('\n', Qt::SkipEmptyParts, Qt::CaseInsensitive);
    if (lines.size() >= 3) {
        lines.removeFirst();
        lines.removeLast();
        lines.removeLast();
    }

    return lines.join("\n");
}

} // namespace studio
} // namespace gams
