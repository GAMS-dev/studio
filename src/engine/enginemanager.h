/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
    ~EngineManager() override;
    void setWorkingDirectory(const QString &dir);
    void setUrl(const QString &url);
    void setIgnoreSslErrors();
    bool ignoreSslErrors();
    QString getToken() const;
    void setToken(const QString &token);
    void abortRequests();


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
    bool parseVersions(QByteArray json, QString &vEngine, QString &vGams) const;

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
