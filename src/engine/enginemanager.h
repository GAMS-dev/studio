/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
 */
#ifndef GAMS_STUDIO_ENGINE_ENGINEMANAGER_H
#define GAMS_STUDIO_ENGINE_ENGINEMANAGER_H

#include <QHash>
#include <QObject>
#include <QMetaEnum>
#include <QNetworkReply>
#include "client/OAIModel_auth_token.h"


namespace OpenAPI {
class OAIAuthApi;
class OAIDefaultApi;
class OAIJobsApi;
class OAINamespacesApi;
class OAIUsageApi;
}

namespace gams {
namespace studio {
namespace engine {

struct UsedQuota {
    UsedQuota() : max(-1), used(-1) {}
    UsedQuota(double _max, double _used) : max(_max), used(_used) {}
    double remain() {
        if (max < 0 || used < 0) return -1;
        return max - used;
    }
    double max;
    double used;
};

struct QuotaData {
    QString name;
    UsedQuota disk;
    UsedQuota volume;
    double parallel = 0;
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
    QString getJobToken() const;
    void setToken(const QString &token);
    void abortRequests();
    void cleanup();

    void authorize(const QString &user, const QString &password, int expireMinutes);
    void setAuthToken(const QString &bearerToken);
    void initUsername(const QString &user);
    void getVersion();
    void getUserInstances();
    void getQuota();
    void listJobs();
    void listNamespaces();
    void submitJob(QString modelName, QString nSpace, QString zipFile, QList<QString> params, QString instance);
    void getJobStatus();
    void getLog();
    void getOutputFile();

    void setDebug(bool debug = true);

signals:
    void syncKillJob(bool hard);

    void reAuthorize(const QString &token);
    void reAuthorizeError(const QString &error);
    void rePing(const QString &value);
    void reVersion(const QString &engineVersion, const QString &gamsVersion, bool isInKubernetes);
    void reVersionError(const QString &errorText);
    void reUserInstances(const QList<QPair<QString, QList<int> > > instances, const QString &defaultLabel);
    void reUserInstancesError(const QString &error);
    void reListNamspaces(const QStringList &list);
    void reListNamespacesError(const QString &error);
    void reQuota(QList<QuotaData*> data);
    void reQuotaError(const QString &error);
    void reListJobs(qint32 count);
    void reListJobsError(const QString &error);
    void reCreateJob(const QString &message, const QString &token);
    void reGetJobStatus(qint32 status, qint32 processStatus);
    void reKillJob(const QString &text);
    void reGetLog(const QByteArray &data);
    void jobIsQueued();
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void allPendingRequestsCompleted();

private slots:
    void killJob(bool hard);
    void debugReceived(QString name, QVariant data);

private:
    bool parseVersions(QByteArray json, QString &vEngine, QString &vGams) const;
    QString getJsonMessageIfFound(const QString &text);

private:
    OpenAPI::OAIAuthApi *mAuthApi;
    OpenAPI::OAIDefaultApi *mDefaultApi;
    OpenAPI::OAIJobsApi *mJobsApi;
    OpenAPI::OAINamespacesApi *mNamespacesApi;
    OpenAPI::OAIUsageApi *mUsageApi;
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
