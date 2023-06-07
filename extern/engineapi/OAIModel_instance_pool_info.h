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

/*
 * OAIModel_instance_pool_info.h
 *
 * 
 */

#ifndef OAIModel_instance_pool_info_H
#define OAIModel_instance_pool_info_H

#include <QJsonObject>

#include "OAIModel_instance_info.h"
#include "OAIModel_user.h"
#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {
class OAIModel_instance_info;
class OAIModel_user;

class OAIModel_instance_pool_info : public OAIObject {
public:
    OAIModel_instance_pool_info();
    OAIModel_instance_pool_info(QString json);
    ~OAIModel_instance_pool_info() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    bool isCancelling() const;
    void setCancelling(const bool &cancelling);
    bool is_cancelling_Set() const;
    bool is_cancelling_Valid() const;

    OAIModel_instance_info getInstance() const;
    void setInstance(const OAIModel_instance_info &instance);
    bool is_instance_Set() const;
    bool is_instance_Valid() const;

    QString getLabel() const;
    void setLabel(const QString &label);
    bool is_label_Set() const;
    bool is_label_Valid() const;

    OAIModel_user getOwner() const;
    void setOwner(const OAIModel_user &owner);
    bool is_owner_Set() const;
    bool is_owner_Valid() const;

    qint32 getSize() const;
    void setSize(const qint32 &size);
    bool is_size_Set() const;
    bool is_size_Valid() const;

    qint32 getSizeActive() const;
    void setSizeActive(const qint32 &size_active);
    bool is_size_active_Set() const;
    bool is_size_active_Valid() const;

    qint32 getSizeBusy() const;
    void setSizeBusy(const qint32 &size_busy);
    bool is_size_busy_Set() const;
    bool is_size_busy_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    bool cancelling;
    bool m_cancelling_isSet;
    bool m_cancelling_isValid;

    OAIModel_instance_info instance;
    bool m_instance_isSet;
    bool m_instance_isValid;

    QString label;
    bool m_label_isSet;
    bool m_label_isValid;

    OAIModel_user owner;
    bool m_owner_isSet;
    bool m_owner_isValid;

    qint32 size;
    bool m_size_isSet;
    bool m_size_isValid;

    qint32 size_active;
    bool m_size_active_isSet;
    bool m_size_active_isValid;

    qint32 size_busy;
    bool m_size_busy_isSet;
    bool m_size_busy_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIModel_instance_pool_info)

#endif // OAIModel_instance_pool_info_H
