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
 * OAIModel_userinstance_info.h
 *
 * 
 */

#ifndef OAIModel_userinstance_info_H
#define OAIModel_userinstance_info_H

#include <QJsonObject>

#include "OAIModel_instance_info.h"
#include <QList>
#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIModel_userinstance_info : public OAIObject {
public:
    OAIModel_userinstance_info();
    OAIModel_userinstance_info(QString json);
    ~OAIModel_userinstance_info() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getDefaultInheritedFrom() const;
    void setDefaultInheritedFrom(const QString &default_inherited_from);
    bool is_default_inherited_from_Set() const;
    bool is_default_inherited_from_Valid() const;

    OAIModel_instance_info getDefaultInstance() const;
    void setDefaultInstance(const OAIModel_instance_info &default_instance);
    bool is_default_instance_Set() const;
    bool is_default_instance_Valid() const;

    QList<OAIModel_instance_info> getInstancesAvailable() const;
    void setInstancesAvailable(const QList<OAIModel_instance_info> &instances_available);
    bool is_instances_available_Set() const;
    bool is_instances_available_Valid() const;

    QString getInstancesInheritedFrom() const;
    void setInstancesInheritedFrom(const QString &instances_inherited_from);
    bool is_instances_inherited_from_Set() const;
    bool is_instances_inherited_from_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString default_inherited_from;
    bool m_default_inherited_from_isSet;
    bool m_default_inherited_from_isValid;

    OAIModel_instance_info default_instance;
    bool m_default_instance_isSet;
    bool m_default_instance_isValid;

    QList<OAIModel_instance_info> instances_available;
    bool m_instances_available_isSet;
    bool m_instances_available_isValid;

    QString instances_inherited_from;
    bool m_instances_inherited_from_isSet;
    bool m_instances_inherited_from_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIModel_userinstance_info)

#endif // OAIModel_userinstance_info_H
