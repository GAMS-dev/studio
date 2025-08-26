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
#include "gamsgetkeyprocess.h"
#include "commandlineparser.h"
#include "commonpaths.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

#include <QDir>

namespace gams {
namespace studio {

GamsGetKeyProcess::GamsGetKeyProcess(QObject *parent)
    : QObject(parent)
    , mApplication("gamsgetkey")
{
    connect(&mProcess, &QProcess::readyReadStandardOutput, this, &GamsGetKeyProcess::readStdOut);
    connect(&mProcess, &QProcess::readyReadStandardError, this, &GamsGetKeyProcess::readStdErr);
    connect(&mProcess, &QProcess::finished, this, &GamsGetKeyProcess::finished);
}

GamsGetKeyProcess::~GamsGetKeyProcess()
{
    disconnect();
    if (mProcess.state() == QProcess::Starting || mProcess.state() == QProcess::Running)
        mProcess.kill();
}

void GamsGetKeyProcess::execute()
{
    if (mProcess.state() != QProcess::NotRunning)
        return;
    if (mAlpId.isEmpty()) {
        mLogMessages << "No Access Code given. Unable to fetch GAMS license.";
        emit finished(1);
        return;
    }
    QStringList args({mAlpId});
    if (!mCheckoutDuration.isEmpty()) {
        args << "-c" << mCheckoutDuration;
    }
    if (!mOnPremSever.isEmpty()) {
        args << "-s" << mOnPremSever;
        args << "-g" << mOnPremCertPath+"/gamslice.crt";
    }
    auto app = nativeAppPath();
    if (app.isEmpty()) {
        mLogMessages << "Could not locate gamsgetkey.";
        emit finished(1);
        return;
    }
    mVerboseOutput = !CommandLineParser::networkLog().isEmpty();
    if (mVerboseOutput) {
        args << "-v";
    }
    mLogMessages << "Running: " + mApplication + " " + args.join(" ");

#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(app, args);
#else
    mProcess.setNativeArguments(args.join(" "));
    mProcess.setProgram(app);
    mProcess.start();
#endif
}

QString GamsGetKeyProcess::content() const
{
    return mContent.join("\n");
}

QString GamsGetKeyProcess::logMessages() const
{
    return mLogMessages.join("\n");
}

void GamsGetKeyProcess::clearState()
{
    mContent.clear();
    mLogMessages.clear();
}

void GamsGetKeyProcess::readStdChannel(QProcess::ProcessChannel channel)
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

void GamsGetKeyProcess::readStdOut()
{
    readStdChannel(QProcess::StandardOutput);
}

void GamsGetKeyProcess::readStdErr()
{
    readStdChannel(QProcess::StandardError);
}

QString GamsGetKeyProcess::application() const
{
    return mApplication;
}

bool GamsGetKeyProcess::isAppAvailable(const QString &app)
{
    if (!app.isEmpty() && QFileInfo::exists(app))
        return true;
    QString msg = QString("The %1 executable could not be found.").arg(application());
    SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
    return false;
}

QString GamsGetKeyProcess::alpId() const
{
    return mAlpId;
}

void GamsGetKeyProcess::setAlpId(const QString &id)
{
    mAlpId = id;
}

QString GamsGetKeyProcess::checkoutDuration() const
{
    return mCheckoutDuration;
}

void GamsGetKeyProcess::setCheckoutDuration(const QString &duration)
{
    mCheckoutDuration = duration;
}

QString GamsGetKeyProcess::onPremSever() const
{
    return mOnPremSever;
}

void GamsGetKeyProcess::setOnPremSever(const QString &address)
{
    mOnPremSever = address;
}

QString GamsGetKeyProcess::onPremCertPath() const
{
    return mOnPremCertPath;
}

void GamsGetKeyProcess::setOnPremCertPath(const QString &newOnPremCertPath)
{
    mOnPremCertPath = newOnPremCertPath;
}

bool GamsGetKeyProcess::verboseOutput() const
{
    return mVerboseOutput;
}

void GamsGetKeyProcess::setVerboseOutput(bool enable)
{
    mVerboseOutput = enable;
}

QString GamsGetKeyProcess::nativeAppPath()
{
    QString systemDir = CommonPaths::systemDir();
    if (systemDir.isEmpty())
        return QString();
    auto appPath = QDir(systemDir).filePath(QDir::toNativeSeparators(mApplication));
    return QDir::toNativeSeparators(appPath);
}

void GamsGetKeyProcess::writeLogToFile(const QString &data)
{
    auto log = CommandLineParser::networkLog();
    if (log.isEmpty())
        return;
    QFile logFile(log);
    if (!logFile.open(QIODevice::Append | QIODevice::Text))
        return;
    logFile.write(data.toLatin1());
    logFile.write("\n");
    logFile.close();
}

}
}
