#ifndef CHECKSYSTEM_MAC_H
#define CHECKSYSTEM_MAC_H

#include <QStringList>
#include <QDateTime>
#include <QTemporaryDir>

class QProcess;

namespace gams {
namespace studio {


class CheckSystem
{
public:
    CheckSystem(QProcess * process) : mProcess(process) {}
    QString gatherInfo(const QTemporaryDir &tempDir) { return QString(); }

private:
    QString captureStackTrace(const QTemporaryDir &tempDir) { return QString(); }
    QString readEventLogEntries(const QString &source, const QTemporaryDir &tempDir) { return QString(); }

private:
    QProcess *mProcess = nullptr;
};

} // namespace studio
} // namespace gams

#endif // CHECKSYSTEM_MAC_H
