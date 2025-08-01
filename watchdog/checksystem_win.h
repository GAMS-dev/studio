#ifndef CHECKSYSTEM_WIN_H
#define CHECKSYSTEM_WIN_H

#include <QStringList>
#include <QDateTime>
#include <Windows.h>
#include <QTemporaryDir>

class QProcess;

namespace gams {
namespace studio {


class EventLogEntry
{
public:
    EventLogEntry(const EVENTLOGRECORD* record);

    QStringList toStringList() const;

    // Optional getters
    quint32 recordId() const { return mRecordId; }
    quint32 eventId() const { return mEventId; }
    QString source() const { return mSource; }
    QString computer() const { return mComputer; }
    QDateTime timeGenerated() const { return mTimeGenerated; }

private:
    quint32 mRecordId;
    quint32 mEventId;
    quint8  mEventIdSev;
    quint16 mEventType;
    quint16 mCategory;
    QDateTime mTimeGenerated;
    QString mSource;
    QString mComputer;
    QList<QString> mStrings;
    bool mHasUserSid;
};


class CheckSystem
{
public:
    CheckSystem(QProcess * process);
    QString gatherInfo(const QTemporaryDir &tempDir);

private:
    QString captureStackTrace(const QTemporaryDir &tempDir);
    QString readEventLogEntries(const QString &source, const QTemporaryDir &tempDir);

private:
    QProcess *mProcess = nullptr;
};

} // namespace studio
} // namespace gams

#endif // CHECKSYSTEM_WIN_H
