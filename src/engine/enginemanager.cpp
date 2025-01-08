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
#include "enginemanager.h"
#include <engineapi/OAIAuthApi.h>
#include <engineapi/OAIDefaultApi.h>
#include <engineapi/OAIJobsApi.h>
#include <engineapi/OAIHelpers.h>
#include <engineapi/OAINamespacesApi.h>
#include <engineapi/OAIUsageApi.h>
#include <engineapi/OAIUsersApi.h>
#include <QString>
#include <QFile>
#include <QSslConfiguration>

#include "networkmanager.h"
#include "logger.h"

using namespace OpenAPI;

namespace gams {
namespace studio {
namespace engine {

bool EngineManager::mStartupDone = false;
QSslConfiguration *EngineManager::mSslConfigurationIgnoreErrOn = nullptr;
QSslConfiguration *EngineManager::mSslConfigurationIgnoreErrOff = nullptr;

EngineManager::EngineManager(QObject* parent)
    : QObject(parent), mAuthApi(new OAIAuthApi()), mDefaultApi(new OAIDefaultApi()), mJobsApi(new OAIJobsApi()),
      mNamespacesApi(new OAINamespacesApi()), mUsageApi(new OAIUsageApi()), mUsersApi(new OAIUsersApi()),
      mNetworkManager(NetworkManager::manager()), mQueueFinished(false)
{
    // ===== initialize Authorization API =====

    mAuthApi->setNetworkAccessManager(mNetworkManager);

    connect(mAuthApi, &OAIAuthApi::createJWTTokenJSONSignal, this,
            [this](const OAIModel_auth_token &summary) {
        emit reAuthorize(summary.getToken());
    });
    connect(mAuthApi, &OAIAuthApi::createJWTTokenJSONSignalEFull, this, [this]
            (OAIHttpRequestWorker *, QNetworkReply::NetworkError , const QString &text) {
        emit reAuthorizeError(getJsonMessageIfFound(text));
    });
    connect(mAuthApi, &OAIAuthApi::listIdentityProvidersSignal, this, [this](const QList<OAIIdentity_provider> &summary) {
        QList<QHash<QString, QVariant>> allProvider;
        for (const OAIIdentity_provider &ip : summary) {
            if (ip.is_oidc_Set()) {
                QHash<QString, QVariant> provider;
                provider.insert("name", ip.getName());
                provider.insert("label", ip.getLabel());
                const OAIIdentity_provider_oidc &oidc = ip.getOidc();
                if (oidc.is_device_authorization_endpoint_Set() && oidc.is_device_client_id_Set()) {
                    provider.insert("authEndpoint", oidc.getDeviceAuthorizationEndpoint());
                    provider.insert("tokenEndpoint", oidc.getTokenEndpoint());
                    provider.insert("clientId", oidc.getDeviceClientId());
                    provider.insert("scopes", oidc.getScopes());
                    provider.insert("hasSecret", oidc.isHasDeviceClientSecret());
                    allProvider << provider;
                }
            }
        }
        emit reListProvider(allProvider);
    });
    connect(mAuthApi, &OAIAuthApi::listIdentityProvidersSignalEFull, this,
            [this](OAIHttpRequestWorker *w, QNetworkReply::NetworkError , const QString& ) {
        emit reListProviderError(w->error_str);
    });
    connect(mAuthApi, &OAIAuthApi::fetchOAuth2TokenOnBehalfSignal, this, [this](const OAIForwarded_token_response &summary) {
        emit reFetchOAuth2Token(summary.getIdToken());
    });
    connect(mAuthApi, &OAIAuthApi::fetchOAuth2TokenOnBehalfSignalEFull, this,
            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError , const QString &text) {
        emit reFetchOAuth2TokenError(worker->getHttpResponseCode(), getJsonMessageIfFound(text));
    });
    connect(mAuthApi, &OAIAuthApi::loginWithOIDCSignal, this, [this](const OAIModel_auth_token &summary) {
        emit reLoginWithOIDC(summary.getToken());
    });
    connect(mAuthApi, &OAIAuthApi::loginWithOIDCSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError , const QString &text) {
        emit reLoginWithOIDCError(getJsonMessageIfFound(text));
    });

