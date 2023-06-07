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
 * OAIModel_instance_info.h
 *
 * 
 */

#ifndef OAIModel_instance_info_H
#define OAIModel_instance_info_H

#include <QJsonObject>

#include "OAIGeneric_key_value_pair.h"
#include <QList>
#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIModel_instance_info : public OAIObject {
public:
    OAIModel_instance_info();
    OAIModel_instance_info(QString json);
    ~OAIModel_instance_info() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    double getCpuRequest() const;
    void setCpuRequest(const double &cpu_request);
    bool is_cpu_request_Set() const;
    bool is_cpu_request_Valid() const;

    QString getLabel() const;
    void setLabel(const QString &label);
    bool is_label_Set() const;
    bool is_label_Valid() const;

    qint32 getMemoryRequest() const;
    void setMemoryRequest(const qint32 &memory_request);
    bool is_memory_request_Set() const;
    bool is_memory_request_Valid() const;

    double getMultiplier() const;
    void setMultiplier(const double &multiplier);
    bool is_multiplier_Set() const;
    bool is_multiplier_Valid() const;

    double getMultiplierIdle() const;
    void setMultiplierIdle(const double &multiplier_idle);
    bool is_multiplier_idle_Set() const;
    bool is_multiplier_idle_Valid() const;

    QList<OAIGeneric_key_value_pair> getNodeSelectors() const;
    void setNodeSelectors(const QList<OAIGeneric_key_value_pair> &node_selectors);
    bool is_node_selectors_Set() const;
    bool is_node_selectors_Valid() const;

    QList<OAIGeneric_key_value_pair> getTolerations() const;
    void setTolerations(const QList<OAIGeneric_key_value_pair> &tolerations);
    bool is_tolerations_Set() const;
    bool is_tolerations_Valid() const;

    qint32 getWorkspaceRequest() const;
    void setWorkspaceRequest(const qint32 &workspace_request);
    bool is_workspace_request_Set() const;
    bool is_workspace_request_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    double cpu_request;
    bool m_cpu_request_isSet;
    bool m_cpu_request_isValid;

    QString label;
    bool m_label_isSet;
    bool m_label_isValid;

    qint32 memory_request;
    bool m_memory_request_isSet;
    bool m_memory_request_isValid;

    double multiplier;
    bool m_multiplier_isSet;
    bool m_multiplier_isValid;

    double multiplier_idle;
    bool m_multiplier_idle_isSet;
    bool m_multiplier_idle_isValid;

    QList<OAIGeneric_key_value_pair> node_selectors;
    bool m_node_selectors_isSet;
    bool m_node_selectors_isValid;

    QList<OAIGeneric_key_value_pair> tolerations;
    bool m_tolerations_isSet;
    bool m_tolerations_isValid;

    qint32 workspace_request;
    bool m_workspace_request_isSet;
    bool m_workspace_request_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIModel_instance_info)

#endif // OAIModel_instance_info_H
