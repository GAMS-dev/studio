/**
 * GAMS Engine
 * With GAMS Engine you can register and solve GAMS models. It has a namespace management system, so you can restrict your users to certain models.
 *
 * The version of the OpenAPI document: 25.04.23
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#ifndef OAI_OAIUsageApi_H
#define OAI_OAIUsageApi_H

#include "OAIHelpers.h"
#include "OAIHttpRequest.h"
#include "OAIServerConfiguration.h"
#include "OAIOauth.h"

#include "OAIBad_input.h"
#include "OAIMessage.h"
#include "OAIModel_default_user_instance.h"
#include "OAIModel_instance_info_full.h"
#include "OAIModel_usage.h"
#include "OAIModel_userinstance_info.h"
#include "OAIModel_userinstancepool_info.h"
#include "OAIQuota.h"
#include <QString>

#include <QObject>
#include <QByteArray>
#include <QStringList>
#include <QList>
#include <QNetworkAccessManager>

namespace OpenAPI {

class OAIUsageApi : public QObject {
    Q_OBJECT

public:
    OAIUsageApi(const int timeOut = 0);
    ~OAIUsageApi();

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
    QString getParamStylePrefix(const QString &style);
    QString getParamStyleSuffix(const QString &style);
    QString getParamStyleDelimiter(const QString &style, const QString &name, bool isExplode);

    /**
    * @param[in]  label QString [required]
    * @param[in]  cpu_request double [required]
    * @param[in]  memory_request qint32 [required]
    * @param[in]  multiplier_idle double [required]
    * @param[in]  multiplier double [required]
    * @param[in]  workspace_request qint32 [required]
    * @param[in]  tolerations QList<QString> [optional]
    * @param[in]  node_selectors QList<QString> [optional]
    * @param[in]  gams_license QString [optional]
    */
    virtual void createInstance(const QString &label, const double &cpu_request, const qint32 &memory_request, const double &multiplier_idle, const double &multiplier, const qint32 &workspace_request, const ::OpenAPI::OptionalParam<QList<QString>> &tolerations = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &node_selectors = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &gams_license = ::OpenAPI::OptionalParam<QString>());

    /**
    * @param[in]  label QString [required]
    * @param[in]  instance QString [required]
    * @param[in]  size qint32 [required]
    */
    virtual void createInstancePool(const QString &label, const QString &instance, const qint32 &size);

    /**
    * @param[in]  label QString [required]
    */
    virtual void deleteInstance(const QString &label);

    /**
    * @param[in]  label QString [required]
    */
    virtual void deleteInstancePool(const QString &label);

    /**
    * @param[in]  username QString [required]
    * @param[in]  field QString [optional]
    */
    virtual void deleteQuota(const QString &username, const ::OpenAPI::OptionalParam<QString> &field = ::OpenAPI::OptionalParam<QString>());

    /**
    * @param[in]  username QString [required]
    */
    virtual void deleteUserInstances(const QString &username);


    virtual void getInstancePools();


    virtual void getInstances();

    /**
    * @param[in]  username QString [required]
    */
    virtual void getQuota(const QString &username);

    /**
    * @param[in]  username QString [required]
    * @param[in]  recursive bool [optional]
    * @param[in]  from_datetime QDateTime [optional]
    * @param[in]  to_datetime QDateTime [optional]
    * @param[in]  x_fields QString [optional]
    * @param[in]  token QList<QString> [optional]
    * @param[in]  hypercube_token QList<QString> [optional]
    */
    virtual void getUsage(const QString &username, const ::OpenAPI::OptionalParam<bool> &recursive = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QDateTime> &from_datetime = ::OpenAPI::OptionalParam<QDateTime>(), const ::OpenAPI::OptionalParam<QDateTime> &to_datetime = ::OpenAPI::OptionalParam<QDateTime>(), const ::OpenAPI::OptionalParam<QString> &x_fields = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &token = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &hypercube_token = ::OpenAPI::OptionalParam<QList<QString>>());

    /**
    * @param[in]  username QString [required]
    */
    virtual void getUserDefaultInstance(const QString &username);

    /**
    * @param[in]  username QString [required]
    */
    virtual void getUserInstancePools(const QString &username);

    /**
    * @param[in]  username QString [required]
    */
    virtual void getUserInstances(const QString &username);

    /**
    * @param[in]  label QString [required]
    * @param[in]  cpu_request double [required]
    * @param[in]  memory_request qint32 [required]
    * @param[in]  multiplier_idle double [required]
    * @param[in]  multiplier double [required]
    * @param[in]  workspace_request qint32 [required]
    * @param[in]  old_label QString [required]
    * @param[in]  tolerations QList<QString> [optional]
    * @param[in]  node_selectors QList<QString> [optional]
    * @param[in]  gams_license QString [optional]
    */
    virtual void updateInstance(const QString &label, const double &cpu_request, const qint32 &memory_request, const double &multiplier_idle, const double &multiplier, const qint32 &workspace_request, const QString &old_label, const ::OpenAPI::OptionalParam<QList<QString>> &tolerations = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &node_selectors = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &gams_license = ::OpenAPI::OptionalParam<QString>());

    /**
    * @param[in]  label QString [required]
    * @param[in]  size qint32 [required]
    */
    virtual void updateInstancePool(const QString &label, const qint32 &size);

    /**
    * @param[in]  username QString [required]
    * @param[in]  parallel_quota double [optional]
    * @param[in]  volume_quota double [optional]
    * @param[in]  disk_quota qint32 [optional]
    */
    virtual void updateQuota(const QString &username, const ::OpenAPI::OptionalParam<double> &parallel_quota = ::OpenAPI::OptionalParam<double>(), const ::OpenAPI::OptionalParam<double> &volume_quota = ::OpenAPI::OptionalParam<double>(), const ::OpenAPI::OptionalParam<qint32> &disk_quota = ::OpenAPI::OptionalParam<qint32>());

    /**
    * @param[in]  username QString [required]
    * @param[in]  default_label QString [required]
    */
    virtual void updateUserDefaultInstance(const QString &username, const QString &default_label);

    /**
    * @param[in]  username QString [required]
    * @param[in]  labels QList<QString> [optional]
    * @param[in]  default_label QString [optional]
    * @param[in]  delete_pools bool [optional]
    */
    virtual void updateUserInstances(const QString &username, const ::OpenAPI::OptionalParam<QList<QString>> &labels = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &default_label = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &delete_pools = ::OpenAPI::OptionalParam<bool>());


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
    QMap<QString, QString> _defaultHeaders;
    bool _isResponseCompressionEnabled;
    bool _isRequestCompressionEnabled;
    OAIHttpRequestInput _latestInput;
    OAIHttpRequestWorker *_latestWorker;
    QStringList _latestScope;
    OauthCode _authFlow;
    OauthImplicit _implicitFlow;
    OauthCredentials _credentialFlow;
    OauthPassword _passwordFlow;
    int _OauthMethod = 0;

    void createInstanceCallback(OAIHttpRequestWorker *worker);
    void createInstancePoolCallback(OAIHttpRequestWorker *worker);
    void deleteInstanceCallback(OAIHttpRequestWorker *worker);
    void deleteInstancePoolCallback(OAIHttpRequestWorker *worker);
    void deleteQuotaCallback(OAIHttpRequestWorker *worker);
    void deleteUserInstancesCallback(OAIHttpRequestWorker *worker);
    void getInstancePoolsCallback(OAIHttpRequestWorker *worker);
    void getInstancesCallback(OAIHttpRequestWorker *worker);
    void getQuotaCallback(OAIHttpRequestWorker *worker);
    void getUsageCallback(OAIHttpRequestWorker *worker);
    void getUserDefaultInstanceCallback(OAIHttpRequestWorker *worker);
    void getUserInstancePoolsCallback(OAIHttpRequestWorker *worker);
    void getUserInstancesCallback(OAIHttpRequestWorker *worker);
    void updateInstanceCallback(OAIHttpRequestWorker *worker);
    void updateInstancePoolCallback(OAIHttpRequestWorker *worker);
    void updateQuotaCallback(OAIHttpRequestWorker *worker);
    void updateUserDefaultInstanceCallback(OAIHttpRequestWorker *worker);
    void updateUserInstancesCallback(OAIHttpRequestWorker *worker);

