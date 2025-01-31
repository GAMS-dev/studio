/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GAMS_STUDIO_ENGINE_ENGINEMANAGER_H
#define GAMS_STUDIO_ENGINE_ENGINEMANAGER_H

#include <QHash>
#include <QObject>
#include <QMetaEnum>
#include <QNetworkReply>
#include <engineapi/OAIModel_auth_token.h>

namespace OpenAPI {
class OAIAuthApi;
class OAIDefaultApi;
class OAIJobsApi;
class OAINamespacesApi;
class OAIUsageApi;
class OAIUsersApi;
}

namespace gams {
namespace studio {
namespace engine {

struct UsedQuota {
    UsedQuota() : max(-1), used(-1) {}
    UsedQuota(int _max, int _used) : max(_max), used(_used) {}
    int remain() {
        if (max < 0 || used < 0) return -1;
        return max - used;
    }
    int max;
    int used;
};

struct QuotaData {
    QString name;
    UsedQuota disk;
    UsedQuota volume;
    int parallel = 0;
};

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
    ~EngineManager() override;
    static void startupInit();

    void setWorkingDirectory(const QString &dir);
    void setUrl(const QString &url);
    QUrl url() { return mUrl; }
    void setIgnoreSslErrorsCurrentUrl(bool ignore);
    bool isIgnoreSslErrors() const;
    QString jobToken() const;
    void setJobToken(const QString &token);
    void abortRequests();
    void cleanup();

    void listProvider(const QString &name);
    void fetchOAuth2Token(const QString &name, const QString &deviceCode);
    void loginWithOIDC(const QString &idToken);
    void authorize(const QString &user, const QString &password, int expireMinutes);
    void setAuthToken(const QString &bearerToken);
    void getUsername();
    void initUsername(const QString &user);
    void getVersion();
    void getUserInstances();
    void getUserInstancePools();
    void updateInstancePool(const QString &label, int size);
    void getQuota();
    void listJobs();
    void listNamespaces();
    void submitJob(const QString &modelName, const QString &nSpace, const QString &zipFile, const QList<QString>& params, const QString &instance, const QString &tag);
    void getJobStatus();
    void getLog();
    void getOutputFile();

    void setDebug(bool debug = true);

signals:
    void syncKillJob(bool hard);

    void reListProvider(const QList<QHash<QString, QVariant> > &allProvider);
    void reListProviderError(const QString &error);
    void reFetchOAuth2Token(const QString &idToken);
    void reFetchOAuth2TokenError(int responseCode, const QString &error);
    void reLoginWithOIDC(const QString &token);
    void reLoginWithOIDCError(const QString &error);
    void reAuthorize(const QString &token);
    void reGetUsername(const QString &name);
    void reGetInvitees(const QStringList &invitees);
    void reAuthorizeError(const QString &error);
    void reGetUsernameError(const QString &error);
    void rePing(const QString &value);
    void reVersion(const QString &engineVersion, const QString &gamsVersion, bool isInKubernetes);
    void reVersionError(const QString &errorText);
    void reUserInstances(const QList<QPair<QString, QList<double> > > instances, QMap<QString, QString> *poolOwners,
                         const QString &defaultLabel = QString());
    void reUserInstancesError(const QString &error);
    void reListNamspaces(const QStringList &list);
    void reListNamespacesError(const QString &error);
    void reQuota(QList<gams::studio::engine::QuotaData*> data);
    void reQuotaError(const QString &error);
    void reListJobs(qint32 count);
    void reListJobsError(const QString &error);
    void reCreateJob(const QString &message, const QString &token);
    void reGetJobStatus(qint32 status, qint32 processStatus);
    void reKillJob(const QString &text);
    void reGetLog(const QByteArray &data);
    void reUpdateInstancePool();
    void reUpdateInstancePoolError(const QString &errorText);
    void jobIsQueued();
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void allPendingRequestsCompleted();

private slots:
    void killJob(bool hard);
    void debugReceived(const QString& name, const QVariant& data);

private:
    bool parseVersions(const QByteArray& json, QString &vEngine, QString &vGams) const;
    QString getJsonMessageIfFound(const QString &text);

private:
    OpenAPI::OAIAuthApi *mAuthApi;
    OpenAPI::OAIDefaultApi *mDefaultApi;
    OpenAPI::OAIJobsApi *mJobsApi;
    OpenAPI::OAINamespacesApi *mNamespacesApi;
    OpenAPI::OAIUsageApi *mUsageApi;
    OpenAPI::OAIUsersApi *mUsersApi;
    QUrl mUrl;
    QUrl mIgnoreSslUrl;
    QString mUser;
    QNetworkAccessManager *mNetworkManager;
    static QSslConfiguration *mSslConfigurationIgnoreErrOn;
    static QSslConfiguration *mSslConfigurationIgnoreErrOff;
    int mJobNumber = 0;
    QString mJobToken;
    bool mQueueFinished = false;
    static bool mStartupDone;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ENGINE_ENGINEMANAGER_H
