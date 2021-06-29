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
#include "enginemanager.h"
#include "logger.h"
#include "client/OAIAuthApi.h"
#include "client/OAIDefaultApi.h"
#include "client/OAIJobsApi.h"
#include "client/OAIHelpers.h"
#include <QString>
#include <iostream>
#include <QFile>
#include <QSslConfiguration>

#include "networkmanager.h"

using namespace OpenAPI;

namespace gams {
namespace studio {
namespace engine {

bool EngineManager::mStartupDone = false;
QSslConfiguration EngineManager::mSslConfigurationIgnoreErrOn;
QSslConfiguration EngineManager::mSslConfigurationIgnoreErrOff;

EngineManager::EngineManager(QObject* parent)
    : QObject(parent), /*mAuthApi(new OAIAuthApi()),*/ mDefaultApi(new OAIDefaultApi()), mJobsApi(new OAIJobsApi()),
      mNetworkManager(NetworkManager::manager()), mQueueFinished(false)
{
//    connect(mAuthApi, &OAIAuthApi::postLoginInterfaceSignal, this,
//            [this](OAIModel_auth_token summary) {
//        emit reAuth(summary.getToken());
//    });
//    connect(mAuthApi, &OAIAuthApi::postLoginInterfaceSignalEFull, this,
//            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError , QString) {
//        emit reError("From postW: "+worker->error_str);
//    });


    mDefaultApi->setNetworkAccessManager(mNetworkManager);

    connect(mDefaultApi, &OAIDefaultApi::getVersionSignalFull, this,
            [this](OAIHttpRequestWorker *worker) {
        QString vEngine;
        QString vGams;
        if (parseVersions(worker->response, vEngine, vGams)) {
            emit reVersion(vEngine, vGams);
        } else {
            emit reVersionError("Could not parse versions");
        }
    });
    connect(mDefaultApi, &OAIDefaultApi::getVersionSignalEFull, this,
            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError e, QString ) {
        if (!QSslSocket::sslLibraryVersionString().startsWith("OpenSSL", Qt::CaseInsensitive)
                && e == QNetworkReply::SslHandshakeFailedError)
            emit sslErrors(nullptr, QList<QSslError>() << QSslError(QSslError::CertificateStatusUnknown));
        else
            emit reVersionError(worker->error_str);
    });

    connect(mNetworkManager, &QNetworkAccessManager::sslErrors, this, &EngineManager::sslErrors);

    mJobsApi->setNetworkAccessManager(mNetworkManager);
//    mJobsApi->setScheme("https");
//    mJobsApi->setPort(443);

    connect(mJobsApi, &OAIJobsApi::createJobSignal, this,
            [this](OAIMessage_and_token summary) {
        emit reCreateJob(summary.getMessage(), summary.getToken());
    });
    connect(mJobsApi, &OAIJobsApi::createJobSignalEFull, this,
            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString ) {
        emit reError("Network error "+QString::number(error_type).toLatin1()+" from createJob: "+worker->error_str);
    });

    connect(mJobsApi, &OAIJobsApi::getJobSignal, this, [this](OAIJob summary) {
        emit reGetJobStatus(summary.getStatus(), summary.getProcessStatus());
    });
    connect(mJobsApi, &OAIJobsApi::getJobSignalEFull, this,
            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString ) {
        emit reError("Network error "+QString::number(error_type).toLatin1()+" from getJob: "+worker->error_str);
    });

    connect(mJobsApi, &OAIJobsApi::getJobZipSignal, this, [this](OAIHttpFileElement summary) {
        emit reGetOutputFile(summary.asByteArray());
    });
    connect(mJobsApi, &OAIJobsApi::getJobZipSignalEFull, this,
            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString ) {
        emit reError("Network error "+QString::number(error_type).toLatin1()+" from getJobZip: "+worker->error_str);
    });

    connect(mJobsApi, &OAIJobsApi::killJobSignal, this, [this](OAIMessage summary) {
        emit reKillJob(summary.getMessage());
    });
    connect(mJobsApi, &OAIJobsApi::killJobSignalEFull, this,
            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString ) {
        emit reError("Network error "+QString::number(error_type).toLatin1()+" from killJob: "+worker->error_str);
    });

    connect(mJobsApi, &OAIJobsApi::popJobLogsSignal, this, [this](OAILog_piece summary) {
        if (!mQueueFinished) {
            mQueueFinished = summary.isQueueFinished();
            emit reGetLog(summary.getMessage().toUtf8());
        }
    });
    connect(mJobsApi, &OAIJobsApi::popJobLogsSignalEFull, this,
            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString ) {
        if (!mQueueFinished && error_type != QNetworkReply::ServiceUnavailableError)
            emit reGetLog("Network error "+QString::number(error_type).toLatin1()+" from popLog: "+worker->error_str.toUtf8());

    });

    connect(mJobsApi, &OAIJobsApi::allPendingRequestsCompleted, this, [this]() {
        emit allPendingRequestsCompleted();
    });
    connect(this, &EngineManager::syncKillJob, this, &EngineManager::killJob, Qt::QueuedConnection);

}

