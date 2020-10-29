#ifndef GAMS_STUDIO_ENGINE_ENGINEMANAGER_H
#define GAMS_STUDIO_ENGINE_ENGINEMANAGER_H

#include <QHash>
#include <QObject>
#include <QMetaEnum>
#include <QNetworkReply>

namespace OpenAPI {
class OAIAuthApi;
class OAIDefaultApi;
class OAIJobsApi;
}

namespace gams {
namespace studio {
namespace engine {

class EngineManager: public QObject
{
    Q_OBJECT
public:
    enum StatusCode {
        Cancelled   = -3,
        Cancelling  = -2,
        Corrupted   = -1,
        Queued      =  0,
        Running     =  1,
        Outputting  =  2,
        Finished    = 10,
    };
    Q_ENUM(StatusCode)

public:
    EngineManager(QObject *parent = nullptr);
    void setWorkingDirectory(const QString &dir);
    void setHost(const QString &host);
    void setBasePath(const QString &path);
    void setIgnoreSslErrors();
    bool ignoreSslErrors();
    QString getToken() const;
    void setToken(const QString &token);

    void authenticate(const QString &user, const QString &password);
    void authenticate(const QString &userToken);
    void getVersion();
    void submitJob(QString modelName, QString nSpace, QString zipFile, QStringList params);
    void getJobStatus();
    void getLog();
    void getOutputFile();

    void setDebug(bool debug = true);

signals:
    void syncKillJob(bool hard);

    void reAuth(const QString &token);
    void rePing(const QString &value);
    void reVersion(const QString &engineVersion, const QString &gamsVersion);
    void reVersionError(const QString &errorText);
    void reCreateJob(const QString &message, const QString &token);
    void reGetJobStatus(qint32 status, qint32 processStatus);
    void reKillJob(const QString &text);
    void reGetLog(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);
    void sslErrors(const QStringList &errors);

private slots:
    void killJob(bool hard);
    void debugReceived(QString name, QVariant data);

    void abortRequestsSignal();

private:
    bool parseVersions(QByteArray json, QString &vEngine, QString &vGams);

private:
//    OpenAPI::OAIAuthApi *mAuthApi;
    OpenAPI::OAIDefaultApi *mDefaultApi;
    OpenAPI::OAIJobsApi *mJobsApi;
    QNetworkAccessManager *mNetworkManager;
    int mJobNumber = 0;
    QString mUser;
    QString mPassword;
    QString mToken;
    bool mQueueFinished = false;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ENGINE_ENGINEMANAGER_H
