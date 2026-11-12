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
#include <QProcess>
#include <QRegularExpression>
#include <QTimer>

#if defined(Q_OS_LINUX)
#  include <sys/resource.h>
#endif

namespace gams {
namespace studio {

namespace {

struct DumpSearch
{
    QString file;           // found dump, may be empty
    bool temporary = false; // file was exported only for the zip and is removed afterwards
    QString info;           // where it was searched, if nothing was found
};

// Newest readable file in dir matching filters that is younger than lastStart
QString newestDump(const QString& dir, const QStringList& filters, const QDateTime& lastStart)
{
    const QFileInfoList dumps = QDir(dir).entryInfoList(filters, QDir::Files, QDir::Time);
    for (const QFileInfo& fi : dumps) {
        if (fi.lastModified() <= lastStart)
            break;
        if (fi.isReadable())
            return fi.absoluteFilePath();
    }
    return QString();
}

#if defined(Q_OS_LINUX)

// systemd-coredump may keep the core only in the journal or as compressed file readable via ACL only.
// coredumpctl handles all cases and writes the uncompressed core to target.
QString exportCoredump(const QString& comm, const QDateTime& lastStart, const QString& target, QString& info)
{
    QProcess proc;
    proc.start(QStringLiteral("coredumpctl"),
               {QStringLiteral("--no-pager"), QStringLiteral("--quiet"),
                QStringLiteral("--since=@") + QString::number(lastStart.toSecsSinceEpoch()),
                QStringLiteral("--output=") + target, QStringLiteral("dump"), comm});
    if (!proc.waitForStarted(3000)) {
        info = QStringLiteral("coredumpctl not available");
        return QString();
    }
    if (!proc.waitForFinished(30000)) {
        proc.kill();
        proc.waitForFinished(1000);
        QFile::remove(target);
        info = QStringLiteral("coredumpctl timed out");
        return QString();
    }
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0 || !QFile::exists(target)) {
        QFile::remove(target);
        info = QStringLiteral("coredumpctl: ") + QString::fromLocal8Bit(proc.readAllStandardError()).simplified();
        return QString();
    }
    return target;
}

QString coreLimitHint()
{
    struct rlimit limit;
    if (getrlimit(RLIMIT_CORE, &limit) == 0 && limit.rlim_cur == 0)
        return QStringLiteral(" Core dumps are disabled by 'ulimit -c 0'.");
    return QString();
}

#endif

// Locates the crash data written by CrashHandler / WER (Windows) or the OS (macOS, Linux)
DumpSearch findDump(const QDateTime& lastStart, const QString& workDir)
{
    DumpSearch res;
    if (!lastStart.isValid()) {
        res.info = QStringLiteral("No start time of the last session to identify its dump.");
        return res;
    }
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
#  if defined(Q_OS_WIN)
    const QStringList filters{QStringLiteral("*.dmp")};
#  else
    const QStringList filters{QFileInfo(QCoreApplication::applicationFilePath()).completeBaseName()
                              + QStringLiteral("*.ips")};
#  endif
    Q_UNUSED(workDir)
    const QString dir = CrashHandler::dumpDirectory();
    res.file = newestDump(dir, filters, lastStart);
    if (res.file.isEmpty())
        res.info = QStringLiteral("No dump %1 found in %2.").arg(filters.join(", "), QDir::toNativeSeparators(dir));
#elif defined(Q_OS_LINUX)
    // the kernel truncates the process name (comm) to 15 characters
    const QString comm = QFileInfo(QCoreApplication::applicationFilePath()).fileName().left(15);
    QFile patternFile(QStringLiteral("/proc/sys/kernel/core_pattern"));
    const QString pattern = patternFile.open(QIODevice::ReadOnly)
                                ? QString::fromLocal8Bit(patternFile.readAll()).trimmed() : QString();

    if (pattern.contains(QStringLiteral("systemd-coredump"))) {
        res.file = newestDump(QStringLiteral("/var/lib/systemd/coredump"), {"core." + comm + ".*"}, lastStart);
        if (res.file.isEmpty()) {
            QString why;
            res.file = exportCoredump(comm, lastStart, workDir + "/core." + comm, why);
            res.temporary = !res.file.isEmpty();
            if (res.file.isEmpty())
                res.info = QStringLiteral("No core dump found by systemd-coredump (%1).").arg(why) + coreLimitHint();
        }
    } else if (pattern.contains(QStringLiteral("apport"))) {
        // apport stores cores of programs that are not part of a distribution package in its coredump folder
        res.file = newestDump(QStringLiteral("/var/lib/apport/coredump"), {"core.*" + comm + ".*"}, lastStart);
        if (res.file.isEmpty())
            res.file = newestDump(QStringLiteral("/var/crash"), {"*" + comm + "*.crash"}, lastStart);
        if (res.file.isEmpty())
            res.info = QStringLiteral("No core dump found by apport in /var/lib/apport/coredump or /var/crash.")
                       + coreLimitHint();
    } else if (pattern.startsWith(QLatin1Char('|'))) {
        res.info = QStringLiteral("Core dumps are passed to an unsupported handler: ") + pattern;
    } else if (!pattern.isEmpty()) {
        // plain file name pattern, relative to the working directory of the crashed process
        QString filePattern = pattern;
        filePattern.replace(QRegularExpression(QStringLiteral("%.")), QStringLiteral("*"));
        const QFileInfo fi(QDir::current(), filePattern);
        res.file = newestDump(fi.path(), {fi.fileName()}, lastStart);
        if (res.file.isEmpty())
            res.info = QStringLiteral("No core dump %1 found in %2.").arg(fi.fileName(), fi.path()) + coreLimitHint();
    } else {
        res.info = QStringLiteral("Cannot read /proc/sys/kernel/core_pattern to locate the core dump.");
    }
#else
    Q_UNUSED(workDir)
    res.info = QStringLiteral("Locating dumps is not supported on this platform.");
#endif
    return res;
}

QString gLogFile; // set by checkPreviousRun()

// Core dumps can be several hundred MB
constexpr int kZipTimeout = 120000;

// Runs gmszip and waits for it (max. kZipTimeout ms).
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
    QTimer::singleShot(kZipTimeout, &loop, &QEventLoop::quit);

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
    gLogFile = logFile;
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

