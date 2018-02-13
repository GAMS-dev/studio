/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "gamspaths.h"
#include "filegroupcontext.h"
#include "logcontext.h"
#include <QDebug>
#include <QDir>

#ifdef _WIN32
#include "windows.h"
#endif

namespace gams {
namespace studio {

const QString GamsProcess::App = "gams";

GamsProcess::GamsProcess(QObject *parent)
    : AbstractProcess(parent)
{
}

QString GamsProcess::app()
{
    return App;
}

QString GamsProcess::nativeAppPath()
{
    return AbstractProcess::nativeAppPath(mSystemDir, App);
}

void GamsProcess::setWorkingDir(const QString &workingDir)
{
    mWorkingDir = workingDir;
}

QString GamsProcess::workingDir() const
{
    return mWorkingDir;
}

void GamsProcess::setContext(FileGroupContext *context)
{
    mContext = context;
}

FileGroupContext* GamsProcess::context()
{
    return mContext;
}

LogContext*GamsProcess::logContext() const
{
    return mContext ? mContext->logContext() : nullptr;
}

void GamsProcess::execute()
{
    mProcess.setWorkingDirectory(mWorkingDir);
    QStringList args({QDir::toNativeSeparators(mInputFile)});
    if (!mCommandLineStr.isEmpty()) {
        QStringList paramList = mCommandLineStr.split(QRegExp("\\s+"));
        args.append(paramList);
    }
    args << "lo=3" << "ide=1" << "er=99" << "errmsg=1" << QString("o=%1.lst").arg(QFileInfo(mInputFile).baseName());
    mProcess.start(nativeAppPath(), args);
}

QString GamsProcess::aboutGAMS()
{
    QProcess process;
    QStringList args({"?", "lo=3"});
    process.start(AbstractProcess::nativeAppPath(GAMSPaths::systemDir(), App), args);
    QString about;
    if (process.waitForFinished()) {
        about = process.readAllStandardOutput();
    }
    return about;
}

QString GamsProcess::commandLineStr() const
{
    return mCommandLineStr;
}

void GamsProcess::setCommandLineStr(const QString &commandLineStr)
{
    mCommandLineStr = commandLineStr;
}

void GamsProcess::interrupt()
{
#ifdef _WIN32
    //IntPtr receiver;
    COPYDATASTRUCT cds;
    const char* msgText = "GAMS Message Interrupt";

    qint64 pid = mProcess.processId();
    QString windowName("___GAMSMSGWINDOW___");
    windowName += QString::number(pid);
    LPCTSTR windowNameL = windowName.toStdWString().c_str();
    HWND receiver = FindWindow(nullptr, windowNameL);

    cds.dwData = (ULONG_PTR) 1;
    cds.lpData = (PVOID) msgText;
    cds.cbData = (DWORD) (strlen(msgText) + 1);

    SendMessage(receiver, WM_COPYDATA, 0, (LPARAM)(LPVOID)&cds);
#elif __APPLE__
    //TODO: implement
#elif __linux__
    mProcess.terminate();
#elif __unix__
    //TODO: implement
#else
    //TODO: implement
#endif
}

} // namespace studio
} // namespace gams
