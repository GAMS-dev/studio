#ifndef GAMS_STUDIO_NEOS_NEOSPROCESS_H
#define GAMS_STUDIO_NEOS_NEOSPROCESS_H

#include "process.h"
#include <QTimer>

namespace gams {
namespace studio {
namespace neos {

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
    void terminate() override;
    void setParameters(const QStringList &parameters) override;
    QProcess::ProcessState state() const override;
    void validate();
    void setIgnoreSslErrors();
    void setStarting();

signals:
    void procStateChanged(AbstractProcess *proc, ProcState progress);
    void requestAcceptSslErrors();
    void sslValidation(QString errorMessage);


protected slots:
    void completed(int exitCode) override;
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
    void setProcState(ProcState newState);
    QStringList compileParameters();
    QStringList remoteParameters();
    QByteArray convertReferences(const QByteArray &data);
    void startUnpacking();

    NeosManager *mManager;
    QString mOutPath;
    QString mJobNumber;
    QString mJobPassword;
    Priority mPrio;
    ProcState mProcState;
    QTimer mPullTimer;
    QProcess::ProcessState mPreparationState;

    AbstractGamsProcess *mSubProc = nullptr;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSPROCESS_H
