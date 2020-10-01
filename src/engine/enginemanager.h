#ifndef GAMS_STUDIO_ENGINE_ENGINEMANAGER_H
#define GAMS_STUDIO_ENGINE_ENGINEMANAGER_H

#include <QHash>
#include <QObject>
#include <QMetaEnum>
//#include "httpmanager.h"

namespace OpenAPI {
    class OAIJobsApi;
}

namespace gams {
namespace studio {
namespace engine {

class EngineManager: public QObject
{
    Q_OBJECT
public:
    enum ProcCall {
        _ping,
        _version,
        _submitJob,
        _getJobStatus,
        _getCompletionCode,
        _getJobInfo,
        _killJob,
        _getIntermediateResultsNonBlocking,
        _getFinalResultsNonBlocking,
        _getOutputFile
    };
    Q_ENUM(ProcCall)

public:
    EngineManager(QObject *parent = nullptr);
    void setUrl(const QString &url);
    void setIgnoreSslErrors();
    bool ignoreSslErrors();

    void ping();
    void version();
    void submitJob(QString fileName, QString params = QString(), bool prioShort = true, bool wantGdx = true);
    void watchJob(int jobNumber, QString password);
    void getJobStatus();
    void getCompletionCode();
    void getJobInfo();
    void killJob(bool &ok);
    void getIntermediateResultsNonBlocking();
    void getFinalResultsNonBlocking();
    void getOutputFile(QString fileName);

    void setDebug(bool debug = true);

signals:
    void submitCall(const QString &method, const QVariantList &params = QVariantList());
    void rePing(const QString &value);
    void reVersion(const QString &value);
    void reSubmitJob(const int &jobNumber, const QString &jobPassword);
    void reGetJobStatus(const QString &value);
    void reGetCompletionCode(const QString &value);
    void reGetJobInfo(const QStringList &info);
    void reKillJob(const QString &text);
    void reGetIntermediateResultsNonBlocking(const QByteArray &data);
    void reGetFinalResultsNonBlocking(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);
    void sslErrors(const QStringList &errors);

private slots:
    void received(QString name, QVariant data);
    void debugReceived(QString name, QVariant data);
private:
    QHash<QString, ProcCall> procCalls;
    OpenAPI::OAIJobsApi *mHttp;
    int mJobNumber = 0;
    QString mPassword;
    int mLogOffset = 0;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ENGINE_ENGINEMANAGER_H
