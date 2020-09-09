#ifndef GAMS_STUDIO_NEOS_NEOSPROCESS_H
#define GAMS_STUDIO_NEOS_NEOSPROCESS_H

#include "process.h"
#include <QTimer>

namespace gams {
namespace studio {

class GmsunzipProcess;

namespace neos {

enum NeosState {
    NeosIdle,
    Neos1Compile,
    Neos2Monitor,
    Neos3GetResult,
    Neos4Unpack,
};

enum Priority {
    prioShort,
    prioLong
};

class NeosManager;

/// \brief The NeosProcess controls all steps to run a job on NEOS
/// This class works in four process steps:
/// 1. compile gms on the local machine
/// 2. monitor the remote job
/// 3. get result file
/// 4. unpack result file and finish
class NeosProcess final : public AbstractGamsProcess
{
    Q_OBJECT
public:
    NeosProcess(QObject *parent = nullptr);
    ~NeosProcess() override;
    void setPriority(Priority prio) { mPrio = prio; }

    void execute() override;
    void interrupt() override;
    void setParameters(const QStringList &parameters) override;
    QProcess::ProcessState state() const override;

signals:
    void neosStateChanged(AbstractProcess *proc, neos::NeosState progress);

protected slots:
    void rePing(const QString &value);
    void reVersion(const QString &value);
    void reSubmitJob(const int &jobNumber, const QString &jobPassword);
    void reGetJobStatus(const QString &status);
    void reGetCompletionCode(const QString &code);
    void reGetJobInfo(const QStringList &info);
    void reKillJob(const QString &text);
    void reGetIntermediateResultsNonBlocking(const QByteArray &data);
    void reGetFinalResultsNonBlocking(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);

private slots:
    void pullStatus();
    void compileCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void unpackCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void sslErrors(const QStringList &errors);
    void parseUnzipStdOut(const QByteArray &data);
    void unzipStateChanged(QProcess::ProcessState newState);

private:
    void setNeosState(NeosState newState);
    QStringList compileParameters();
    QStringList remoteParameters();
    QByteArray convertReferences(const QByteArray &data);
    void startUnpacking();

    NeosManager *mManager;
    QString mOutPath;
    QString mJobNumber;
    QString mJobPassword;
    Priority mPrio;
    NeosState mNeosState = NeosIdle;
    QTimer mPullTimer;

    GmsunzipProcess *mSubProc = nullptr;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSPROCESS_H
