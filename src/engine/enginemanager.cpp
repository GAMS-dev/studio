#include "enginemanager.h"
#include "client/OAIAuthApi.h"
#include "client/OAIJobsApi.h"
#include <QString>
#include <iostream>
#include <QFile>

using namespace OpenAPI;

namespace gams {
namespace studio {
namespace engine {

EngineManager::EngineManager(QObject* parent)
    : QObject(parent), mJobsApi(new OAIJobsApi())
{
    mAuthApi->setScheme("https");
    mAuthApi->setPort(443);
    mAuthApi->setBasePath("/engine/api");
    connect(mAuthApi, &OAIAuthApi::postWSignal,
            [this](OAIModel_auth_token summary) {
        emit reAuth(summary.getToken());
    });
    connect(mAuthApi, &OAIAuthApi::postWSignalE,
            [this](OAIModel_auth_token , QNetworkReply::NetworkError , QString error_str) {
        emit reError(error_str);
    });

    mJobsApi->setScheme("https");
    mJobsApi->setHost("miro.gams.com");
    mJobsApi->setPort(443);
    mJobsApi->setBasePath("/engine/api");
    // namespace = studiotests

    connect(mJobsApi, &OAIJobsApi::createJobSignal,
            [this](OAIMessage_and_token summary) {
        emit reCreateJob(summary.getMessage(), summary.getToken());
    });
    connect(mJobsApi, &OAIJobsApi::createJobSignalE,
            [this](OAIMessage_and_token, QNetworkReply::NetworkError, QString error_str) {
        emit reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::getJobSignal, [this](OAIJob summary) {
        emit reGetJobStatus(summary.getStatus(), summary.getProcessStatus());
    });
    connect(mJobsApi, &OAIJobsApi::getJobSignalE,
            [this](OAIJob, QNetworkReply::NetworkError, QString error_str) {
        emit reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::getJobZipSignalFull, [this](OAIHttpRequestWorker *worker) {
        emit reGetOutputFile(worker->response);
    });
    connect(mJobsApi, &OAIJobsApi::getJobZipSignalE,
            [this](QNetworkReply::NetworkError, QString error_str) {
        emit reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::killJobSignal, [this](OAIMessage summary) {
        reKillJob(summary.getMessage());
    });
    connect(mJobsApi, &OAIJobsApi::killJobSignalE,
            [this](OAIMessage, QNetworkReply::NetworkError, QString error_str) {
        emit reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::popJobLogsSignal, [this](OAILog_piece summary) {
        // -> jobs/{token}/unread-logs
        reGetLog(summary.getMessage().toUtf8());
    });
    connect(mJobsApi, &OAIJobsApi::popJobLogsSignalE,
            [this](OAILog_piece, QNetworkReply::NetworkError, QString error_str) {
        emit reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::abortRequestsSignal, this, &EngineManager::abortRequestsSignal);

}

void EngineManager::setHost(const QString &host)
{
    mJobsApi->setHost(host);
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
    mAuthApi->postW(mUser, mPassword);
}

void EngineManager::ping()
{

    mJobsApi->getJob("", "status");
}

//void EngineManager::version()
//{
//}

void EngineManager::submitJob(QString fileName, QString zipFile, QStringList params)
{
    OAIHttpFileElement model;
    model.setMimeType("application/zip");
    model.setFileName(zipFile);
    OAIHttpFileElement dummy;
    mJobsApi->createJob(fileName, "studiotests", "solver.log", QStringList(), QStringList(), params, model, dummy, dummy);
}

void EngineManager::getJobStatus()
{
    if (!mToken.isEmpty())
        mJobsApi->getJob(mToken, "status process_status");
}

void EngineManager::killJob(bool hard, bool &ok)
{
    ok = !mToken.isEmpty();
    if (ok)
        mJobsApi->killJob(mToken, hard);
}

void EngineManager::getLog()
{
    if (!mToken.isEmpty())
        mJobsApi->popJobLogs(mToken);
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

void EngineManager::getJobZipSignalE(QNetworkReply::NetworkError error_type, QString error_str)
{

}

void EngineManager::abortRequestsSignal()
{

}

void EngineManager::reCreateJob(QString message, QString token)
{

}

QString EngineManager::getToken() const
{
    return mToken;
}

void EngineManager::setToken(const QString &token)
{
    mToken = token;
}

} // namespace engine
} // namespace studio
} // namespace gams
