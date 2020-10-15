#include "enginemanager.h"
#include "logger.h"
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
    : QObject(parent), /*mAuthApi(new OAIAuthApi()),*/ mJobsApi(new OAIJobsApi()), mQueueFinished(false)
{
//    mAuthApi->setScheme("https");
//    mAuthApi->setPort(443);
//    connect(mAuthApi, &OAIAuthApi::postLoginInterfaceSignal,
//            [this](OAIModel_auth_token summary) {
//        emit reAuth(summary.getToken());
//    });
//    connect(mAuthApi, &OAIAuthApi::postLoginInterfaceSignalE,
//            [this](OAIModel_auth_token , QNetworkReply::NetworkError , QString error_str) {
//        emit reError("From postW: "+error_str);
//    });

    mJobsApi->setScheme("https");
    mJobsApi->setPort(443);

    connect(mJobsApi, &OAIJobsApi::createJobSignal,
            [this](OAIMessage_and_token summary) {
        emit reCreateJob(summary.getMessage(), summary.getToken());
    });
    connect(mJobsApi, &OAIJobsApi::createJobSignalE,
            [this](OAIMessage_and_token, QNetworkReply::NetworkError error_type, QString error_str) {
        emit reError("Code "+QString::number(error_type).toLatin1()+" from createJob: "+error_str);
    });

    connect(mJobsApi, &OAIJobsApi::getJobSignal, [this](OAIJob summary) {
        emit reGetJobStatus(summary.getStatus(), summary.getProcessStatus());
    });
    connect(mJobsApi, &OAIJobsApi::getJobSignalE,
            [this](OAIJob, QNetworkReply::NetworkError error_type, QString error_str) {
        emit reError("Code "+QString::number(error_type).toLatin1()+" from getJob: "+error_str);
    });

    connect(mJobsApi, &OAIJobsApi::getJobZipSignal, [this](OAIHttpFileElement summary) {
        emit reGetOutputFile(summary.asByteArray());
    });
    connect(mJobsApi, &OAIJobsApi::getJobZipSignalE,
            [this](OAIHttpFileElement, QNetworkReply::NetworkError error_type, QString error_str) {
        emit reError("Code "+QString::number(error_type).toLatin1()+" from getJobZip: "+error_str);
    });

    connect(mJobsApi, &OAIJobsApi::killJobSignal, [this](OAIMessage summary) {
        emit reKillJob(summary.getMessage());
    });
    connect(mJobsApi, &OAIJobsApi::killJobSignalE,
            [this](OAIMessage, QNetworkReply::NetworkError error_type, QString error_str) {
        emit reError("Code "+QString::number(error_type).toLatin1()+" from killJob: "+error_str);
    });

    connect(mJobsApi, &OAIJobsApi::popJobLogsSignal, [this](OAILog_piece summary) {
        if (!mQueueFinished) {
            mQueueFinished = summary.isQueueFinished();
            emit reGetLog(summary.getMessage().toUtf8());
        }
    });
    connect(mJobsApi, &OAIJobsApi::popJobLogsSignalE,
            [this](OAILog_piece, QNetworkReply::NetworkError error_type, QString error_str) {
        if (error_type != QNetworkReply::ServiceUnavailableError)
            emit reGetLog("Code "+QString::number(error_type).toLatin1()+" from popLog: "+error_str.toUtf8());
    });

    connect(mJobsApi, &OAIJobsApi::abortRequestsSignal, this, &EngineManager::abortRequestsSignal);
    connect(this, &EngineManager::syncKillJob, this, &EngineManager::killJob, Qt::QueuedConnection);

}

void EngineManager::setWorkingDirectory(const QString &dir)
{
    mJobsApi->setWorkingDirectory(dir);
}

void EngineManager::setUrl(const QString &url)
{
    int sp1 = url.indexOf("://")+1;
    if (sp1) sp1 += 2;
    int sp2 = url.indexOf('/', sp1);
    if (sp2 < 0) sp2 = url.length();
    mJobsApi->setHost(url.mid(sp1, sp2-sp1));
    mJobsApi->setBasePath(url.right(url.length()-sp2));
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
}

void EngineManager::authenticate(const QString &userToken)
{
    Q_UNUSED(userToken)
    DEB() << "Authentication by token not implementet yet.";
    // TODO(JM) prepared authentication by user-token
}

void EngineManager::ping()
{

    mJobsApi->getJob("", "status");
}

//void EngineManager::version()
//{
//}

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
