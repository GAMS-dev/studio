/**
 * GAMS Engine
 * With GAMS Engine you can register and solve GAMS models. It has a namespace management system, so you can restrict your users to certain models.
 *
 * The version of the OpenAPI document: latest
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#ifndef OAI_OAIJobsApi_H
#define OAI_OAIJobsApi_H

#include "OAIHelpers.h"
#include "OAIHttpRequest.h"
#include "OAIServerConfiguration.h"

#include "OAIHttpFileElement.h"
#include "OAIJob.h"
#include "OAIJob_no_text_entry_page.h"
#include "OAILog_piece.h"
#include "OAIMessage.h"
#include "OAIMessage_and_token.h"
#include "OAIStatus_code_meaning.h"
#include "OAIStream_entry.h"
#include "OAIText_entry.h"
#include <QString>

#include <QObject>
#include <QByteArray>
#include <QStringList>
#include <QList>
#include <QNetworkAccessManager>

namespace OpenAPI {

class OAIJobsApi : public QObject {
    Q_OBJECT

public:
    OAIJobsApi(const int timeOut = 0);
    ~OAIJobsApi();

    void initializeServerConfigs();
    int setDefaultServerValue(int serverIndex,const QString &operation, const QString &variable,const QString &val);
    void setServerIndex(const QString &operation, int serverIndex);
    void setApiKey(const QString &apiKeyName, const QString &apiKey);
    void setBearerToken(const QString &token);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setTimeOut(const int timeOut);
    void setWorkingDirectory(const QString &path);
    void setNetworkAccessManager(QNetworkAccessManager* manager);
    int addServerConfiguration(const QString &operation, const QUrl &url, const QString &description = "", const QMap<QString, OAIServerVariable> &variables = QMap<QString, OAIServerVariable>());
    void setNewServerForAllOperations(const QUrl &url, const QString &description = "", const QMap<QString, OAIServerVariable> &variables =  QMap<QString, OAIServerVariable>());
    void setNewServer(const QString &operation, const QUrl &url, const QString &description = "", const QMap<QString, OAIServerVariable> &variables =  QMap<QString, OAIServerVariable>());
    void addHeaders(const QString &key, const QString &value);
    void enableRequestCompression();
    void enableResponseCompression();
    void abortRequests();
    QString getParamStylePrefix(QString style);
    QString getParamStyleSuffix(QString style);
    QString getParamStyleDelimiter(QString style, QString name, bool isExplode);

    /**
    * @param[in]  model QString [required]
    * @param[in]  r_namespace QString [required]
    * @param[in]  run QString [optional]
    * @param[in]  text_entries QList<QString> [optional]
    * @param[in]  stream_entries QList<QString> [optional]
    * @param[in]  stdout_filename QString [optional]
    * @param[in]  arguments QList<QString> [optional]
    * @param[in]  dep_tokens QList<QString> [optional]
    * @param[in]  labels QList<QString> [optional]
    * @param[in]  model_data OAIHttpFileElement [optional]
    * @param[in]  data OAIHttpFileElement [optional]
    * @param[in]  inex_file OAIHttpFileElement [optional]
    */
    void createJob(const QString &model, const QString &r_namespace, const ::OpenAPI::OptionalParam<QString> &run = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &text_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &stream_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &stdout_filename = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &arguments = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &dep_tokens = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &labels = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<OAIHttpFileElement> &model_data = ::OpenAPI::OptionalParam<OAIHttpFileElement>(), const ::OpenAPI::OptionalParam<OAIHttpFileElement> &data = ::OpenAPI::OptionalParam<OAIHttpFileElement>(), const ::OpenAPI::OptionalParam<OAIHttpFileElement> &inex_file = ::OpenAPI::OptionalParam<OAIHttpFileElement>());

    /**
    * @param[in]  token QString [required]
    */
    void deleteJobZip(const QString &token);

    /**
    * @param[in]  token QString [required]
    * @param[in]  x_fields QString [optional]
    */
    void getJob(const QString &token, const ::OpenAPI::OptionalParam<QString> &x_fields = ::OpenAPI::OptionalParam<QString>());

    /**
    * @param[in]  token QString [required]
    * @param[in]  entry_name QString [required]
    * @param[in]  start_position qint32 [optional]
    * @param[in]  length qint32 [optional]
    */
    void getJobTextEntry(const QString &token, const QString &entry_name, const ::OpenAPI::OptionalParam<qint32> &start_position = ::OpenAPI::OptionalParam<qint32>(), const ::OpenAPI::OptionalParam<qint32> &length = ::OpenAPI::OptionalParam<qint32>());

    /**
    * @param[in]  token QString [required]
    * @param[in]  entry_name QString [required]
    */
    void getJobTextEntryInfo(const QString &token, const QString &entry_name);

    /**
    * @param[in]  token QString [required]
    */
    void getJobZip(const QString &token);

    /**
    * @param[in]  token QString [required]
    */
    void getJobZipInfo(const QString &token);

    /**
    * @param[in]  x_fields QString [optional]
    */
    void getStatusCodes(const ::OpenAPI::OptionalParam<QString> &x_fields = ::OpenAPI::OptionalParam<QString>());

    /**
    * @param[in]  token QString [required]
    * @param[in]  hard_kill bool [optional]
    */
    void killJob(const QString &token, const ::OpenAPI::OptionalParam<bool> &hard_kill = ::OpenAPI::OptionalParam<bool>());

    /**
    * @param[in]  everyone bool [optional]
    * @param[in]  x_fields QString [optional]
    * @param[in]  page qint32 [optional]
    * @param[in]  per_page qint32 [optional]
    * @param[in]  order_by QString [optional]
    * @param[in]  order_asc bool [optional]
    * @param[in]  show_only_active bool [optional]
    */
    void listJobs(const ::OpenAPI::OptionalParam<bool> &everyone = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QString> &x_fields = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<qint32> &page = ::OpenAPI::OptionalParam<qint32>(), const ::OpenAPI::OptionalParam<qint32> &per_page = ::OpenAPI::OptionalParam<qint32>(), const ::OpenAPI::OptionalParam<QString> &order_by = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &order_asc = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &show_only_active = ::OpenAPI::OptionalParam<bool>());

    /**
    * @param[in]  token QString [required]
    */
    void popJobLogs(const QString &token);

    /**
    * @param[in]  token QString [required]
    * @param[in]  entry_name QString [required]
    */
    void popStreamEntry(const QString &token, const QString &entry_name);