Q_SIGNALS:

    void createInstanceSignal(OAIMessage summary);
    void createInstancePoolSignal(OAIMessage summary);
    void deleteInstanceSignal(OAIMessage summary);
    void deleteInstancePoolSignal(OAIMessage summary);
    void deleteQuotaSignal(OAIMessage summary);
    void deleteUserInstancesSignal(OAIMessage summary);
    void getInstancePoolsSignal(OAIModel_userinstancepool_info summary);
    void getInstancesSignal(QList<OAIModel_instance_info_full> summary);
    void getQuotaSignal(QList<OAIQuota> summary);
    void getUsageSignal(OAIModel_usage summary);
    void getUserDefaultInstanceSignal(OAIModel_default_user_instance summary);
    void getUserInstancePoolsSignal(OAIModel_userinstancepool_info summary);
    void getUserInstancesSignal(OAIModel_userinstance_info summary);
    void updateInstanceSignal(OAIMessage summary);
    void updateInstancePoolSignal(OAIMessage summary);
    void updateQuotaSignal(OAIMessage summary);
    void updateUserDefaultInstanceSignal(OAIMessage summary);
    void updateUserInstancesSignal(OAIMessage summary);


    void createInstanceSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void createInstancePoolSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteInstanceSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteInstancePoolSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteQuotaSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteUserInstancesSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void getInstancePoolsSignalFull(OAIHttpRequestWorker *worker, OAIModel_userinstancepool_info summary);
    void getInstancesSignalFull(OAIHttpRequestWorker *worker, QList<OAIModel_instance_info_full> summary);
    void getQuotaSignalFull(OAIHttpRequestWorker *worker, QList<OAIQuota> summary);
    void getUsageSignalFull(OAIHttpRequestWorker *worker, OAIModel_usage summary);
    void getUserDefaultInstanceSignalFull(OAIHttpRequestWorker *worker, OAIModel_default_user_instance summary);
    void getUserInstancePoolsSignalFull(OAIHttpRequestWorker *worker, OAIModel_userinstancepool_info summary);
    void getUserInstancesSignalFull(OAIHttpRequestWorker *worker, OAIModel_userinstance_info summary);
    void updateInstanceSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void updateInstancePoolSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void updateQuotaSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void updateUserDefaultInstanceSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void updateUserInstancesSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);

    Q_DECL_DEPRECATED_X("Use createInstanceSignalError() instead")
    void createInstanceSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void createInstanceSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use createInstancePoolSignalError() instead")
    void createInstancePoolSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void createInstancePoolSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use deleteInstanceSignalError() instead")
    void deleteInstanceSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteInstanceSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use deleteInstancePoolSignalError() instead")
    void deleteInstancePoolSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteInstancePoolSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use deleteQuotaSignalError() instead")
    void deleteQuotaSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteQuotaSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use deleteUserInstancesSignalError() instead")
    void deleteUserInstancesSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteUserInstancesSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getInstancePoolsSignalError() instead")
    void getInstancePoolsSignalE(OAIModel_userinstancepool_info summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getInstancePoolsSignalError(OAIModel_userinstancepool_info summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getInstancesSignalError() instead")
    void getInstancesSignalE(QList<OAIModel_instance_info_full> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getInstancesSignalError(QList<OAIModel_instance_info_full> summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getQuotaSignalError() instead")
    void getQuotaSignalE(QList<OAIQuota> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getQuotaSignalError(QList<OAIQuota> summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getUsageSignalError() instead")
    void getUsageSignalE(OAIModel_usage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getUsageSignalError(OAIModel_usage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getUserDefaultInstanceSignalError() instead")
    void getUserDefaultInstanceSignalE(OAIModel_default_user_instance summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserDefaultInstanceSignalError(OAIModel_default_user_instance summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getUserInstancePoolsSignalError() instead")
    void getUserInstancePoolsSignalE(OAIModel_userinstancepool_info summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserInstancePoolsSignalError(OAIModel_userinstancepool_info summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getUserInstancesSignalError() instead")
    void getUserInstancesSignalE(OAIModel_userinstance_info summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserInstancesSignalError(OAIModel_userinstance_info summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateInstanceSignalError() instead")
    void updateInstanceSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateInstanceSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateInstancePoolSignalError() instead")
    void updateInstancePoolSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateInstancePoolSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateQuotaSignalError() instead")
    void updateQuotaSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateQuotaSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateUserDefaultInstanceSignalError() instead")
    void updateUserDefaultInstanceSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateUserDefaultInstanceSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateUserInstancesSignalError() instead")
    void updateUserInstancesSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateUserInstancesSignalError(OAIMessage summary, QNetworkReply::NetworkError error_type, const QString &error_str);

    Q_DECL_DEPRECATED_X("Use createInstanceSignalErrorFull() instead")
    void createInstanceSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void createInstanceSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use createInstancePoolSignalErrorFull() instead")
    void createInstancePoolSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void createInstancePoolSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use deleteInstanceSignalErrorFull() instead")
    void deleteInstanceSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteInstanceSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use deleteInstancePoolSignalErrorFull() instead")
    void deleteInstancePoolSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteInstancePoolSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use deleteQuotaSignalErrorFull() instead")
    void deleteQuotaSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteQuotaSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use deleteUserInstancesSignalErrorFull() instead")
    void deleteUserInstancesSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteUserInstancesSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getInstancePoolsSignalErrorFull() instead")
    void getInstancePoolsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getInstancePoolsSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getInstancesSignalErrorFull() instead")
    void getInstancesSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getInstancesSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getQuotaSignalErrorFull() instead")
    void getQuotaSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getQuotaSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getUsageSignalErrorFull() instead")
    void getUsageSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getUsageSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getUserDefaultInstanceSignalErrorFull() instead")
    void getUserDefaultInstanceSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserDefaultInstanceSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getUserInstancePoolsSignalErrorFull() instead")
    void getUserInstancePoolsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserInstancePoolsSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use getUserInstancesSignalErrorFull() instead")
    void getUserInstancesSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserInstancesSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateInstanceSignalErrorFull() instead")
    void updateInstanceSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateInstanceSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateInstancePoolSignalErrorFull() instead")
    void updateInstancePoolSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateInstancePoolSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateQuotaSignalErrorFull() instead")
    void updateQuotaSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateQuotaSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateUserDefaultInstanceSignalErrorFull() instead")
    void updateUserDefaultInstanceSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateUserDefaultInstanceSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);
    Q_DECL_DEPRECATED_X("Use updateUserInstancesSignalErrorFull() instead")
    void updateUserInstancesSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateUserInstancesSignalErrorFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, const QString &error_str);

    void abortRequestsSignal();
    void allPendingRequestsCompleted();

public Q_SLOTS:
    void tokenAvailable();
};

} // namespace OpenAPI
#endif
