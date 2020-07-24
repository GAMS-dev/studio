#ifndef GAMS_STUDIO_NEOS_NEOSPROCESS_H
#define GAMS_STUDIO_NEOS_NEOSPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {
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
///
/// \brief The NeosProcess class works in four steps:
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

signals:
    void neosStateChanged(AbstractProcess *proc, neos::NeosState progress);

protected slots:
    void rePing(const QString &value);
    void reVersion(const QString &value);
    void reSubmitJob(const int &jobNumber, const QString &jobPassword);
    void reGetJobStatus(const QString &value);
    void reGetCompletionCode(const QString &value);
    void reGetJobInfo(const QString &category, const QString &solverName, const QString &input, const QString &status,
                      const QString &completionCode);
    void reKillJob();
    void reGetIntermediateResultsNonBlocking(const QByteArray &data);
    void reGetFinalResultsNonBlocking(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);

private slots:
    void readSubStdOut();
    void readSubStdErr();
    void compileCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void unpackCompleted(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void setNeosState(NeosState newState);
    QString nativeAppPathX(QString appName);
    QStringList compileParameters();
    QStringList remoteParameters();
    QByteArray convertReferences(const QByteArray &data);
    QString rawData(QString runFile, QString parameters, QString workdir);
    QString rawKill();
    void startUnpacking();

    NeosManager *mManager;
    QString mOutPath;
    QString mJobNumber;
    QString mJobPassword;
    Priority mPrio;
    NeosState mNeosState = NeosIdle;

    QProcess mSubProc;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSPROCESS_H
