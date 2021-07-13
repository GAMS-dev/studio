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
    : QObject(parent), mAuthApi(new OAIAuthApi()), mDefaultApi(new OAIDefaultApi()), mJobsApi(new OAIJobsApi()),
      mNetworkManager(NetworkManager::manager()), mQueueFinished(false)
{
    mAuthApi->initializeServerConfigs();
    mAuthApi->setNetworkAccessManager(mNetworkManager);

    connect(mAuthApi, &OAIAuthApi::createJWTTokenJSONSignal, this,
            [this](OAIModel_auth_token summary) {
        emit reAuthorize(summary.getToken());
    });
    connect(mAuthApi, &OAIAuthApi::createJWTTokenJSONSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError e, QString text) {
        emit reAuthorizeError(getJsonMessageIfFound(text));
    });


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
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError e, QString text) {
        if (!QSslSocket::sslLibraryVersionString().startsWith("OpenSSL", Qt::CaseInsensitive)
                && e == QNetworkReply::SslHandshakeFailedError)
            emit sslErrors(nullptr, QList<QSslError>() << QSslError(QSslError::CertificateStatusUnknown));
        else
            emit reVersionError(getJsonMessageIfFound(text));
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
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, QString text) {
        emit reError("Network error " + QString::number(error_type).toLatin1() +
                     " from createJob:\n " + getJsonMessageIfFound(text));
    });

    connect(mJobsApi, &OAIJobsApi::getJobSignal, this, [this](OAIJob summary) {
        emit reGetJobStatus(summary.getStatus(), summary.getProcessStatus());
    });
    connect(mJobsApi, &OAIJobsApi::getJobSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, QString text) {
        emit reError("Network error " + QString::number(error_type).toLatin1() +
                     " from getJob:\n  " + getJsonMessageIfFound(text));
    });

    connect(mJobsApi, &OAIJobsApi::listJobsSignal, this, [this](OAIJob_no_text_entry_page summary) {
        emit reListJobs(summary.getCount());
    });
    connect(mJobsApi, &OAIJobsApi::listJobsSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, QString text) {
        emit reListJobsError("Network error " + QString::number(error_type) +
                             " from listJobs:\n  " + getJsonMessageIfFound(text));
    });


    connect(mJobsApi, &OAIJobsApi::getJobZipSignal, this, [this](OAIHttpFileElement summary) {
        emit reGetOutputFile(summary.asByteArray());
    });
    connect(mJobsApi, &OAIJobsApi::getJobZipSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, QString text) {
        emit reError("Network error " + QString::number(error_type).toLatin1() +
                     " from getJobZip:\n  " + getJsonMessageIfFound(text));
    });

    connect(mJobsApi, &OAIJobsApi::killJobSignal, this, [this](OAIMessage summary) {
        emit reKillJob(summary.getMessage());
    });
    connect(mJobsApi, &OAIJobsApi::killJobSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, QString text) {
        emit reError("Network error " + QString::number(error_type).toLatin1() +
                     " from killJob:\n  " + getJsonMessageIfFound(text));
    });

    connect(mJobsApi, &OAIJobsApi::popJobLogsSignal, this, [this](OAILog_piece summary) {
        if (!mQueueFinished) {
            mQueueFinished = summary.isQueueFinished();
            emit reGetLog(summary.getMessage().toUtf8());
        }
    });
    connect(mJobsApi, &OAIJobsApi::popJobLogsSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, QString text) {
        if (!mQueueFinished && error_type != QNetworkReply::ServiceUnavailableError)
            emit reGetLog("Network error " + QString::number(error_type).toLatin1() +
                          " from popLog:\n  " + getJsonMessageIfFound(text).toUtf8());

    });

    connect(mJobsApi, &OAIJobsApi::allPendingRequestsCompleted, this, [this]() {
        emit allPendingRequestsCompleted();
    });
    connect(this, &EngineManager::syncKillJob, this, &EngineManager::killJob, Qt::QueuedConnection);

}

EngineManager::~EngineManager()
{
    abortRequests();
    mAuthApi->deleteLater();
    mDefaultApi->deleteLater();
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

QString EngineManager::getJsonMessageIfFound(const QString &text)
{
    if (text.endsWith('}') || text.endsWith("}\n")) {
        int i = text.lastIndexOf("{\"message\": ");
        int j = text.lastIndexOf("\"}");
        if (i > 0)
            return text.mid(i+13, j-i-13) + "\n";
    }
    return text;
}

void EngineManager::setWorkingDirectory(const QString &dir)
{
    mJobsApi->setWorkingDirectory(dir);
}

void EngineManager::setUrl(const QString &url)
{
    mUrl = QUrl(url.endsWith('/') ? url.left(url.length()-1) : url);
    mAuthApi->setNewServerForAllOperations(mUrl);
    QUrl cleanUrl = url.endsWith('/') ? QUrl(url.left(url.length()-1)) : mUrl;
    mDefaultApi->setNewServerForAllOperations(cleanUrl);
    mJobsApi->setNewServerForAllOperations(cleanUrl);
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
    mAuthApi->setNetworkAccessManager(mNetworkManager);
    mDefaultApi->setNetworkAccessManager(mNetworkManager);
    mJobsApi->setNetworkAccessManager(mNetworkManager);
}

bool EngineManager::isIgnoreSslErrors() const
{
    return mNetworkManager == NetworkManager::managerSelfCert();
}

void EngineManager::authorize(const QString &user, const QString &password, int expireMinutes)
{
    mAuthApi->createJWTTokenJSON(user, password, expireMinutes * 60);
}

void EngineManager::setAuthToken(const QString &bearerToken)
{
    // JM workaround: set headers directly (and remove PW to avoid overwrite) until OAI is complete
    mJobsApi->addHeaders("Authorization", "Bearer " + bearerToken);
    mJobsApi->setPassword("");
}

void EngineManager::getVersion()
{
    mDefaultApi->getVersion();
}

void EngineManager::listJobs()
{
    mJobsApi->listJobs(false, QString("status process_status"), 1, 1);
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
    if (!mJobToken.isEmpty())
        mJobsApi->getJob(mJobToken, QString("status process_status"));
}

void EngineManager::killJob(bool hard)
{
    bool ok = !mJobToken.isEmpty();
    if (ok) {
        mJobsApi->killJob(mJobToken, hard);
    }
}

void EngineManager::getLog()
{
    if (!mJobToken.isEmpty()) {
        mJobsApi->popJobLogs(mJobToken);
    }
}

void EngineManager::getOutputFile()
{
    if (!mJobToken.isEmpty())
        mJobsApi->getJobZip(mJobToken);
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

QString EngineManager::getJobToken() const
{
    return mJobToken;
}

void EngineManager::setToken(const QString &token)
{
    mJobToken = token;
}

void EngineManager::abortRequests()
{
    mAuthApi->abortRequests();
    mDefaultApi->abortRequests();
    mJobsApi->abortRequests();
}

void EngineManager::cleanup()
{
    if (!mJobToken.isEmpty())
        mJobsApi->deleteJobZip(mJobToken);
}

} // namespace engine
} // namespace studio
} // namespace gams