EngineManager::~EngineManager()
{
    mJobsApi->abortRequests();
    mJobsApi->deleteLater();
}

void EngineManager::startupInit()
{
    if (!mStartupDone) {
        mSslConfigurationIgnoreErrOn.setPeerVerifyMode(QSslSocket::VerifyNone);
        OAIHttpRequestWorker::sslDefaultConfiguration = &mSslConfigurationIgnoreErrOff;
        mStartupDone = true;
    }
}

void EngineManager::setWorkingDirectory(const QString &dir)
{
    mJobsApi->setWorkingDirectory(dir);
}

void EngineManager::setUrl(const QString &url)
{
    mUrl = QUrl(url);
    mDefaultApi->setNewServerForAllOperations(mUrl);
    mJobsApi->setNewServerForAllOperations(mUrl);
//    DEB() << "ManagerHost: " << mUrl.host() << ":" << mUrl.port() << "  scheme:" << mUrl.scheme();
    if (mUrl.scheme() == "https")
        setIgnoreSslErrorsCurrentUrl(mUrl.host() == mIgnoreSslUrl.host() && mUrl.port() == mIgnoreSslUrl.port());
}

void EngineManager::setIgnoreSslErrorsCurrentUrl(bool ignore)
{
    if (ignore) {
        mIgnoreSslUrl = mUrl;
        mNetworkManager = NetworkManager::managerSelfCert();
        OAIHttpRequestWorker::sslDefaultConfiguration = &mSslConfigurationIgnoreErrOn;
    } else {
        mIgnoreSslUrl = QUrl();
        mNetworkManager = NetworkManager::manager();
        OAIHttpRequestWorker::sslDefaultConfiguration = &mSslConfigurationIgnoreErrOff;
    }
    mDefaultApi->setNetworkAccessManager(mNetworkManager);
    mJobsApi->setNetworkAccessManager(mNetworkManager);
}

bool EngineManager::isIgnoreSslErrors() const
{
    return mNetworkManager == NetworkManager::managerSelfCert();
}

bool EngineManager::ignoreSslErrors()
{
    return false;
}

void EngineManager::authenticate(const QString &user, const QString &password)
{
    mJobsApi->setUsername(user);
    mJobsApi->setPassword(password);
}

void EngineManager::authenticate(const QString &bearerToken)
{
    // JM workaround: set headers directly (and remove PW to avoid overwrite) until OAI is complete
    mJobsApi->addHeaders("Authorization", "Bearer " + bearerToken);
    mJobsApi->setPassword("");
}

void EngineManager::getVersion()
{
    mDefaultApi->getVersion();
}

void EngineManager::submitJob(QString modelName, QString nSpace, QString zipFile, QList<QString> params)
{
    OAIHttpFileElement model;
    model.setMimeType("application/zip");
    model.setFileName(zipFile);
    QString dummy;
    QStringList dummyL;

    mJobsApi->createJob(modelName, nSpace, dummy, dummyL, dummyL, QString("solver.log"), params, dummyL, dummyL, model);
}

void EngineManager::getJobStatus()
{
    if (!mToken.isEmpty())
        mJobsApi->getJob(mToken, QString("status process_status"));
}

void EngineManager::killJob(bool hard)
{
    bool ok = !mToken.isEmpty();
    if (ok) {
        mJobsApi->killJob(mToken, hard);
    }
}

void EngineManager::getLog()
{
    if (!mToken.isEmpty()) {
        mJobsApi->popJobLogs(mToken);
    }
}

void EngineManager::getOutputFile()
{
    if (!mToken.isEmpty())
        mJobsApi->getJobZip(mToken);
}

void EngineManager::setDebug(bool debug)
{
    Q_UNUSED(debug)
//    if (debug)
//        connect(&mHttp, &HttpManager::received, this, &EngineManager::debugReceived, Qt::UniqueConnection);
//    else
//        disconnect(&mHttp, &HttpManager::received, this, &EngineManager::debugReceived);
}


void EngineManager::debugReceived(QString name, QVariant data)
{
    qDebug() << "\nResult from " << name << ":\n" << data;
}

bool EngineManager::parseVersions(QByteArray json, QString &vEngine, QString &vGams) const
{
    QJsonDocument jDoc = QJsonDocument::fromJson(json);
    QJsonObject jObj = jDoc.object();
    if (!::OpenAPI::fromJsonValue(vEngine, jObj[QString("version")])) return false;
    if (jObj[QString("version")].isNull()) return false;
    if (!::OpenAPI::fromJsonValue(vGams, jObj[QString("gams_version")])) return false;
    if (jObj[QString("gams_version")].isNull()) return false;
    return true;
}

QString EngineManager::getToken() const
{
    return mToken;
}

void EngineManager::setToken(const QString &token)
{
    mToken = token;
}

void EngineManager::abortRequests()
{
    mDefaultApi->abortRequests();
    mJobsApi->abortRequests();
}

void EngineManager::cleanup()
{
    if (!mToken.isEmpty())
        mJobsApi->deleteJobZip(mToken);
}

} // namespace engine
} // namespace studio
} // namespace gams
