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
#include "gamsaboutprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"


#include <QDir>

namespace gams {
namespace studio {

GamsAboutProcess::GamsAboutProcess(QObject *parent)
    : QObject(parent)
    , mApplication("gams")
{
    connect(&mProcess, &QProcess::readyReadStandardOutput, this, &GamsAboutProcess::readStdOut);
    connect(&mProcess, &QProcess::readyReadStandardError, this, &GamsAboutProcess::readStdErr);
    connect(&mProcess, &QProcess::finished, this, &GamsAboutProcess::finished);
}

GamsAboutProcess::~GamsAboutProcess()
{
    disconnect();
    if (mProcess.state() == QProcess::Starting || mProcess.state() == QProcess::Running)
        mProcess.kill();
}

void GamsAboutProcess::execute()
{
    QStringList args({"/??", "lo=3", "procTreeMemMonitor=0", "curdir=\"" + mCurDir + "\""});
    QString app = nativeAppPath();

    QString msg = QString("GAMS licensing running: %1 %2").arg(application(), args.join(" "));
    SysLogLocator::systemLog()->append(msg, LogMsgType::Info);

#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(app, args);
#else
    mProcess.setNativeArguments(args.join(" "));
    mProcess.setProgram(app);
    mProcess.start();
#endif
}

QString GamsAboutProcess::content() const
{
    return mContent.join("\n");
}

QString GamsAboutProcess::logMessages() const
{
    return mLogMessages.join("\n");
}

QString GamsAboutProcess::application() const
{
    return mApplication;
}

void GamsAboutProcess::clearState()
{
    mContent.clear();
    mLogMessages.clear();
}

QString GamsAboutProcess::curDir() const
{
    return mCurDir;
}

void GamsAboutProcess::setCurDir(const QString &path)
{
    mCurDir = path;
}

bool GamsAboutProcess::isAppAvailable(const QString &app)
{
    if (!app.isEmpty() && QFileInfo::exists(app))
        return true;
    QString msg = QString("The %1 executable could not be found.").arg(application());
    SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
    return false;
}

void GamsAboutProcess::readStdOut()
{
    readStdChannel(QProcess::StandardOutput);
}

void GamsAboutProcess::readStdErr()
{
    readStdChannel(QProcess::StandardError);
}

void GamsAboutProcess::readStdChannel(QProcess::ProcessChannel channel)
{
    mOutputMutex.lock();
    mProcess.setReadChannel(channel);
    bool available = mProcess.bytesAvailable();
    mOutputMutex.unlock();

    while (available) {
        mOutputMutex.lock();
        mProcess.setReadChannel(channel);
        if (QProcess::ProcessChannel::StandardOutput == channel) {
            mContent << mProcess.readLine().constData();
        } else {
            mLogMessages << mProcess.readLine().constData();
        }
        available = mProcess.bytesAvailable();
        mOutputMutex.unlock();
    }
}

QString GamsAboutProcess::nativeAppPath()
{
    QString systemDir = CommonPaths::systemDir();
    if (systemDir.isEmpty())
        return QString();
    auto appPath = QDir(systemDir).filePath(QDir::toNativeSeparators(application()));
    return QDir::toNativeSeparators(appPath);
}

}
}
