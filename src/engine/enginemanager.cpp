#include "enginemanager.h"
#include "client/OAIJobsApi.h"
#include <QString>
#include <iostream>
#include <QFile>

namespace gams {
namespace studio {
namespace engine {

EngineManager::EngineManager(QObject* parent)
    : QObject(parent), mJobsApi(new OpenAPI::OAIJobsApi())
{
    mJobsApi->setScheme("https");
    mJobsApi->setHost("https://miro.gams.com/engine/api");
    mJobsApi->setScheme("studiotests");
    mUser = "studiotests";
    mPassword = "rercud-qinRa9-wagbew";

    connect(mJobsApi, &OpenAPI::OAIJobsApi::createJobSignal, [this](OpenAPI::OAIMessage_and_token summary) {
        reCreateJob(summary.getMessage(), summary.getToken());
    });
    connect(mJobsApi, &OpenAPI::OAIJobsApi::getJobSignal, [this](OpenAPI::OAIJob summary) {
        reGetJobStatus(summary.getStatus());
    });
    connect(mJobsApi, &OpenAPI::OAIJobsApi::getJobZipSignalFull, [this](OpenAPI::OAIHttpRequestWorker *worker) {
        reGetOutputFile(worker->response);
    });
    connect(mJobsApi, &OpenAPI::OAIJobsApi::getJobZipSignalE, this, &EngineManager::getJobZipSignalE);
    connect(mJobsApi, &OpenAPI::OAIJobsApi::killJobSignal, [this](OpenAPI::OAIMessage summary) {
        reKillJob(summary.getMessage());
    });
    connect(mJobsApi, &OpenAPI::OAIJobsApi::popStreamEntrySignal, [this](OpenAPI::OAIStream_entry summary) {
        reGetIntermediateResultsNonBlocking(summary.getEntryValue().toUtf8());
    });
    connect(mJobsApi, &OpenAPI::OAIJobsApi::popJobLogsSignal, [this](OpenAPI::OAILog_piece summary) {
        reGetIntermediateResultsNonBlocking(summary.getMessage().toUtf8());
    });
    connect(mJobsApi, &OpenAPI::OAIJobsApi::abortRequestsSignal, this, &EngineManager::abortRequestsSignal);

    QMetaEnum meta = QMetaEnum::fromType<ProcCall>();
    for (int i = 0; i < meta.keyCount(); ++i) {
        QString c = QString(meta.key(i));
        procCalls.insert(c.right(c.length()-1), ProcCall(meta.value(i)));
    }

}

void EngineManager::setUrl(const QString &url)
{
    mJobsApi->setHost(url);
}

void EngineManager::setIgnoreSslErrors()
{
//    mHttp.setIgnoreSslErrors();
}

bool EngineManager::ignoreSslErrors()
{
//    return mHttp.ignoreSslErrors();
    return false;
}

void EngineManager::ping()
{
//    emit submitCall("ping");
}

void EngineManager::version()
{
//    emit submitCall("version");
}

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
    //  "Done", "Running", "Waiting", "Unknown Job", or "Bad Password"
//    emit submitCall("getJobStatus", QVariantList() << mJobNumber << mPassword);
}

void EngineManager::getCompletionCode()
{
    // Only if Job is "Done":
    //  "Normal", "Out of memory", "Timed out", "Disk Space", "Server error", "Unknown Job", "Bad Password"
//    emit submitCall("getCompletionCode", QVariantList() << mJobNumber << mPassword);
}

void EngineManager::getJobInfo()
{
    // tuple (category, solver_name, input, status, completion_code)
//    emit submitCall("getJobInfo", QVariantList() << mJobNumber << mPassword);
}

void EngineManager::killJob(bool &ok)
{
//    if ((ok = mJobNumber))
//        emit submitCall("killJob", QVariantList() << mJobNumber << mPassword);
}

void EngineManager::getIntermediateResultsNonBlocking()
{
//    emit submitCall("getIntermediateResultsNonBlocking", QVariantList() << mJobNumber << mPassword << mLogOffset);
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

} // namespace engine
} // namespace studio
} // namespace gams
