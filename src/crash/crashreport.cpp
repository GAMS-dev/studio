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
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLockFile>

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
    const QDateTime lastStart = lockInfo.lastModified();

    static QLockFile lock(lockPath); // removed by its destructor on clean exit only
    lock.setStaleLockTime(0);        // stale only if the owning process is gone
    if (!lock.tryLock(0) || !leftOver)
        return result;

    result.failed = true;
    result.zipFile = log.absolutePath() + QStringLiteral("/studio_report_")
                     + lastStart.toString(QStringLiteral("yyyyMMdd_HHmmss")) + QStringLiteral(".zip");

    const QString dumpDirectory = CrashHandler::dumpDirectory();
    if (!dumpDirectory.isEmpty()) {
        const QFileInfoList dumps =
            QDir(dumpDirectory).entryInfoList(dumpFilters(), QDir::Files, QDir::Time);
        if (!dumps.isEmpty() && dumps.first().lastModified() > lastStart)
            result.dumpFile = dumps.first().absoluteFilePath();
    }
    return result;
}

bool CrashReport::createZip(const PreviousRun& run, const QString& logFile, QObject* parent,
                            const std::function<void(bool)>& done)
{
    QStringList files;
    if (QFile::exists(logFile))
        files << logFile;
    if (!run.dumpFile.isEmpty())
        files << run.dumpFile;
    if (!run.failed || files.isEmpty())
        return false;

    const QString zipFile = run.zipFile;
    auto* subProc = new GmszipProcess(parent);
    QObject::connect(subProc, QOverload<int, QProcess::ExitStatus>::of(&GmszipProcess::finished), subProc,
                     [subProc, zipFile, done](int exitCode, QProcess::ExitStatus status) {
                         if (done)
                             done(status == QProcess::NormalExit && exitCode == 0 && QFile::exists(zipFile));
                         subProc->deleteLater();
                     });

    subProc->setWorkingDirectory(QFileInfo(zipFile).absolutePath());
    subProc->setParameters(QStringList{"-8", "-j", QFileInfo(zipFile).fileName()} + files);
    subProc->execute();
    return true;
}

} // namespace studio
} // namespace gams