private:
    QMap<QString,int> _serverIndices;
    QMap<QString,QList<OAIServerConfiguration>> _serverConfigs;
    QMap<QString, QString> _apiKeys;
    QString _bearerToken;
    QString _username;
    QString _password;
    int _timeOut;
    QString _workingDirectory;
    QNetworkAccessManager* _manager;
    QMap<QString, QString> defaultHeaders;
    bool isResponseCompressionEnabled;
    bool isRequestCompressionEnabled;

    void createJobCallback(OAIHttpRequestWorker *worker);
    void deleteJobZipCallback(OAIHttpRequestWorker *worker);
    void getJobCallback(OAIHttpRequestWorker *worker);
    void getJobTextEntryCallback(OAIHttpRequestWorker *worker);
    void getJobTextEntryInfoCallback(OAIHttpRequestWorker *worker);
    void getJobZipCallback(OAIHttpRequestWorker *worker);
    void getJobZipInfoCallback(OAIHttpRequestWorker *worker);
    void getStatusCodesCallback(OAIHttpRequestWorker *worker);
    void killJobCallback(OAIHttpRequestWorker *worker);
    void listJobsCallback(OAIHttpRequestWorker *worker);
    void popJobLogsCallback(OAIHttpRequestWorker *worker);
    void popStreamEntryCallback(OAIHttpRequestWorker *worker);

signals:

    void createJobSignal(OAIMessage_and_token summary);
    void deleteJobZipSignal(OAIMessage summary);
    void getJobSignal(OAIJob summary);
    void getJobTextEntrySignal(OAIText_entry summary);
    void getJobTextEntryInfoSignal();
    void getJobZipSignal(OAIHttpFileElement summary);
    void getJobZipInfoSignal();
    void getStatusCodesSignal(QList<OAIStatus_code_meaning> summary);
    void killJobSignal(OAIMessage summary);
    void listJobsSignal(OAIJob_no_text_entry_page summary);
    void popJobLogsSignal(OAILog_piece summary);
    void popStreamEntrySignal(OAIStream_entry summary);

    void createJobSignalFull(OAIHttpRequestWorker *worker, OAIMessage_and_token summary);
    void deleteJobZipSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void getJobSignalFull(OAIHttpRequestWorker *worker, OAIJob summary);
    void getJobTextEntrySignalFull(OAIHttpRequestWorker *worker, OAIText_entry summary);
    void getJobTextEntryInfoSignalFull(OAIHttpRequestWorker *worker);
    void getJobZipSignalFull(OAIHttpRequestWorker *worker, OAIHttpFileElement summary);
    void getJobZipInfoSignalFull(OAIHttpRequestWorker *worker);
    void getStatusCodesSignalFull(OAIHttpRequestWorker *worker, QList<OAIStatus_code_meaning> summary);
    void killJobSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void listJobsSignalFull(OAIHttpRequestWorker *worker, OAIJob_no_text_entry_page summary);
    void popJobLogsSignalFull(OAIHttpRequestWorker *worker, OAILog_piece summary);
    void popStreamEntrySignalFull(OAIHttpRequestWorker *worker, OAIStream_entry summary);

    void createJobSignalE(OAIMessage_and_token summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteJobZipSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getJobSignalE(OAIJob summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getJobTextEntrySignalE(OAIText_entry summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getJobTextEntryInfoSignalE(QNetworkReply::NetworkError error_type, QString error_str);
    void getJobZipSignalE(OAIHttpFileElement summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getJobZipInfoSignalE(QNetworkReply::NetworkError error_type, QString error_str);
    void getStatusCodesSignalE(QList<OAIStatus_code_meaning> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void killJobSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listJobsSignalE(OAIJob_no_text_entry_page summary, QNetworkReply::NetworkError error_type, QString error_str);
    void popJobLogsSignalE(OAILog_piece summary, QNetworkReply::NetworkError error_type, QString error_str);
    void popStreamEntrySignalE(OAIStream_entry summary, QNetworkReply::NetworkError error_type, QString error_str);

    void createJobSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteJobZipSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getJobSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getJobTextEntrySignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getJobTextEntryInfoSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getJobZipSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getJobZipInfoSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getStatusCodesSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void killJobSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listJobsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void popJobLogsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void popStreamEntrySignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);

    void abortRequestsSignal();
    void allPendingRequestsCompleted();
};

} // namespace OpenAPI
#endif
