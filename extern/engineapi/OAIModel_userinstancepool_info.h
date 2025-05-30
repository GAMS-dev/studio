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

/*
 * OAIModel_userinstancepool_info.h
 *
 * 
 */

#ifndef OAIModel_userinstancepool_info_H
#define OAIModel_userinstancepool_info_H

#include <QJsonObject>

#include "OAIModel_instance_pool_info.h"
#include <QList>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {
class OAIModel_instance_pool_info;

class OAIModel_userinstancepool_info : public OAIObject {
public:
    OAIModel_userinstancepool_info();
    OAIModel_userinstancepool_info(QString json);
    ~OAIModel_userinstancepool_info() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QList<OAIModel_instance_pool_info> getInstancePoolsAvailable() const;
    void setInstancePoolsAvailable(const QList<OAIModel_instance_pool_info> &instance_pools_available);
    bool is_instance_pools_available_Set() const;
    bool is_instance_pools_available_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QList<OAIModel_instance_pool_info> m_instance_pools_available;
    bool m_instance_pools_available_isSet;
    bool m_instance_pools_available_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIModel_userinstancepool_info)

#endif // OAIModel_userinstancepool_info_H
