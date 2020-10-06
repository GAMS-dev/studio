#include "enginemanager.h"
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
    mJobsApi->setScheme("https");
    mJobsApi->setHost("https://miro.gams.com/engine/api");
    mJobsApi->setPort(443);
    mJobsApi->setBasePath("studiotests");
    mUser = "studiotests";
    mPassword = "rercud-qinRa9-wagbew";

    connect(mJobsApi, &OAIJobsApi::createJobSignal,
            [this](OAIMessage_and_token summary) {
        reCreateJob(summary.getMessage(), summary.getToken());
    });
    connect(mJobsApi, &OAIJobsApi::createJobSignalE,
            [this](OAIMessage_and_token, QNetworkReply::NetworkError, QString error_str) {
        reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::getJobSignal, [this](OAIJob summary) {
        reGetJobStatus(summary.getStatus(), summary.getProcessStatus());
    });
    connect(mJobsApi, &OAIJobsApi::getJobSignalE,
            [this](OAIJob, QNetworkReply::NetworkError, QString error_str) {
        reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::getJobZipSignalFull, [this](OAIHttpRequestWorker *worker) {
        reGetOutputFile(worker->response);
    });
    connect(mJobsApi, &OAIJobsApi::getJobZipSignalE,
            [this](QNetworkReply::NetworkError, QString error_str) {
        reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::killJobSignal, [this](OAIMessage summary) {
        reKillJob(summary.getMessage());
    });
    connect(mJobsApi, &OAIJobsApi::killJobSignalE,
            [this](OAIMessage, QNetworkReply::NetworkError, QString error_str) {
        reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::popJobLogsSignal, [this](OAILog_piece summary) {
        // -> jobs/{token}/unread-logs
        reGetLog(summary.getMessage().toUtf8());
    });
    connect(mJobsApi, &OAIJobsApi::popJobLogsSignalE,
            [this](OAILog_piece, QNetworkReply::NetworkError, QString error_str) {
        reError(error_str);
    });

    connect(mJobsApi, &OAIJobsApi::abortRequestsSignal, this, &EngineManager::abortRequestsSignal);

}

void EngineManager::setUrl(const QString &url)
{
    mJobsApi->setHost(url);
}

void EngineManager::setIgnoreSslErrors()
{
}

bool EngineManager::ignoreSslErrors()
{
    return false;
}

void EngineManager::ping()
{
    mJobsApi->getJob("", "status");
}

//void EngineManager::version()
//{
//}

void EngineManager::submitJob(QString fileName, QString params)
{
    QFile f(fileName);
    if (!f.exists() || !f.open(QFile::ReadOnly)) return;
    QByteArray data = f.readAll();
    f.close();
    mLogOffset = 0;
    QString sData = data.toBase64();

//    emit submitCall("submitJob", QVariantList() << jobData);
}

void EngineManager::watchJob(int jobNumber, QString password)
{
    mJobNumber = jobNumber;
    mPassword = password;
    getJobStatus();
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

void EngineManager::getFinalResultsNonBlocking()
{
//    emit submitCall("getFinalResultsNonBlocking", QVariantList() << mJobNumber << mPassword);
}

void EngineManager::getOutputFile(QString fileName)
{
//    emit submitCall("getOutputFile", QVariantList() << mJobNumber << mPassword << fileName);
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
