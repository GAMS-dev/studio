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
#include "abstractprocess.h"
#include "../commonpaths.h"

#include <QDir>
#include <QMetaType>

namespace gams {
namespace studio {

AbstractProcess::AbstractProcess(const QString &appName, QObject *parent)
    : QObject (parent),
      mProcess(this),
      mApplication(appName)
{
    if (!QMetaType::isRegistered(qMetaTypeId<QProcess::ProcessState>()))
        qRegisterMetaType<QProcess::ProcessState>();
    if (!QMetaType::isRegistered(qMetaTypeId<NodeId>()))
        qRegisterMetaType<NodeId>();
}

void AbstractProcess::setInputFile(const QString &file)
{
    mInputFile = file;
}

QString AbstractProcess::inputFile() const
{
    return mInputFile;
}

void AbstractProcess::interrupt()
{
    mProcess.kill();
}

void AbstractProcess::terminate()
{
    mProcess.kill();
}

void AbstractProcess::setWorkingDirectory(const QString &workingDirectory)
{
    mWorkingDirectory = workingDirectory;
}

QStringList AbstractProcess::parameters() const
{
    return mParameters;
}

void AbstractProcess::setParameters(const QStringList &parameters)
{
    mParameters = parameters;
}

QString AbstractProcess::workingDirectory() const
{
    return mWorkingDirectory;
}

void AbstractProcess::completed(int exitCode)
{
    emit finished(mGroupId, exitCode);
}

QString AbstractProcess::nativeAppPath()
{
    return QDir::toNativeSeparators(mApplication);
}

NodeId AbstractProcess::groupId() const
{
    return mGroupId;
}

void AbstractProcess::setGroupId(const NodeId &groupId)
{
    mGroupId = groupId;
}

int AbstractProcess::exitCode() const
{
    return mProcess.exitCode();
}

AbstractSingleProcess::AbstractSingleProcess(const QString &application, QObject *parent)
    : AbstractProcess(application, parent)
{
    connect(&mProcess, &QProcess::readyReadStandardOutput, this, &AbstractSingleProcess::readStdOut);
    connect(&mProcess, &QProcess::readyReadStandardError, this, &AbstractSingleProcess::readStdErr);
}

QProcess::ProcessState AbstractSingleProcess::state() const
{
    return mProcess.state();
}

void AbstractSingleProcess::readStdChannel(QProcess::ProcessChannel channel)
{
    mOutputMutex.lock();
    mProcess.setReadChannel(channel);
    bool avail = mProcess.bytesAvailable();
    mOutputMutex.unlock();

    while (avail) {
        mOutputMutex.lock();
        mProcess.setReadChannel(channel);
        emit newStdChannelData(mProcess.readLine().constData());
        avail = mProcess.bytesAvailable();
        mOutputMutex.unlock();
    }
}

void AbstractSingleProcess::readStdOut()
{
    readStdChannel(QProcess::StandardOutput);
}

void AbstractSingleProcess::readStdErr()
{
    readStdChannel(QProcess::StandardError);
}

AbstractGamsProcess::AbstractGamsProcess(const QString &application, QObject *parent)
    : AbstractSingleProcess(application, parent)
{
    connect(&mProcess, &QProcess::stateChanged, this, &AbstractProcess::stateChanged);
    connect(&mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(completed(int)));
}

QString AbstractGamsProcess::nativeAppPath()
{
    QString systemDir = CommonPaths::systemDir();
    if (systemDir.isEmpty())
        return QString();
    auto appPath = QDir(systemDir).filePath(AbstractProcess::nativeAppPath());
    return QDir::toNativeSeparators(appPath);
}

} // namespace studio
} // namespace gams