    const DumpSearch dump = findDump(lastStart, log.absolutePath());
    result.dumpFile = dump.file;
    result.dumpInfo = dump.info;
    if (!dump.file.isEmpty())
        files << dump.file;

    // Get the optimal date/time for the zip file name
    // (an exported dump has the time of the export)
    const bool useLog = result.dumpFile.isEmpty() || dump.temporary;
    QDateTime crashTime = useLog ? log.lastModified() : QFileInfo(result.dumpFile).lastModified();
    if (!crashTime.isValid()) {
        crashTime = useLog ? log.birthTime() : QFileInfo(result.dumpFile).birthTime();
        if (!crashTime.isValid())
            crashTime = QDateTime::currentDateTime();
    }
    result.zipFile = log.absolutePath() + QStringLiteral("/studio_dump_")
                     + crashTime.toString(QStringLiteral("yyyyMMdd_HHmmss")) + QStringLiteral(".zip");

    result.zipped = !files.isEmpty() && zipFiles(result.zipFile, log.absolutePath(), files);
    if (dump.temporary)
        QFile::remove(dump.file);
    return result;
}

QString CrashReport::zipSessionDumps()
{
    if (gLogFile.isEmpty())
        return QString();
    QStringList files;
    const QStringList dumps = CrashHandler::sessionDumps();
    for (const QString& dump : dumps) {
        if (QFile::exists(dump))
            files << dump;
    }
    if (files.isEmpty())
        return QString();

    const QFileInfo log(gLogFile);
    const QString zipFile = log.absolutePath() + QStringLiteral("/studio_dump_")
                            + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))
                            + QStringLiteral(".zip");
    // written before zipping to be part of the zipped log
    qInfo() << "CrashReport: writing" << files.size() << "dump(s) of this session to" << QDir::toNativeSeparators(zipFile);
    if (log.exists())
        files.prepend(log.fileName());
    if (!zipFiles(zipFile, log.absolutePath(), files)) {
        qWarning() << "CrashReport: could not create" << QDir::toNativeSeparators(zipFile);
        return QString();
    }
    return zipFile;
}

} // namespace studio
} // namespace gams
