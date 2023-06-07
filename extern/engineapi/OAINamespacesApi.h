/**
 * GAMS Engine
 * With GAMS Engine you can register and solve GAMS models. It has a namespace management system, so you can restrict your users to certain models.
 *
 * The version of the OpenAPI document: 23.06.02
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#ifndef OAI_OAINamespacesApi_H
#define OAI_OAINamespacesApi_H

#include "OAIHelpers.h"
#include "OAIHttpRequest.h"
#include "OAIServerConfiguration.h"
#include "OAIOauth.h"

#include "OAIHttpFileElement.h"
#include "OAIMessage.h"
#include "OAIModels.h"
#include "OAINamespace.h"
#include "OAINamespace_quota.h"
#include "OAINamespace_with_permission.h"
#include "OAIPerm_and_username.h"
#include "OAIUser_groups.h"
#include <QString>

#include <QObject>
#include <QByteArray>
#include <QStringList>
#include <QList>
#include <QNetworkAccessManager>

namespace OpenAPI {

class OAINamespacesApi : public QObject {
    Q_OBJECT

public:
    OAINamespacesApi(const int timeOut = 0);
    ~OAINamespacesApi();

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
    * @param[in]  r_namespace QString [required]
    * @param[in]  label QString [required]
    * @param[in]  username QString [required]
    */
    void addUserToGroup(const QString &r_namespace, const QString &label, const QString &username);

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  model QString [required]
    * @param[in]  data OAIHttpFileElement [required]
    * @param[in]  inex_string QString [optional]
    * @param[in]  arguments QList<QString> [optional]
    * @param[in]  text_entries QList<QString> [optional]
    * @param[in]  stream_entries QList<QString> [optional]
    * @param[in]  run QString [optional]
    * @param[in]  protect_model_files bool [optional]
    * @param[in]  user_groups QList<QString> [optional]
    * @param[in]  inex_file OAIHttpFileElement [optional]
    */
    void createModel(const QString &r_namespace, const QString &model, const OAIHttpFileElement &data, const ::OpenAPI::OptionalParam<QString> &inex_string = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &arguments = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &text_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &stream_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &run = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &protect_model_files = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QList<QString>> &user_groups = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<OAIHttpFileElement> &inex_file = ::OpenAPI::OptionalParam<OAIHttpFileElement>());

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  model QString [required]
    * @param[in]  data OAIHttpFileElement [required]
    * @param[in]  inex_string QString [optional]
    * @param[in]  arguments QList<QString> [optional]
    * @param[in]  text_entries QList<QString> [optional]
    * @param[in]  stream_entries QList<QString> [optional]
    * @param[in]  run QString [optional]
    * @param[in]  protect_model_files bool [optional]
    * @param[in]  user_groups QList<QString> [optional]
    * @param[in]  inex_file OAIHttpFileElement [optional]
    */
    Q_DECL_DEPRECATED void createModelDeprecated(const QString &r_namespace, const QString &model, const OAIHttpFileElement &data, const ::OpenAPI::OptionalParam<QString> &inex_string = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &arguments = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &text_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &stream_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &run = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &protect_model_files = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QList<QString>> &user_groups = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<OAIHttpFileElement> &inex_file = ::OpenAPI::OptionalParam<OAIHttpFileElement>());

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  disk_quota qint32 [optional]
    */
    void createNamespace(const QString &r_namespace, const ::OpenAPI::OptionalParam<qint32> &disk_quota = ::OpenAPI::OptionalParam<qint32>());

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  label QString [required]
    */
    void createUserGroup(const QString &r_namespace, const QString &label);

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  model QString [required]
    */
    void deleteModel(const QString &r_namespace, const QString &model);

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  model QString [required]
    */
    Q_DECL_DEPRECATED void deleteModelDeprecated(const QString &r_namespace, const QString &model);

    /**
    * @param[in]  r_namespace QString [required]
    */
    void deleteNamespace(const QString &r_namespace);

    /**
    * @param[in]  r_namespace QString [required]
    */
    void deleteNamespaceQuota(const QString &r_namespace);

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  label QString [required]
    */
    void deleteUserGroup(const QString &r_namespace, const QString &label);


    Q_DECL_DEPRECATED void getAccessibleNamespaces();

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  model QString [required]
    */
    void getModel(const QString &r_namespace, const QString &model);

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  model QString [required]
    */
    Q_DECL_DEPRECATED void getModelDeprecated(const QString &r_namespace, const QString &model);

    /**
    * @param[in]  r_namespace QString [required]
    */
    Q_DECL_DEPRECATED void getMyPermissions(const QString &r_namespace);

    /**
    * @param[in]  r_namespace QString [required]
    */
    void getNamespaceQuota(const QString &r_namespace);

    /**
    * @param[in]  r_namespace QString [required]
    */
    void getUserGroups(const QString &r_namespace);

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  username QString [required]
    */
    void getUserPermission(const QString &r_namespace, const QString &username);

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  x_fields QString [optional]
    * @param[in]  model QString [optional]
    */
    void listModels(const QString &r_namespace, const ::OpenAPI::OptionalParam<QString> &x_fields = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QString> &model = ::OpenAPI::OptionalParam<QString>());


    void listNamespaces();

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  label QString [required]
    * @param[in]  username QString [required]
    */
    void removeUserFromGroup(const QString &r_namespace, const QString &label, const QString &username);

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  username QString [required]
    * @param[in]  permissions qint32 [required]
    */
    void replaceUserPermission(const QString &r_namespace, const QString &username, const qint32 &permissions);

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  model QString [required]
    * @param[in]  inex_string QString [optional]
    * @param[in]  arguments QList<QString> [optional]
    * @param[in]  run QString [optional]
    * @param[in]  protect_model_files bool [optional]
    * @param[in]  user_groups QList<QString> [optional]
    * @param[in]  text_entries QList<QString> [optional]
    * @param[in]  stream_entries QList<QString> [optional]
    * @param[in]  delete_inex_file bool [optional]
    * @param[in]  delete_arguments bool [optional]
    * @param[in]  delete_run bool [optional]
    * @param[in]  delete_user_groups bool [optional]
    * @param[in]  delete_text_entries bool [optional]
    * @param[in]  delete_stream_entries bool [optional]
    * @param[in]  data OAIHttpFileElement [optional]
    * @param[in]  inex_file OAIHttpFileElement [optional]
    */
    void updateModel(const QString &r_namespace, const QString &model, const ::OpenAPI::OptionalParam<QString> &inex_string = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &arguments = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &run = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &protect_model_files = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QList<QString>> &user_groups = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &text_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &stream_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<bool> &delete_inex_file = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_arguments = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_run = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_user_groups = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_text_entries = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_stream_entries = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<OAIHttpFileElement> &data = ::OpenAPI::OptionalParam<OAIHttpFileElement>(), const ::OpenAPI::OptionalParam<OAIHttpFileElement> &inex_file = ::OpenAPI::OptionalParam<OAIHttpFileElement>());

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  model QString [required]
    * @param[in]  inex_string QString [optional]
    * @param[in]  arguments QList<QString> [optional]
    * @param[in]  run QString [optional]
    * @param[in]  protect_model_files bool [optional]
    * @param[in]  user_groups QList<QString> [optional]
    * @param[in]  text_entries QList<QString> [optional]
    * @param[in]  stream_entries QList<QString> [optional]
    * @param[in]  delete_inex_file bool [optional]
    * @param[in]  delete_arguments bool [optional]
    * @param[in]  delete_run bool [optional]
    * @param[in]  delete_user_groups bool [optional]
    * @param[in]  delete_text_entries bool [optional]
    * @param[in]  delete_stream_entries bool [optional]
    * @param[in]  data OAIHttpFileElement [optional]
    * @param[in]  inex_file OAIHttpFileElement [optional]
    */
    Q_DECL_DEPRECATED void updateModelDeprecated(const QString &r_namespace, const QString &model, const ::OpenAPI::OptionalParam<QString> &inex_string = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &arguments = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &run = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &protect_model_files = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QList<QString>> &user_groups = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &text_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &stream_entries = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<bool> &delete_inex_file = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_arguments = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_run = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_user_groups = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_text_entries = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_stream_entries = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<OAIHttpFileElement> &data = ::OpenAPI::OptionalParam<OAIHttpFileElement>(), const ::OpenAPI::OptionalParam<OAIHttpFileElement> &inex_file = ::OpenAPI::OptionalParam<OAIHttpFileElement>());

    /**
    * @param[in]  r_namespace QString [required]
    * @param[in]  disk_quota qint32 [required]
    */
    void updateNamespaceQuota(const QString &r_namespace, const qint32 &disk_quota);


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

    void addUserToGroupCallback(OAIHttpRequestWorker *worker);
    void createModelCallback(OAIHttpRequestWorker *worker);
    void createModelDeprecatedCallback(OAIHttpRequestWorker *worker);
    void createNamespaceCallback(OAIHttpRequestWorker *worker);
    void createUserGroupCallback(OAIHttpRequestWorker *worker);
    void deleteModelCallback(OAIHttpRequestWorker *worker);
    void deleteModelDeprecatedCallback(OAIHttpRequestWorker *worker);
    void deleteNamespaceCallback(OAIHttpRequestWorker *worker);
    void deleteNamespaceQuotaCallback(OAIHttpRequestWorker *worker);
    void deleteUserGroupCallback(OAIHttpRequestWorker *worker);
    void getAccessibleNamespacesCallback(OAIHttpRequestWorker *worker);
    void getModelCallback(OAIHttpRequestWorker *worker);
    void getModelDeprecatedCallback(OAIHttpRequestWorker *worker);
    void getMyPermissionsCallback(OAIHttpRequestWorker *worker);
    void getNamespaceQuotaCallback(OAIHttpRequestWorker *worker);
    void getUserGroupsCallback(OAIHttpRequestWorker *worker);
    void getUserPermissionCallback(OAIHttpRequestWorker *worker);
    void listModelsCallback(OAIHttpRequestWorker *worker);
    void listNamespacesCallback(OAIHttpRequestWorker *worker);
    void removeUserFromGroupCallback(OAIHttpRequestWorker *worker);
    void replaceUserPermissionCallback(OAIHttpRequestWorker *worker);
    void updateModelCallback(OAIHttpRequestWorker *worker);
    void updateModelDeprecatedCallback(OAIHttpRequestWorker *worker);
    void updateNamespaceQuotaCallback(OAIHttpRequestWorker *worker);

