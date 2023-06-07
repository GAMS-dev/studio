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

#ifndef OAI_OAIUsersApi_H
#define OAI_OAIUsersApi_H

#include "OAIHelpers.h"
#include "OAIHttpRequest.h"
#include "OAIServerConfiguration.h"
#include "OAIOauth.h"

#include "OAIBad_input.h"
#include "OAIIdentity_provider.h"
#include "OAIInvitation.h"
#include "OAIInvitation_token.h"
#include "OAIMessage.h"
#include "OAINot_found.h"
#include "OAIUser.h"
#include "OAIWebhook.h"
#include <QString>

#include <QObject>
#include <QByteArray>
#include <QStringList>
#include <QList>
#include <QNetworkAccessManager>

namespace OpenAPI {

class OAIUsersApi : public QObject {
    Q_OBJECT

public:
    OAIUsersApi(const int timeOut = 0);
    ~OAIUsersApi();

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
    * @param[in]  username QString [required]
    * @param[in]  name QList<QString> [optional]
    */
    void assignIdentityProvidersToInviter(const QString &username, const ::OpenAPI::OptionalParam<QList<QString>> &name = ::OpenAPI::OptionalParam<QList<QString>>());

    /**
    * @param[in]  roles QList<QString> [optional]
    * @param[in]  namespace_permissions QList<QString> [optional]
    * @param[in]  parallel_quota double [optional]
    * @param[in]  volume_quota double [optional]
    * @param[in]  disk_quota qint32 [optional]
    * @param[in]  labels QList<QString> [optional]
    * @param[in]  default_label QString [optional]
    * @param[in]  inherit_instances bool [optional]
    * @param[in]  user_groups QList<QString> [optional]
    * @param[in]  gams_license QString [optional]
    * @param[in]  identity_provider_name QString [optional]
    * @param[in]  identity_provider_user_subject QString [optional]
    * @param[in]  invitable_identity_providers QList<QString> [optional]
    */
    void createInvitation(const ::OpenAPI::OptionalParam<QList<QString>> &roles = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &namespace_permissions = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<double> &parallel_quota = ::OpenAPI::OptionalParam<double>(), const ::OpenAPI::OptionalParam<double> &volume_quota = ::OpenAPI::OptionalParam<double>(), const ::OpenAPI::OptionalParam<qint32> &disk_quota = ::OpenAPI::OptionalParam<qint32>(), const ::OpenAPI::OptionalParam<QList<QString>> &labels = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &default_label = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &inherit_instances = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QList<QString>> &user_groups = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QString> &gams_license = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QString> &identity_provider_name = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QString> &identity_provider_user_subject = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &invitable_identity_providers = ::OpenAPI::OptionalParam<QList<QString>>());

    /**
    * @param[in]  url QString [required]
    * @param[in]  content_type QString [optional]
    * @param[in]  secret QString [optional]
    * @param[in]  recursive bool [optional]
    * @param[in]  events QList<QString> [optional]
    * @param[in]  parameterized_events QList<QString> [optional]
    * @param[in]  insecure_ssl bool [optional]
    */
    void createWebhook(const QString &url, const ::OpenAPI::OptionalParam<QString> &content_type = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QString> &secret = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &recursive = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QList<QString>> &events = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<QList<QString>> &parameterized_events = ::OpenAPI::OptionalParam<QList<QString>>(), const ::OpenAPI::OptionalParam<bool> &insecure_ssl = ::OpenAPI::OptionalParam<bool>());

    /**
    * @param[in]  token QString [required]
    */
    void deleteInvitation(const QString &token);

    /**
    * @param[in]  username QString [required]
    * @param[in]  delete_results bool [optional]
    * @param[in]  delete_children bool [optional]
    */
    void deleteUser(const QString &username, const ::OpenAPI::OptionalParam<bool> &delete_results = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<bool> &delete_children = ::OpenAPI::OptionalParam<bool>());

    /**
    * @param[in]  id qint32 [required]
    */
    void deleteWebhook(const qint32 &id);

    /**
    * @param[in]  token QString [required]
    */
    void getInvitationMetadata(const QString &token);

    /**
    * @param[in]  username QString [required]
    */
    void listIdentityProvidersOfInviter(const QString &username);

    /**
    * @param[in]  token QString [optional]
    * @param[in]  everyone bool [optional]
    * @param[in]  x_fields QString [optional]
    * @param[in]  filter QList<QString> [optional]
    */
    void listInvitations(const ::OpenAPI::OptionalParam<QString> &token = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &everyone = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QString> &x_fields = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &filter = ::OpenAPI::OptionalParam<QList<QString>>());

    /**
    * @param[in]  username QString [optional]
    * @param[in]  everyone bool [optional]
    * @param[in]  x_fields QString [optional]
    * @param[in]  filter QList<QString> [optional]
    */
    void listUsers(const ::OpenAPI::OptionalParam<QString> &username = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<bool> &everyone = ::OpenAPI::OptionalParam<bool>(), const ::OpenAPI::OptionalParam<QString> &x_fields = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QList<QString>> &filter = ::OpenAPI::OptionalParam<QList<QString>>());


    void listWebhooks();

    /**
    * @param[in]  username QString [required]
    * @param[in]  invitation_code QString [required]
    * @param[in]  password QString [optional]
    * @param[in]  identification_token QString [optional]
    */
    void r_register(const QString &username, const QString &invitation_code, const ::OpenAPI::OptionalParam<QString> &password = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QString> &identification_token = ::OpenAPI::OptionalParam<QString>());

    /**
    * @param[in]  username QString [required]
    * @param[in]  new_username QString [required]
    */
    void replaceUserName(const QString &username, const QString &new_username);

