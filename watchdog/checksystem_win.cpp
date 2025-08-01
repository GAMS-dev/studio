#include "checksystem_win.h"
#include <QProcess>
#include <QDateTime>

namespace gams {
namespace studio {

enum Severity {
    Success       = 0x000,
    Informational = 0x001,
    Warning       = 0x010,
    Error         = 0x011,

    Customer      = 0x100
};

EventLogEntry::EventLogEntry(const EVENTLOGRECORD* record)
{
    mRecordId = record->RecordNumber;
    mEventId = record->EventID & 0xFFFF;
    mEventIdSev = ((record->EventID >> 30) & 0x03); // highest 2 bits are the severity
    mEventType = record->EventType;
    mCategory = record->EventCategory;
    mTimeGenerated = QDateTime::fromSecsSinceEpoch(record->TimeGenerated);

    // Extract variable-length strings
    LPCWSTR pSource = (LPCWSTR)((LPBYTE)record + sizeof(EVENTLOGRECORD));
    LPCWSTR pComputer = pSource + wcslen(pSource) + 1;
    mSource = QString::fromWCharArray(pSource);
    mComputer = QString::fromWCharArray(pComputer);
    mHasUserSid = record->UserSidLength > 0;

    // Extract event-specific strings (message inserts)
    if (record->NumStrings > 0) {
        LPCWSTR pStr = (LPCWSTR)((LPBYTE)record + record->StringOffset);
        for (DWORD i = 0; i < record->NumStrings; ++i) {
            mStrings.append(QString::fromWCharArray(pStr));
            pStr += wcslen(pStr) + 1;
        }
    }
}

QStringList EventLogEntry::toStringList() const
{
    QStringList list;
    list << QString("%1 - ").arg(mTimeGenerated.toString(Qt::ISODate));
    list << QString("RecordID: %1").arg(mRecordId);
    list << QString("EventID: %1").arg(mEventId);
    list << QString("EventID.sev: %1,").arg(mEventIdSev);
    list << QString("EventType: %1").arg(mEventType);
    list << QString("Category: %1").arg(mCategory);
    list << QString("Source: %1").arg(mSource);
    list << QString("Computer: %1").arg(mComputer);
    list << QString("User SID: %1").arg(mHasUserSid ? "present" : "not present");

    for (int i = 0; i < mStrings.size(); ++i) {
        list << QString("\n  [%1]: %2").arg(i).arg(mStrings[i]);
    }

    return list;
}

QString CheckSystem::gatherInfo(const QTemporaryDir &tempDir)
{
    QString res;
    res += readEventLogEntries("Application", tempDir);
    res += "\n" + captureStackTrace(tempDir);
    return res;
}

CheckSystem::CheckSystem(QProcess *process)
{
    mProcess = process;
}

QString CheckSystem::captureStackTrace(const QTemporaryDir &tempDir)
{
    QString res("\n==== Stack trace ====\n");

    if (!mProcess || mProcess->processId() <= 0) {
        qDebug() << "\nProcess already finished.";
        return QString();
    }

    QString outputFile = tempDir.path() + QString("/stacktrace_%1.txt") .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    QStringList arguments = {
        "-batch",
        "-ex", "thread apply all bt",
        "-p", QString::number(mProcess->processId())
    };

    QProcess gdb;
    gdb.setStandardOutputFile(outputFile);
    gdb.start("gdb", arguments);
    if (!gdb.waitForFinished(5000)) {
        qWarning() << "GDB timeout.";
        res += "\nGDB timeout.";
    } else {
        res += "\nStack trace written to:" + outputFile;
    }
    return res;
}

QString CheckSystem::readEventLogEntries(const QString &source, const QTemporaryDir &tempDir)
{
    QString res("\n==== Event LOG ====\n");

    HANDLE hEventLog = OpenEventLog(nullptr, const_cast<wchar_t*>(source.toStdWString().c_str()));
    if (hEventLog == nullptr) {
        res += &"\nOpenEventLog failed with " [GetLastError()];
        return res;
    }

    BYTE buffer[4096];
    DWORD bytesRead = 0;
    DWORD minBytesNeeded = 0;

    int count = 16;
    while (ReadEventLog(hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0,
                        buffer, sizeof(buffer), &bytesRead, &minBytesNeeded)) {
        DWORD offset = 0;
        while (offset < bytesRead) {
            EVENTLOGRECORD* record = (EVENTLOGRECORD*)&buffer[offset];
            EventLogEntry entry(record);
            if (entry.eventId() == 1000 || entry.eventId() == 1001)
                res += "\n" + entry.toStringList().join(", ");
            offset += record->Length;
        }
        res += "\n";
        if (--count <= 0) break;
    }

    if (GetLastError() != ERROR_HANDLE_EOF)
        res += &"\nReadEventLog failed with " [GetLastError()];

    CloseEventLog(hEventLog);
    return res;
}


} // namespace studio
} // namespace gams