signals:

    void addUserToGroupSignal(OAIMessage summary);
    void createModelSignal(OAIMessage summary);
    void createModelDeprecatedSignal(OAIMessage summary);
    void createNamespaceSignal(OAIMessage summary);
    void createUserGroupSignal(OAIMessage summary);
    void deleteModelSignal(OAIMessage summary);
    void deleteModelDeprecatedSignal(OAIMessage summary);
    void deleteNamespaceSignal(OAIMessage summary);
    void deleteNamespaceQuotaSignal(OAIMessage summary);
    void deleteUserGroupSignal(OAIMessage summary);
    void getAccessibleNamespacesSignal(QList<OAINamespace_with_permission> summary);
    void getModelSignal(OAIHttpFileElement summary);
    void getModelDeprecatedSignal(OAIHttpFileElement summary);
    void getMyPermissionsSignal(OAIPerm_and_username summary);
    void getNamespaceQuotaSignal(OAINamespace_quota summary);
    void getUserGroupsSignal(QList<OAIUser_groups> summary);
    void getUserPermissionSignal(OAIPerm_and_username summary);
    void listModelsSignal(QList<OAIModels> summary);
    void listNamespacesSignal(QList<OAINamespace> summary);
    void removeUserFromGroupSignal(OAIMessage summary);
    void replaceUserPermissionSignal(OAIMessage summary);
    void updateModelSignal(OAIMessage summary);
    void updateModelDeprecatedSignal(OAIMessage summary);
    void updateNamespaceQuotaSignal(OAIMessage summary);

    void addUserToGroupSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void createModelSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void createModelDeprecatedSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void createNamespaceSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void createUserGroupSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteModelSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteModelDeprecatedSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteNamespaceSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteNamespaceQuotaSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteUserGroupSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void getAccessibleNamespacesSignalFull(OAIHttpRequestWorker *worker, QList<OAINamespace_with_permission> summary);
    void getModelSignalFull(OAIHttpRequestWorker *worker, OAIHttpFileElement summary);
    void getModelDeprecatedSignalFull(OAIHttpRequestWorker *worker, OAIHttpFileElement summary);
    void getMyPermissionsSignalFull(OAIHttpRequestWorker *worker, OAIPerm_and_username summary);
    void getNamespaceQuotaSignalFull(OAIHttpRequestWorker *worker, OAINamespace_quota summary);
    void getUserGroupsSignalFull(OAIHttpRequestWorker *worker, QList<OAIUser_groups> summary);
    void getUserPermissionSignalFull(OAIHttpRequestWorker *worker, OAIPerm_and_username summary);
    void listModelsSignalFull(OAIHttpRequestWorker *worker, QList<OAIModels> summary);
    void listNamespacesSignalFull(OAIHttpRequestWorker *worker, QList<OAINamespace> summary);
    void removeUserFromGroupSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void replaceUserPermissionSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void updateModelSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void updateModelDeprecatedSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void updateNamespaceQuotaSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);

    void addUserToGroupSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void createModelSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void createModelDeprecatedSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void createNamespaceSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void createUserGroupSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteModelSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteModelDeprecatedSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteNamespaceSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteNamespaceQuotaSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteUserGroupSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getAccessibleNamespacesSignalE(QList<OAINamespace_with_permission> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getModelSignalE(OAIHttpFileElement summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getModelDeprecatedSignalE(OAIHttpFileElement summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getMyPermissionsSignalE(OAIPerm_and_username summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getNamespaceQuotaSignalE(OAINamespace_quota summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserGroupsSignalE(QList<OAIUser_groups> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserPermissionSignalE(OAIPerm_and_username summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listModelsSignalE(QList<OAIModels> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listNamespacesSignalE(QList<OAINamespace> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void removeUserFromGroupSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void replaceUserPermissionSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateModelSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateModelDeprecatedSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateNamespaceQuotaSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);

    void addUserToGroupSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void createModelSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void createModelDeprecatedSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void createNamespaceSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void createUserGroupSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteModelSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteModelDeprecatedSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteNamespaceSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteNamespaceQuotaSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteUserGroupSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getAccessibleNamespacesSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getModelSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getModelDeprecatedSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getMyPermissionsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getNamespaceQuotaSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserGroupsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getUserPermissionSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listModelsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listNamespacesSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void removeUserFromGroupSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void replaceUserPermissionSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateModelSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateModelDeprecatedSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateNamespaceQuotaSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);

    void abortRequestsSignal();
    void allPendingRequestsCompleted();

public slots:
    void tokenAvailable();
    
};

} // namespace OpenAPI
#endif
