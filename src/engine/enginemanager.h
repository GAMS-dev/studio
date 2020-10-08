#ifndef GAMS_STUDIO_ENGINE_ENGINEMANAGER_H
#define GAMS_STUDIO_ENGINE_ENGINEMANAGER_H

#include <QHash>
#include <QObject>
#include <QMetaEnum>
#include <QNetworkReply>
//#include "httpmanager.h"

namespace OpenAPI {
class OAIAuthApi;
class OAIJobsApi;
}

namespace gams {
namespace studio {
namespace engine {

class EngineManager: public QObject
{
    Q_OBJECT
public:
    enum StatusCodes {
        Cancelled   = -3,
        Cancelling  = -2,
        Corrupted   = -1,
        Queued      =  0,
        Running     =  1,
        Outputting  =  2,
        Finished    = 10,
    };
    Q_ENUM(StatusCodes)

public:
    EngineManager(QObject *parent = nullptr);
    void setHost(const QString &host);
    void setIgnoreSslErrors();
    bool ignoreSslErrors();
    QString getToken() const;
    void setToken(const QString &token);

    void authenticate(const QString &user, const QString &password);
    void ping();
//    void version();
    void submitJob(QString fileName, QString zipFile, QStringList params);
    void getJobStatus();
    void killJob(bool hard, bool &ok);
    void getLog();
    void getOutputFile();

    void setDebug(bool debug = true);

signals:
    void reAuth(const QString &token);
    void rePing(const QString &value);
    void reVersion(const QString &value);
    void reSubmitJob(const QString &message, const QString &token);
    void reGetJobStatus(qint32 status, qint32 processStatus);
    void reGetCompletionCode(const QString &value);
    void reGetJobInfo(const QStringList &info);
    void reKillJob(const QString &text);
    void reGetLog(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);
    void sslErrors(const QStringList &errors);

private slots:
    void debugReceived(QString name, QVariant data);

    void getJobZipSignalE(QNetworkReply::NetworkError error_type, QString error_str);
    void abortRequestsSignal();

private:
    void reCreateJob(QString message, QString token);

private:
    OpenAPI::OAIAuthApi *mAuthApi;
    OpenAPI::OAIJobsApi *mJobsApi;
    int mJobNumber = 0;
    QString mUser;
    QString mPassword;
    QString mToken;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ENGINE_ENGINEMANAGER_H
