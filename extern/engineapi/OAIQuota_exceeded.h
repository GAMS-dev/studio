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
 * OAIQuota_exceeded.h
 *
 * 
 */

#ifndef OAIQuota_exceeded_H
#define OAIQuota_exceeded_H

#include <QJsonObject>

#include "OAIQuota.h"
#include <QList>
#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {
class OAIQuota;

class OAIQuota_exceeded : public OAIObject {
public:
    OAIQuota_exceeded();
    OAIQuota_exceeded(QString json);
    ~OAIQuota_exceeded() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QList<OAIQuota> getExceededQuotas() const;
    void setExceededQuotas(const QList<OAIQuota> &exceeded_quotas);
    bool is_exceeded_quotas_Set() const;
    bool is_exceeded_quotas_Valid() const;

    QString getMessage() const;
    void setMessage(const QString &message);
    bool is_message_Set() const;
    bool is_message_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QList<OAIQuota> exceeded_quotas;
    bool m_exceeded_quotas_isSet;
    bool m_exceeded_quotas_isValid;

    QString message;
    bool m_message_isSet;
    bool m_message_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIQuota_exceeded)

#endif // OAIQuota_exceeded_H