    /**
    * @param[in]  username QString [required]
    * @param[in]  password QString [required]
    */
    void replaceUserPassword(const QString &username, const QString &password);

    /**
    * @param[in]  username QString [required]
    * @param[in]  identity_provider_name QString [optional]
    * @param[in]  identity_provider_user_subject QString [optional]
    * @param[in]  password QString [optional]
    */
    void updateUserIdentityProvider(const QString &username, const ::OpenAPI::OptionalParam<QString> &identity_provider_name = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QString> &identity_provider_user_subject = ::OpenAPI::OptionalParam<QString>(), const ::OpenAPI::OptionalParam<QString> &password = ::OpenAPI::OptionalParam<QString>());

    /**
    * @param[in]  username QString [required]
    * @param[in]  roles QList<QString> [optional]
    */
    void updateUserRole(const QString &username, const ::OpenAPI::OptionalParam<QList<QString>> &roles = ::OpenAPI::OptionalParam<QList<QString>>());


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

    void assignIdentityProvidersToInviterCallback(OAIHttpRequestWorker *worker);
    void createInvitationCallback(OAIHttpRequestWorker *worker);
    void createWebhookCallback(OAIHttpRequestWorker *worker);
    void deleteInvitationCallback(OAIHttpRequestWorker *worker);
    void deleteUserCallback(OAIHttpRequestWorker *worker);
    void deleteWebhookCallback(OAIHttpRequestWorker *worker);
    void getInvitationMetadataCallback(OAIHttpRequestWorker *worker);
    void listIdentityProvidersOfInviterCallback(OAIHttpRequestWorker *worker);
    void listInvitationsCallback(OAIHttpRequestWorker *worker);
    void listUsersCallback(OAIHttpRequestWorker *worker);
    void listWebhooksCallback(OAIHttpRequestWorker *worker);
    void r_registerCallback(OAIHttpRequestWorker *worker);
    void replaceUserNameCallback(OAIHttpRequestWorker *worker);
    void replaceUserPasswordCallback(OAIHttpRequestWorker *worker);
    void updateUserIdentityProviderCallback(OAIHttpRequestWorker *worker);
    void updateUserRoleCallback(OAIHttpRequestWorker *worker);

signals:

    void assignIdentityProvidersToInviterSignal(OAIMessage summary);
    void createInvitationSignal(OAIInvitation_token summary);
    void createWebhookSignal(OAIMessage summary);
    void deleteInvitationSignal(OAIMessage summary);
    void deleteUserSignal(OAIMessage summary);
    void deleteWebhookSignal(OAIMessage summary);
    void getInvitationMetadataSignal(OAIInvitation summary);
    void listIdentityProvidersOfInviterSignal(QList<OAIIdentity_provider> summary);
    void listInvitationsSignal(QList<OAIInvitation> summary);
    void listUsersSignal(QList<OAIUser> summary);
    void listWebhooksSignal(QList<OAIWebhook> summary);
    void r_registerSignal(OAIMessage summary);
    void replaceUserNameSignal(OAIMessage summary);
    void replaceUserPasswordSignal(OAIMessage summary);
    void updateUserIdentityProviderSignal(OAIMessage summary);
    void updateUserRoleSignal(OAIMessage summary);

    void assignIdentityProvidersToInviterSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void createInvitationSignalFull(OAIHttpRequestWorker *worker, OAIInvitation_token summary);
    void createWebhookSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteInvitationSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteUserSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void deleteWebhookSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void getInvitationMetadataSignalFull(OAIHttpRequestWorker *worker, OAIInvitation summary);
    void listIdentityProvidersOfInviterSignalFull(OAIHttpRequestWorker *worker, QList<OAIIdentity_provider> summary);
    void listInvitationsSignalFull(OAIHttpRequestWorker *worker, QList<OAIInvitation> summary);
    void listUsersSignalFull(OAIHttpRequestWorker *worker, QList<OAIUser> summary);
    void listWebhooksSignalFull(OAIHttpRequestWorker *worker, QList<OAIWebhook> summary);
    void r_registerSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void replaceUserNameSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void replaceUserPasswordSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void updateUserIdentityProviderSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);
    void updateUserRoleSignalFull(OAIHttpRequestWorker *worker, OAIMessage summary);

    void assignIdentityProvidersToInviterSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void createInvitationSignalE(OAIInvitation_token summary, QNetworkReply::NetworkError error_type, QString error_str);
    void createWebhookSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteInvitationSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteUserSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteWebhookSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getInvitationMetadataSignalE(OAIInvitation summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listIdentityProvidersOfInviterSignalE(QList<OAIIdentity_provider> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listInvitationsSignalE(QList<OAIInvitation> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listUsersSignalE(QList<OAIUser> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listWebhooksSignalE(QList<OAIWebhook> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void r_registerSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void replaceUserNameSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void replaceUserPasswordSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateUserIdentityProviderSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);
    void updateUserRoleSignalE(OAIMessage summary, QNetworkReply::NetworkError error_type, QString error_str);

    void assignIdentityProvidersToInviterSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void createInvitationSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void createWebhookSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteInvitationSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteUserSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void deleteWebhookSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getInvitationMetadataSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listIdentityProvidersOfInviterSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listInvitationsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listUsersSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listWebhooksSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void r_registerSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void replaceUserNameSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void replaceUserPasswordSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateUserIdentityProviderSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void updateUserRoleSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);

    void abortRequestsSignal();
    void allPendingRequestsCompleted();

public slots:
    void tokenAvailable();
    
};

} // namespace OpenAPI
#endif