    // ===== initialize Namespaces API =====

    mNamespacesApi->setNetworkAccessManager(mNetworkManager);

    connect(mNamespacesApi, &OAINamespacesApi::listNamespacesSignal, this, [this](const QList<OAINamespace> &summary) {
        QStringList nSpaces;
        for (const OAINamespace &nspace : summary) {
            bool go = false;
            const QList<OAIPerm_and_username> perms = nspace.getPermissions();
            for (const OAIPerm_and_username &perm : perms) {
                if (perm.getUsername().compare(mUser) == 0) {
                    int permVal = perm.getPermission();
                    if ((permVal & 3) == 3) {
                        go = true;
                        break;
                    }
                }
            }
            if (go) nSpaces << nspace.getName();
        }
        emit reListNamspaces(nSpaces);
    });
    connect(mNamespacesApi, &OAINamespacesApi::listNamespacesSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError , const QString &text) {
        emit reListNamespacesError(getJsonMessageIfFound(text));
    });


    // ===== initialize Usage API =====

    mUsageApi->setNetworkAccessManager(mNetworkManager);

    connect(mUsageApi, &OAIUsageApi::getUserInstancesSignal, this, [this](const OAIModel_userinstance_info &summary) {
        OAIModel_instance_info iiDef = summary.getDefaultInstance();
        const QList<OAIModel_instance_info> infoList = summary.getInstancesAvailable();
        QList<QPair<QString, QList<double>>> instList;
        for (const OAIModel_instance_info &ii : infoList) {
            QList<double> list;
            qreal memGb = qreal(ii.getMemoryRequest()) / 1000.;
            list << ii.getCpuRequest() << memGb << ii.getMultiplier();
            instList << QPair<QString, QList<double>>(ii.getLabel(), list);
        }
        emit reUserInstances(instList, iiDef.getLabel());
    });
    connect(mUsageApi, &OAIUsageApi::getUserInstancesSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError , const QString &text) {
        emit reUserInstancesError(getJsonMessageIfFound(text));
    });

    connect(mUsageApi, &OAIUsageApi::getQuotaSignal, this, [this](const QList<OAIQuota> &summary) {
        QList<QuotaData*> dataList;
        for (const OAIQuota &quota : summary) {
            QuotaData *data = new QuotaData();
            data->name = quota.getUsername();
            if (quota.is_disk_quota_Set())
                data->disk = UsedQuota(quota.getDiskQuota(), quota.getDiskUsed());
            if (quota.is_volume_quota_Set())
                data->volume = UsedQuota(int(quota.getVolumeQuota()), int(quota.getVolumeUsed()));
            if (quota.is_parallel_quota_Set())
                data->parallel = int(quota.getParallelQuota());
            dataList << data;
        }
        emit reQuota(dataList);
    });
    connect(mUsageApi, &OAIUsageApi::getQuotaSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError , const QString &text) {
        emit reQuotaError(getJsonMessageIfFound(text));
    });

    // ===== initialize Users API =====

    mUsersApi->setNetworkAccessManager(mNetworkManager);

    connect(mUsersApi, &OAIUsersApi::listUsersSignal, this, [this](QList<OAIUser> summary) {
        if (summary.size() == 1) {
            mUser = summary.first().getUsername();
            emit reGetUsername(mUser);
        } else
            emit reGetUsernameError("Error while fetching username");
    });

    connect(mUsersApi, &OAIUsersApi::listUsersSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError , const QString &text) {
        emit reGetUsernameError("ERR: "+getJsonMessageIfFound(text));
    });


    // ===== initialize Default API =====

    mDefaultApi->setNetworkAccessManager(mNetworkManager);

    connect(mDefaultApi, &OAIDefaultApi::getVersionSignalFull, this,
            [this](OAIHttpRequestWorker *worker, const OAIModel_version &summary) {
        QString vEngine;
        QString vGams;
        if (parseVersions(worker->response, vEngine, vGams)) {
            emit reVersion(vEngine, vGams, summary.isInKubernetes());
        } else {
            emit reVersionError("Could not parse versions");
        }
    });
    connect(mDefaultApi, &OAIDefaultApi::getVersionSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError e, const QString &text) {
        if (!QSslSocket::sslLibraryVersionString().startsWith("OpenSSL", Qt::CaseInsensitive)
                && e == QNetworkReply::SslHandshakeFailedError)
            emit sslErrors(nullptr, QList<QSslError>() << QSslError(QSslError::CertificateStatusUnknown));
        else
            emit reVersionError(getJsonMessageIfFound(text));
    });

    connect(mNetworkManager, &QNetworkAccessManager::sslErrors, this, &EngineManager::sslErrors);


    // ===== initialize Job API =====

    mJobsApi->setNetworkAccessManager(mNetworkManager);

    // TODO(JM) discuss this
    // Would be convenient at the end of  OAIHttpRequestWorker::execute ...
