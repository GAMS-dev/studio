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
#include "abstractmiroprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"

#include <QDir>

#ifdef _WIN64
#include "Windows.h"
#endif

namespace gams {
namespace studio {
namespace miro {

AbstractMiroProcess::AbstractMiroProcess(const QString &application, QObject *parent)
    : AbstractProcess(application, parent)
{
    // GAMS connections
    connect(&mProcess, &QProcess::stateChanged, this, &AbstractProcess::stateChanged);
    connect(&mProcess, &QProcess::readyReadStandardOutput, this, &AbstractMiroProcess::readStdOut);
    connect(&mProcess, &QProcess::readyReadStandardError, this, &AbstractMiroProcess::readStdErr);
    connect(&mProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(subProcessCompleted(int)));

    // MIRO connections
    connect(this, &AbstractMiroProcess::executeMiro, this, &AbstractMiroProcess::executeNext);
    connect(&mMiro, &QProcess::stateChanged, this, &AbstractMiroProcess::stateChanged);
    connect(&mMiro, &QProcess::readyReadStandardOutput, this, &AbstractMiroProcess::readStdOut);
    connect(&mMiro, &QProcess::readyReadStandardError, this, &AbstractMiroProcess::readStdErr);
    connect(&mMiro, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(completed(int)));
}

void AbstractMiroProcess::execute()
{
    QString app = nativeAppPath();
    if (!isAppAvailable(app))
        return;
    mProcess.setWorkingDirectory(workingDirectory());
#if defined(__unix__) || defined(__APPLE__)
    emit newProcessCall("Running:", appCall(app, parameters()));
    mProcess.start(app, parameters());
#else
    mProcess.setNativeArguments(parameters().join(" "));
    mProcess.setProgram(app);
    emit newProcessCall("Running:", appCall(app, parameters()));
    mProcess.start();
#endif
}

void AbstractMiroProcess::interrupt()
{
    if (mProcess.state() == QProcess::Running || mProcess.state() == QProcess::Starting)
        interruptIntern();
    if (mMiro.state() == QProcess::Running || mMiro.state() == QProcess::Starting)
        mMiro.terminate();
}

void AbstractMiroProcess::terminate()
{
    if (mProcess.state() == QProcess::Running || mProcess.state() == QProcess::Starting)
        interruptIntern(true);
    if (mMiro.state() == QProcess::Running || mMiro.state() == QProcess::Starting)
        mMiro.terminate();
}

QProcess::ProcessState AbstractMiroProcess::state() const
{
    if (mProcess.state() == QProcess::Running || mMiro.state() == QProcess::Running)
        return QProcess::Running;
    if (mProcess.state() == QProcess::Starting || mMiro.state() == QProcess::Starting)
        return QProcess::Starting;
    return QProcess::NotRunning;
}

void AbstractMiroProcess::setMiroPath(const QString &miroPath) {
    mMiroPath = miroPath;
}

QString AbstractMiroProcess::modelName() const
{
    return mModelName;
}

void AbstractMiroProcess::setModelName(const QString &modelFile)
{
    mModelName = modelFile;
}

QString AbstractMiroProcess::modelPath() const {
    return workingDirectory() + "/" + modelName() + ".gms";
}

void AbstractMiroProcess::readStdOut()
{
    if (mProcess.state() == QProcess::Running || mProcess.state() == QProcess::Starting)
        readStdChannel(mProcess, QProcess::StandardOutput);
    else if (mMiro.state() == QProcess::Running || mMiro.state() == QProcess::Starting)
        readStdChannel(mMiro, QProcess::StandardOutput);
}

void AbstractMiroProcess::readStdErr()
{
    if (mProcess.state() == QProcess::Running || mProcess.state() == QProcess::Starting)
        readStdChannel(mProcess, QProcess::StandardError);
    else if (mMiro.state() == QProcess::Running || mMiro.state() == QProcess::Starting)
        readStdChannel(mMiro, QProcess::StandardError);
}

void AbstractMiroProcess::completed(int exitCode)
{
    if (exitCode)
        SysLogLocator::systemLog()->append(QString("Could not run MIRO. Exit Code: %1")
                                           .arg(exitCode), LogMsgType::Error);
    emit finished(mProjectId, exitCode);
}

void AbstractMiroProcess::subProcessCompleted(int exitCode)
{
    if (exitCode) {
        SysLogLocator::systemLog()->append(QString("Could not run GAMS. Exit Code: %1")
                                           .arg(exitCode), LogMsgType::Error);
        emit finished(mProjectId, exitCode);
        return;
    }
    emit executeMiro();
}

void AbstractMiroProcess::executeNext()
{
    mMiro.setProgram(mMiroPath);
    mMiro.setWorkingDirectory(workingDirectory());
    mMiro.setProcessEnvironment(miroProcessEnvironment());
    emit newProcessCall("Running:", appCall(mMiroPath, mMiro.arguments()));
    mMiro.start();
}

QString AbstractMiroProcess::nativeAppPath()
{
    QString systemDir = CommonPaths::systemDir();
    if (systemDir.isEmpty())
        return QString();
    auto appPath = QDir(systemDir).filePath(AbstractProcess::nativeAppPath());
    return QDir::toNativeSeparators(appPath);
}

void AbstractMiroProcess::readStdChannel(QProcess &process, QProcess::ProcessChannel channel)
{
    mOutputMutex.lock();
    process.setReadChannel(channel);
    bool avail = process.bytesAvailable();
    mOutputMutex.unlock();

    while (avail) {
        mOutputMutex.lock();
        process.setReadChannel(channel);
        emit newStdChannelData(process.readLine().constData());
        avail = process.bytesAvailable();
        mOutputMutex.unlock();
    }
}

void AbstractMiroProcess::gamsInterrupt()
{
    QString pid = QString::number(mProcess.processId());
#ifdef _WIN64
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

}
}
}
