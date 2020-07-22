#ifndef GAMS_STUDIO_NEOS_NEOSPROCESS_H
#define GAMS_STUDIO_NEOS_NEOSPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {
namespace neos {

enum NeosState {
    NeosNoState,
    NeosCompile,
    NeosMonitor,
    NeosGetResult,
    NeosUnpack,
    NeosFinished
};

enum Priority {
    prioShort,
    prioLong
};

class NeosManager;

class NeosProcess final : public AbstractGamsProcess
{
    Q_OBJECT

public:
    NeosProcess(QObject *parent = nullptr);
    ~NeosProcess();
    void setGmsFile(QString gmsFile);
    void setPriority(Priority prio) { mPrio = prio; }

    void execute() override;
    void interrupt() override;

signals:
    void neosStateChanged(AbstractProcess *proc, neos::NeosState progress);

protected:
    void readStdChannel(QProcess::ProcessChannel channel) override;

protected slots:
    void completed(int exitCode) override;
    void rePing(const QString &value);
    void reVersion(const QString &value);
    void reSubmitJob(const int &jobNumber, const QString &jobPassword);
    void reGetJobStatus(const QString &value);
    void reGetCompletionCode(const QString &value);
    void reGetJobInfo(const QString &category, const QString &solverName, const QString &input, const QString &status,
                      const QString &completionCode);
    void reKillJob();
    void reGetIntermediateResults(const QByteArray &data);
    void reGetFinalResultsNonBlocking(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);

private slots:
    void readSubStdOut();
    void readSubStdErr();
    void compileStateChanged(QProcess::ProcessState state);
    void compileCompleted(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void compile();

    void setNeosState(NeosState newState);
    bool prepareCompileParameters();
    bool prepareKill(QStringList &tempParams);
    void scanForCredentials(const QByteArray &data);
    QString rawData(QString runFile, QString parameters, QString workdir);
    QString rawKill();

    NeosManager *mManager;
    QString mRunFile;
    QString mJobNumber;
    QString mJobPassword;
    Priority mPrio;
    NeosState mNeosState = NeosNoState;

    QProcess *mSubProc;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSPROCESS_H