//    connect(reply, &QNetworkReply::downloadProgress, this, &OAIHttpRequestWorker::downloadProgress);
    // pass though OAIJobApi and use here
//    connect(mJobsApi, &OAIJobsApi::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal) {
//        DEB() << "progress " << bytesReceived << " from " << bytesTotal;
//    });

    connect(mJobsApi, &OAIJobsApi::createJobSignal, this,
            [this](const OAIMessage_and_token &summary) {
        emit reCreateJob(summary.getMessage(), summary.getToken());
    });
    connect(mJobsApi, &OAIJobsApi::createJobSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, const QString &text) {
        emit reError("Network error " + QString::number(error_type).toLatin1() +
                     " from createJob:\n " + getJsonMessageIfFound(text));
    });

    connect(mJobsApi, &OAIJobsApi::getJobSignal, this, [this](const OAIJob &summary) {
        emit reGetJobStatus(summary.getStatus(), summary.getProcessStatus());
    });
    connect(mJobsApi, &OAIJobsApi::getJobSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, const QString &text) {
        emit reError("Network error " + QString::number(error_type).toLatin1() +
                     " from getJob:\n  " + getJsonMessageIfFound(text));
    });

    connect(mJobsApi, &OAIJobsApi::listJobsSignal, this, [this](const OAIJob_no_text_entry_page &summary) {
        emit reListJobs(summary.getCount());
    });
    connect(mJobsApi, &OAIJobsApi::listJobsSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, const QString &text) {
        emit reListJobsError("Network error " + QString::number(error_type) +
                             " from listJobs:\n  " + getJsonMessageIfFound(text));
    });


    connect(mJobsApi, &OAIJobsApi::getJobZipSignal, this, [this](const OAIHttpFileElement &summary) {
        emit reGetOutputFile(summary.asByteArray());
    });
    connect(mJobsApi, &OAIJobsApi::getJobZipSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, const QString &text) {
        emit reError("Network error " + QString::number(error_type).toLatin1() +
                     " from getJobZip:\n  " + getJsonMessageIfFound(text));
    });

    connect(mJobsApi, &OAIJobsApi::killJobSignal, this, [this](const OAIMessage &summary) {
        emit reKillJob(summary.getMessage());
    });
    connect(mJobsApi, &OAIJobsApi::killJobSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, const QString &text) {
        emit reError("Network error " + QString::number(error_type).toLatin1() +
                     " from killJob:\n  " + getJsonMessageIfFound(text));
    });

    connect(mJobsApi, &OAIJobsApi::popJobLogsSignal, this, [this](const OAILog_piece &summary) {
        if (!mQueueFinished) {
            mQueueFinished = summary.isQueueFinished();
            emit reGetLog(summary.getMessage().toUtf8());
        }
        if (summary.is_gams_return_code_Set() || mQueueFinished)
            getJobStatus();
    });
    connect(mJobsApi, &OAIJobsApi::popJobLogsSignalEFull, this,
            [this](OAIHttpRequestWorker *, QNetworkReply::NetworkError error_type, const QString &text) {
        if (!mQueueFinished && error_type != QNetworkReply::ContentAccessDenied) {
            emit reError("Network error " + QString::number(error_type).toLatin1() +
                         " from popLog:\n  " + getJsonMessageIfFound(text).toUtf8());
        } else {
            emit jobIsQueued();
        }
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
    mNamespacesApi->deleteLater();
    mUsageApi->deleteLater();
    mUsersApi->deleteLater();
}

void EngineManager::startupInit()
{
    if (!mStartupDone) {
        mSslConfigurationIgnoreErrOff = new QSslConfiguration(QSslConfiguration::defaultConfiguration());
        mSslConfigurationIgnoreErrOff->setBackendConfigurationOption("MinProtocol", "TLSv1.2");
        mSslConfigurationIgnoreErrOn = new QSslConfiguration(QSslConfiguration::defaultConfiguration());
        mSslConfigurationIgnoreErrOn->setPeerVerifyMode(QSslSocket::VerifyNone);
        OAIHttpRequestWorker::sslDefaultConfiguration = mSslConfigurationIgnoreErrOff;
        mStartupDone = true;
    }
}

QString EngineManager::getJsonMessageIfFound(const QString &text)
{
    if (text.endsWith('}') || text.endsWith("}\n")) {
        qsizetype i = text.lastIndexOf("{\"message\": ");
        qsizetype j = text.lastIndexOf("\"}");
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
    mNamespacesApi->setNewServerForAllOperations(cleanUrl);
    mUsageApi->setNewServerForAllOperations(cleanUrl);
    mUsersApi->setNewServerForAllOperations(cleanUrl);
    if (mUrl.scheme() == "https")
        setIgnoreSslErrorsCurrentUrl(mUrl.host() == mIgnoreSslUrl.host() && mUrl.port() == mIgnoreSslUrl.port());
}

void EngineManager::setIgnoreSslErrorsCurrentUrl(bool ignore)
{
    if (ignore) {
        mIgnoreSslUrl = mUrl;
        mNetworkManager = NetworkManager::managerSelfCert();
        OAIHttpRequestWorker::sslDefaultConfiguration = mSslConfigurationIgnoreErrOn;
    } else {
        mIgnoreSslUrl = QUrl();
        mNetworkManager = NetworkManager::manager();
        OAIHttpRequestWorker::sslDefaultConfiguration = mSslConfigurationIgnoreErrOff;
    }
    mAuthApi->setNetworkAccessManager(mNetworkManager);
    mDefaultApi->setNetworkAccessManager(mNetworkManager);
    mJobsApi->setNetworkAccessManager(mNetworkManager);
    mNamespacesApi->setNetworkAccessManager(mNetworkManager);
    mUsageApi->setNetworkAccessManager(mNetworkManager);
    mUsersApi->setNetworkAccessManager(mNetworkManager);
}

bool EngineManager::isIgnoreSslErrors() const
{
    return mNetworkManager == NetworkManager::managerSelfCert();
}

void EngineManager::listProvider(const QString &name)
{
    mAuthApi->listIdentityProviders(name);
}

void EngineManager::fetchOAuth2Token(const QString &name, const QString &deviceCode)
{
    ::OpenAPI::OptionalParam<QString> dummy;
    mAuthApi->fetchOAuth2TokenOnBehalf(name, QString("urn:ietf:params:oauth:grant-type:device_code"), dummy, dummy, dummy, deviceCode);
}

void EngineManager::loginWithOIDC(const QString &idToken)
{
    mAuthApi->loginWithOIDC(idToken);
}

void EngineManager::authorize(const QString &user, const QString &password, int expireMinutes)
{
    mUser = user;
    ::OpenAPI::OptionalParam<QString> dummy;
    mAuthApi->createJWTTokenJSON(user, password, dummy, dummy, expireMinutes * 60);
}

void EngineManager::setAuthToken(const QString &bearerToken)
{
    // JM workaround: set headers directly (and remove PW to avoid overwrite) until OAI is complete
    mJobsApi->addHeaders("Authorization", "Bearer " + bearerToken);
    mJobsApi->setPassword("");
    mNamespacesApi->addHeaders("Authorization", "Bearer " + bearerToken);
    mNamespacesApi->setPassword("");
    mUsageApi->addHeaders("Authorization", "Bearer " + bearerToken);
    mUsageApi->setPassword("");
    mUsersApi->addHeaders("Authorization", "Bearer " + bearerToken);
    mUsersApi->setPassword("");
}

void EngineManager::getUsername()
{
    ::OpenAPI::OptionalParam<QString> dummy;
    mUsersApi->listUsers(dummy, false);
}

void EngineManager::initUsername(const QString &user)
{
    mUser = user;
}

void EngineManager::getVersion()
{
    mDefaultApi->getVersion();
}

void EngineManager::getUserInstances()
{
    mUsageApi->getUserInstances(mUser);
}

void EngineManager::getQuota()
{
    mUsageApi->getQuota(mUser);
}

void EngineManager::listJobs()
{
    mJobsApi->listJobs(false, QString("status process_status"), 1, 1);
}

void EngineManager::listNamespaces()
{
    mNamespacesApi->listNamespaces();
}

void EngineManager::submitJob(const QString &modelName, const QString &nSpace, const QString &zipFile,
                              const QList<QString>& params, const QString &instance, const QString &tag)
{
    OAIHttpFileElement model;
    model.setMimeType("application/zip");
    model.setFileName(zipFile);
    QString dummy;
    QStringList dummyL;
    QStringList labels;
    if (!instance.isEmpty())
        labels << QString("instance=%1").arg(instance);

    mJobsApi->createJob(modelName, nSpace, dummy, dummy, dummyL, dummyL, QString("solver.log"), params, dummyL, labels, tag, dummyL, model);
}

void EngineManager::getJobStatus()
{
    if (!mJobToken.isEmpty())
        mJobsApi->getJob(mJobToken, QString("status, process_status"));
}

void EngineManager::killJob(bool hard)
{
    bool ok = !mJobToken.isEmpty();
    if (ok) {
        mJobsApi->killJob(mJobToken, hard);
        mJobToken = "";
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


void EngineManager::debugReceived(const QString& name, const QVariant& data)
{
    qDebug() << "\nResult from " << name << ":\n" << data;
}

bool EngineManager::parseVersions(const QByteArray& json, QString &vEngine, QString &vGams) const
{
    QJsonDocument jDoc = QJsonDocument::fromJson(json);
    QJsonObject jObj = jDoc.object();
    if (!::OpenAPI::fromJsonValue(vEngine, jObj[QString("version")])) return false;
    if (jObj[QString("version")].isNull()) return false;
    if (!::OpenAPI::fromJsonValue(vGams, jObj[QString("gams_version")])) return false;
    if (jObj[QString("gams_version")].isNull()) return false;
    return true;
}

QString EngineManager::jobToken() const
{
    return mJobToken;
}

void EngineManager::setJobToken(const QString &token)
{
    mJobToken = token;
}

void EngineManager::abortRequests()
{
    mAuthApi->abortRequests();
    mDefaultApi->abortRequests();
    mJobsApi->abortRequests();
    mUsageApi->abortRequests();
    mUsersApi->abortRequests();
}

void EngineManager::cleanup()
{
    if (!mJobToken.isEmpty())
        mJobsApi->deleteJobZip(mJobToken);
}

} // namespace engine
} // namespace studio
} // namespace gams
