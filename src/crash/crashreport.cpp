/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "crashreport.h"
#include "crashhandler.h"
#include "process/gmszipprocess.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QLockFile>
#include <QTimer>

namespace gams {
namespace studio {

namespace {

// Crash data written by CrashHandler / WER (Windows) or the OS (macOS, Linux)
QStringList dumpFilters()
{
#if defined(Q_OS_WIN)
    return {QStringLiteral("*.dmp")};
#else
    const QString exe = QFileInfo(QCoreApplication::applicationFilePath()).completeBaseName();
#  if defined(Q_OS_MACOS)
    return {exe + QStringLiteral("*.ips")};
#  else
    return {QStringLiteral("core.") + exe + QStringLiteral(".*")};
#  endif
#endif
}

// Runs gmszip and waits for it (max. 10 s).
bool zipFiles(const QString& zipFile, const QString& workDir, const QStringList& files)
{
    GmszipProcess zipProc;
    QEventLoop loop;
    bool finished = false;
    bool ok = false;
    QObject::connect(&zipProc, QOverload<int, QProcess::ExitStatus>::of(&GmszipProcess::finished), &loop,
                     [&](int exitCode, QProcess::ExitStatus status) {
                         finished = true;
                         ok = status == QProcess::NormalExit && exitCode == 0;
                         loop.quit();
                     });
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);

    // -j: store without paths (the dump lives in another folder)
    zipProc.setWorkingDirectory(workDir);
    zipProc.setParameters(QStringList{"-8", "-j", zipFile} + files);
    zipProc.execute();
    if (!finished)
        loop.exec();
    return ok && QFile::exists(zipFile);
}

} // namespace

CrashReport::PreviousRun CrashReport::checkPreviousRun(const QString& logFile)
{
    PreviousRun result;
    const QFileInfo log(logFile);
    QDir().mkpath(log.absolutePath());
    const QString lockPath = log.absolutePath() + QLatin1Char('/') + log.completeBaseName()
                             + QStringLiteral(".lock");

    // A lock file left over from a dead process means the last run did not exit cleanly.
    const QFileInfo lockInfo(lockPath);
    const bool leftOver = lockInfo.exists();
    QDateTime lastStart = lockInfo.lastModified();
    if (!lastStart.isValid())
        lastStart = lockInfo.birthTime();
    if (!lastStart.isValid())
        qWarning() << "CrashReport: no timestamp for" << lockPath;

    static QLockFile lock(lockPath); // removed by its destructor on clean exit only
    lock.setStaleLockTime(0);        // stale only if the owning process is gone
    if (!lock.tryLock(0) || !leftOver)
        return result;

    result.failed = true;
    QStringList files;
    if (log.exists())
        files << log.fileName();

    const QString dumpDirectory = CrashHandler::dumpDirectory();
    if (!dumpDirectory.isEmpty() && lastStart.isValid()) {
        const QFileInfoList dumps =
            QDir(dumpDirectory).entryInfoList(dumpFilters(), QDir::Files, QDir::Time);
        if (!dumps.isEmpty() && dumps.first().lastModified() > lastStart) {
            result.dumpFile = dumps.first().absoluteFilePath();
            files << result.dumpFile;
        }
    }

    // Get the optimal date/time for the zip file name
    QDateTime crashTime = result.dumpFile.isEmpty() ? log.lastModified()
                                                    : QFileInfo(result.dumpFile).lastModified();
    if (!crashTime.isValid()) {
        crashTime = result.dumpFile.isEmpty() ? log.birthTime()
                                              : QFileInfo(result.dumpFile).birthTime();
        if (!crashTime.isValid())
            crashTime = QDateTime::currentDateTime();
    }
    result.zipFile = log.absolutePath() + QStringLiteral("/studio_report_")
                     + crashTime.toString(QStringLiteral("yyyyMMdd_HHmmss")) + QStringLiteral(".zip");

    result.zipped = !files.isEmpty() && zipFiles(result.zipFile, log.absolutePath(), files);
    return result;
}

} // namespace studio
} // namespace gams
