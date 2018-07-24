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
#include "gamsargmanager.h"
#include "gamsprocess.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#ifdef _WIN32
#include "windows.h"
#endif

namespace gams {
namespace studio {

GamsProcess::GamsProcess(QObject *parent)
    : AbstractProcess("gams", parent)
{
}

void GamsProcess::execute()
{
    mProcess.start(nativeAppPath(), mArgManager->getGamsParameters());
}

QString GamsProcess::aboutGAMS()
{
    QProcess process;
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QStringList args({"/??", "lo=3", "curdir=" + tempDir});
    QString appPath = nativeAppPath();
    if (appPath.isEmpty())
        return QString();
    process.start(appPath, args);
    QString about;
    if (process.waitForFinished()) {
        about = process.readAllStandardOutput();
    }
    QStringList lines = about.split('\n', QString::SkipEmptyParts, Qt::CaseInsensitive);
    lines.removeFirst();
    lines.removeLast();
    lines.removeLast();
    return lines.join("\n");
}

void GamsProcess::interrupt()
{
    QString pid = QString::number(mProcess.processId());
#ifdef _WIN32
    //IntPtr receiver;
    COPYDATASTRUCT cds;
    const char* msgText = "GAMS Message Interrupt";

    QString windowName("___GAMSMSGWINDOW___");
    windowName += pid;
    HWND receiver = FindWindowA(nullptr, windowName.toUtf8().constData());

    cds.dwData = (ULONG_PTR) 1;
    cds.lpData = (PVOID) msgText;
    cds.cbData = (DWORD) (strlen(msgText) + 1);

    SendMessageA(receiver, WM_COPYDATA, 0, (LPARAM)(LPVOID)&cds);
#else // Linux and Mac OS X
    QProcess proc;
    proc.setProgram("/bin/bash");
    QStringList args { "-c", "kill -2 " + pid};
    proc.setArguments(args);
    proc.start();
    proc.waitForFinished(-1);
#endif
}

void GamsProcess::stop()
{
    mProcess.kill();
}

GamsArgManager* GamsProcess::argManager()
{
    return mArgManager;
}

void GamsProcess::setArgManager(GamsArgManager *argManager)
{
    mArgManager = argManager;
}

} // namespace studio
} // namespace gams
