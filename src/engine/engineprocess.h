#ifndef GAMS_STUDIO_ENGINE_ENGINEPROCESS_H
#define GAMS_STUDIO_ENGINE_ENGINEPROCESS_H

#include "process.h"
#include <QTimer>

namespace gams {
namespace studio {
namespace engine {

// TODO(JM) join ProcState from neos and engine
enum ProcState {
    ProcCheck,
    ProcIdle,
    Proc1Compile,
    Proc2Pack,
    Proc3Monitor,
    Proc4GetResult,
    Proc5Unpack,
};

class EngineManager;

/// \brief The EngineProcess controls all steps to run a job on GAMS Engine
/// This class works in four process steps:
/// 1. compile gms on the local machine
/// 2. monitor the remote job
/// 3. get result file
/// 4. unpack result file and finish
class EngineProcess final : public AbstractGamsProcess
{
    Q_OBJECT
public:
    EngineProcess(QObject *parent = nullptr);
    ~EngineProcess() override;

    void execute() override;
    void interrupt() override;
    void terminate() override;
    void setParameters(const QStringList &parameters) override;
    QProcess::ProcessState state() const override;
    void authenticate(const QString &host, const QString &user, const QString &password);
    void setNamespace(const QString &nSpace);
    void validate();
    void setIgnoreSslErrors();

signals:
    void authenticated(QString token);
    void procStateChanged(AbstractProcess *proc, ProcState progress);
    void requestAcceptSslErrors();
    void sslValidation(QString errorMessage);

protected slots:
    void rePing(const QString &value);
    void reVersion(const QString &value);
    void reSubmitJob(const QString &message, const QString &token);
    void reGetJobStatus(const qint32 &status, const qint32 &gamsExitCode);
    void reGetCompletionCode(const QString &code);
    void reGetJobInfo(const QStringList &info);
    void reKillJob(const QString &text);
    void reGetIntermediateResultsNonBlocking(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);

private slots:
    void pullStatus();
    void compileCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void packCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void unpackCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void sslErrors(const QStringList &errors);
    void parseUnZipStdOut(const QByteArray &data);
    void subProcStateChanged(QProcess::ProcessState newState);

private:
    void setProcState(ProcState newState);
    QStringList compileParameters();
    QStringList remoteParameters();
    QByteArray convertReferences(const QByteArray &data);
    void startPacking();
    void startUnpacking();

    EngineManager *mManager;
    QString mUser;
    QString mPassword;
    QString mNamespace;
    QString mOutPath;
    QString mToken;

    QString mJobNumber;
    QString mJobPassword;
    ProcState mProcState;
    QTimer mPullTimer;

    AbstractGamsProcess *mSubProc = nullptr;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ENGINE_ENGINEPROCESS_H
