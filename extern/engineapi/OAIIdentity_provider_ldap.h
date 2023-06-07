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
 * OAIIdentity_provider_ldap.h
 *
 * 
 */

#ifndef OAIIdentity_provider_ldap_H
#define OAIIdentity_provider_ldap_H

#include <QJsonObject>

#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIIdentity_provider_ldap : public OAIObject {
public:
    OAIIdentity_provider_ldap();
    OAIIdentity_provider_ldap(QString json);
    ~OAIIdentity_provider_ldap() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    bool isActiveDirectory() const;
    void setActiveDirectory(const bool &active_directory);
    bool is_active_directory_Set() const;
    bool is_active_directory_Valid() const;

    QString getBase() const;
    void setBase(const QString &base);
    bool is_base_Set() const;
    bool is_base_Valid() const;

    QString getBindDn() const;
    void setBindDn(const QString &bind_dn);
    bool is_bind_dn_Set() const;
    bool is_bind_dn_Valid() const;

    QString getEncryption() const;
    void setEncryption(const QString &encryption);
    bool is_encryption_Set() const;
    bool is_encryption_Valid() const;

    bool isHidden() const;
    void setHidden(const bool &hidden);
    bool is_hidden_Set() const;
    bool is_hidden_Valid() const;

    QString getHost() const;
    void setHost(const QString &host);
    bool is_host_Set() const;
    bool is_host_Valid() const;

    QString getLabel() const;
    void setLabel(const QString &label);
    bool is_label_Set() const;
    bool is_label_Valid() const;

    QString getName() const;
    void setName(const QString &name);
    bool is_name_Set() const;
    bool is_name_Valid() const;

    QString getPassword() const;
    void setPassword(const QString &password);
    bool is_password_Set() const;
    bool is_password_Valid() const;

    qint32 getPort() const;
    void setPort(const qint32 &port);
    bool is_port_Set() const;
    bool is_port_Valid() const;

    QString getUid() const;
    void setUid(const QString &uid);
    bool is_uid_Set() const;
    bool is_uid_Valid() const;

    QString getUserFilter() const;
    void setUserFilter(const QString &user_filter);
    bool is_user_filter_Set() const;
    bool is_user_filter_Valid() const;

    bool isVerifyCertificates() const;
    void setVerifyCertificates(const bool &verify_certificates);
    bool is_verify_certificates_Set() const;
    bool is_verify_certificates_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    bool active_directory;
    bool m_active_directory_isSet;
    bool m_active_directory_isValid;

    QString base;
    bool m_base_isSet;
    bool m_base_isValid;

    QString bind_dn;
    bool m_bind_dn_isSet;
    bool m_bind_dn_isValid;

    QString encryption;
    bool m_encryption_isSet;
    bool m_encryption_isValid;

    bool hidden;
    bool m_hidden_isSet;
    bool m_hidden_isValid;

    QString host;
    bool m_host_isSet;
    bool m_host_isValid;

    QString label;
    bool m_label_isSet;
    bool m_label_isValid;

    QString name;
    bool m_name_isSet;
    bool m_name_isValid;

    QString password;
    bool m_password_isSet;
    bool m_password_isValid;

    qint32 port;
    bool m_port_isSet;
    bool m_port_isValid;

    QString uid;
    bool m_uid_isSet;
    bool m_uid_isValid;

    QString user_filter;
    bool m_user_filter_isSet;
    bool m_user_filter_isValid;

    bool verify_certificates;
    bool m_verify_certificates_isSet;
    bool m_verify_certificates_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIIdentity_provider_ldap)

#endif // OAIIdentity_provider_ldap_H
