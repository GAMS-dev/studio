#include "enginemanager.h"
#include "logger.h"
#include "client/OAIAuthApi.h"
#include "client/OAIDefaultApi.h"
#include "client/OAIJobsApi.h"
#include "client/OAIHelpers.h"
#include <QString>
#include <iostream>
#include <QFile>

#include "networkmanager.h"

using namespace OpenAPI;

namespace gams {
namespace studio {
namespace engine {

EngineManager::EngineManager(QObject* parent)
    : QObject(parent), /*mAuthApi(new OAIAuthApi()),*/ mDefaultApi(new OAIDefaultApi()), mJobsApi(new OAIJobsApi()),
      mNetworkManager(NetworkManager::manager()), mQueueFinished(false)
{
//    mAuthApi->setScheme("https");
//    mAuthApi->setPort(443);
//    connect(mAuthApi, &OAIAuthApi::postLoginInterfaceSignal, this,
//            [this](OAIModel_auth_token summary) {
//        emit reAuth(summary.getToken());
//    });
//    connect(mAuthApi, &OAIAuthApi::postLoginInterfaceSignalEFull, this,
//            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError , QString) {
//        emit reError("From postW: "+worker->error_str);
//    });


    mDefaultApi->setNetworkAccessManager(mNetworkManager);
    mDefaultApi->setScheme("https");
    mDefaultApi->setPort(443);

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
            [this](OAIHttpRequestWorker *worker, QNetworkReply::NetworkError , QString ) {
        emit reVersionError(worker->error_str);
    });


    mJobsApi->setNetworkAccessManager(mNetworkManager);
    mJobsApi->setScheme("https");
    mJobsApi->setPort(443);

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

    connect(mJobsApi, &OAIJobsApi::abortRequestsSignal, this, &EngineManager::abortRequestsSignal);
    connect(this, &EngineManager::syncKillJob, this, &EngineManager::killJob, Qt::QueuedConnection);

}

EngineManager::~EngineManager()
{
    mJobsApi->abortRequests();
    mJobsApi->deleteLater();
}

void EngineManager::setWorkingDirectory(const QString &dir)
{
    mJobsApi->setWorkingDirectory(dir);
}

void EngineManager::setHost(const QString &host)
{
    mJobsApi->setHost(host);
    mDefaultApi->setHost(host);
}

void EngineManager::setPort(const int &port)
{
    mJobsApi->setPort(port);
    mDefaultApi->setPort(port);
}

void EngineManager::setBasePath(const QString &path)
{
    mJobsApi->setBasePath(path);
    mDefaultApi->setBasePath(path);
}

void EngineManager::setIgnoreSslErrors()
{
}

bool EngineManager::ignoreSslErrors()
{
    return false;
}

void EngineManager::authenticate(const QString &user, const QString &password)
{
    mUser = user;
    mPassword = password;

    QByteArray auth = "Basic " + (user + ":" + password).toLatin1().toBase64();
    mJobsApi->addHeaders("Authorization", auth);
    mDefaultApi->addHeaders("Authorization", auth);
}

void EngineManager::authenticate(const QString &userToken)
{
    Q_UNUSED(userToken)
    DEB() << "Authentication by token not implementet yet.";
    // TODO(JM) prepared authentication by user-token
}

void EngineManager::getVersion()
{
    mDefaultApi->getVersion();
}

void EngineManager::submitJob(QString modelName, QString nSpace, QString zipFile, QStringList params)
{
    OAIHttpFileElement model;
    model.setMimeType("application/zip");
    model.setFileName(zipFile);
    OAIHttpFileElement dummy;
    mJobsApi->createJob(modelName, nSpace, "solver.log", QStringList(), QStringList(), params, model, dummy, dummy);
}

void EngineManager::getJobStatus()
{
    if (!mToken.isEmpty())
        mJobsApi->getJob(mToken, "status process_status");
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

void EngineManager::abortRequestsSignal()
{

}

bool EngineManager::parseVersions(QByteArray json, QString &vEngine, QString &vGams)
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
    mJobsApi->abortRequests();
}

} // namespace engine
} // namespace studio
} // namespace gams
