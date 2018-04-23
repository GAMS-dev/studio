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
#include "abstractprocess.h"
#include "gamspaths.h"

#include <QDir>

namespace gams {
namespace studio {

AbstractProcess::AbstractProcess(QObject *parent)
    : QObject (parent),
      mSystemDir(GAMSPaths::systemDir()),
      mProcess(this)
{
    connect(&mProcess, &QProcess::stateChanged, this, &AbstractProcess::stateChanged);
    connect(&mProcess, &QProcess::readyReadStandardOutput, this, &AbstractProcess::readStdOut);
    connect(&mProcess, &QProcess::readyReadStandardError, this, &AbstractProcess::readStdErr);
    connect(&mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(completed(int)));
}

QString AbstractProcess::nativeAppPath(const QString &dir, const QString &app)
{
    auto appPath = QDir(dir).filePath(app);
    return QDir::toNativeSeparators(appPath);
}

void AbstractProcess::setSystemDir(const QString &systemDir)
{
    mSystemDir = systemDir;
}

QString AbstractProcess::systemDir() const
{
    return mSystemDir;
}

void AbstractProcess::setInputFile(const QString &file)
{
    mInputFile = file;
}

QString AbstractProcess::inputFile() const
{
    return mInputFile;
}

QProcess::ProcessState AbstractProcess::state() const
{
    return mProcess.state();
}

void AbstractProcess::completed(int exitCode)
{
    emit finished(this, exitCode);
}

void AbstractProcess::readStdOut()
{
    readStdChannel(QProcess::StandardOutput);
}

void AbstractProcess::readStdErr()
{
    readStdChannel(QProcess::StandardError);
}

void AbstractProcess::readStdChannel(QProcess::ProcessChannel channel)
{
    mOutputMutex.lock();
    mProcess.setReadChannel(channel);
    bool avail = mProcess.bytesAvailable();
    mOutputMutex.unlock();

    while (avail) {
        mOutputMutex.lock();
        mProcess.setReadChannel(channel);
        emit newStdChannelData(channel, mProcess.readLine());
        avail = mProcess.bytesAvailable();
        mOutputMutex.unlock();
    }
}

} // namespace studio
} // namespace gams
