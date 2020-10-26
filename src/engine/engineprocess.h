#ifndef GAMS_STUDIO_ENGINE_ENGINEPROCESS_H
#define GAMS_STUDIO_ENGINE_ENGINEPROCESS_H

#include "process.h"
#include <QTimer>

namespace gams {
namespace studio {
namespace engine {

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
    void setHasPreviousWorkOption(bool value);
    bool hasPreviousWorkOption() const { return mHasPreviousWorkOption; }
    QProcess::ProcessState state() const override;
    void setUrl(const QString &url);
    void setHost(const QString &_host);
    QString host() const;
    void setBasePath(const QString &path);
    QString basePath() const;
    void authenticate(const QString &user, const QString &password);
//    void authenticate(const QString &host, const QString &token);
    void setNamespace(const QString &nSpace);
    void setIgnoreSslErrors();
    void getVersions();

    bool forceGdx() const;
    void setForceGdx(bool forceGdx);

signals:
    void authenticated(QString token);
    void procStateChanged(AbstractProcess *proc, ProcState progress);
    void requestAcceptSslErrors();
    void sslValidation(QString errorMessage);
    void reVersion(const QString &engineVersion, const QString &gamsVersion);
    void reVersionError(const QString &errorText);

protected slots:
    void completed(int exitCode) override;
    void rePing(const QString &value);
    void reCreateJob(const QString &message, const QString &token);
    void reGetJobStatus(const qint32 &status, const qint32 &gamsExitCode);
    void reKillJob(const QString &text);
    void reGetLog(const QByteArray &data);
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
    void reVersionIntern(const QString &engineVersion, const QString &gamsVersion);

private:
    void setProcState(ProcState newState);
    QStringList compileParameters();
    QStringList remoteParameters();
    QByteArray convertReferences(const QByteArray &data);
    void startPacking();
    void startUnpacking();
    QString modelName();

    EngineManager *mManager;
    QString mHost;
    QString mBasePath;
    QString mUser;
    QString mPassword;
    QString mNamespace;
    QString mOutPath;
    QString mEngineVersion;
    QString mGamsVersion;
    bool mHasPreviousWorkOption = false;
    bool mForceGdx = true;

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
