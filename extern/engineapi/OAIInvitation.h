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

/*
 * OAIInvitation.h
 *
 * 
 */

#ifndef OAIInvitation_H
#define OAIInvitation_H

#include <QJsonObject>

#include "OAIInvitation_quota.h"
#include <QDateTime>
#include <QList>
#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIInvitation : public OAIObject {
public:
    OAIInvitation();
    OAIInvitation(QString json);
    ~OAIInvitation() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QDateTime getCreated() const;
    void setCreated(const QDateTime &created);
    bool is_created_Set() const;
    bool is_created_Valid() const;

    QString getGamsLicense() const;
    void setGamsLicense(const QString &gams_license);
    bool is_gams_license_Set() const;
    bool is_gams_license_Valid() const;

    QString getIdentityProvider() const;
    void setIdentityProvider(const QString &identity_provider);
    bool is_identity_provider_Set() const;
    bool is_identity_provider_Valid() const;

    QString getIdentityProviderUserSubject() const;
    void setIdentityProviderUserSubject(const QString &identity_provider_user_subject);
    bool is_identity_provider_user_subject_Set() const;
    bool is_identity_provider_user_subject_Valid() const;

    QList<QString> getInvitableIdentityProviders() const;
    void setInvitableIdentityProviders(const QList<QString> &invitable_identity_providers);
    bool is_invitable_identity_providers_Set() const;
    bool is_invitable_identity_providers_Valid() const;

    QString getInviterName() const;
    void setInviterName(const QString &inviter_name);
    bool is_inviter_name_Set() const;
    bool is_inviter_name_Valid() const;

    QList<QString> getPermissions() const;
    void setPermissions(const QList<QString> &permissions);
    bool is_permissions_Set() const;
    bool is_permissions_Valid() const;

    OAIInvitation_quota getQuota() const;
    void setQuota(const OAIInvitation_quota &quota);
    bool is_quota_Set() const;
    bool is_quota_Valid() const;

    QList<QString> getRoles() const;
    void setRoles(const QList<QString> &roles);
    bool is_roles_Set() const;
    bool is_roles_Valid() const;

    QString getToken() const;
    void setToken(const QString &token);
    bool is_token_Set() const;
    bool is_token_Valid() const;

    bool isUsed() const;
    void setUsed(const bool &used);
    bool is_used_Set() const;
    bool is_used_Valid() const;

    QList<QString> getUserGroups() const;
    void setUserGroups(const QList<QString> &user_groups);
    bool is_user_groups_Set() const;
    bool is_user_groups_Valid() const;

    QString getUsername() const;
    void setUsername(const QString &username);
    bool is_username_Set() const;
    bool is_username_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QDateTime created;
    bool m_created_isSet;
    bool m_created_isValid;

    QString gams_license;
    bool m_gams_license_isSet;
    bool m_gams_license_isValid;

    QString identity_provider;
    bool m_identity_provider_isSet;
    bool m_identity_provider_isValid;

    QString identity_provider_user_subject;
    bool m_identity_provider_user_subject_isSet;
    bool m_identity_provider_user_subject_isValid;

    QList<QString> invitable_identity_providers;
    bool m_invitable_identity_providers_isSet;
    bool m_invitable_identity_providers_isValid;

    QString inviter_name;
    bool m_inviter_name_isSet;
    bool m_inviter_name_isValid;

    QList<QString> permissions;
    bool m_permissions_isSet;
    bool m_permissions_isValid;

    OAIInvitation_quota quota;
    bool m_quota_isSet;
    bool m_quota_isValid;

    QList<QString> roles;
    bool m_roles_isSet;
    bool m_roles_isValid;

    QString token;
    bool m_token_isSet;
    bool m_token_isValid;

    bool used;
    bool m_used_isSet;
    bool m_used_isValid;

    QList<QString> user_groups;
    bool m_user_groups_isSet;
    bool m_user_groups_isValid;

    QString username;
    bool m_username_isSet;
    bool m_username_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIInvitation)

#endif // OAIInvitation_H
