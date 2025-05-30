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

#include "OAIModel_job_labels.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIModel_job_labels::OAIModel_job_labels(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIModel_job_labels::OAIModel_job_labels() {
    this->initializeModel();
}

OAIModel_job_labels::~OAIModel_job_labels() {}

void OAIModel_job_labels::initializeModel() {

    m_cpu_request_isSet = false;
    m_cpu_request_isValid = false;

    m_instance_isSet = false;
    m_instance_isValid = false;

    m_memory_request_isSet = false;
    m_memory_request_isValid = false;

    m_multiplier_isSet = false;
    m_multiplier_isValid = false;

    m_node_selectors_isSet = false;
    m_node_selectors_isValid = false;

    m_resource_warning_isSet = false;
    m_resource_warning_isValid = false;

    m_tolerations_isSet = false;
    m_tolerations_isValid = false;

    m_workspace_request_isSet = false;
    m_workspace_request_isValid = false;
}

void OAIModel_job_labels::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIModel_job_labels::fromJsonObject(QJsonObject json) {

    m_cpu_request_isValid = ::OpenAPI::fromJsonValue(m_cpu_request, json[QString("cpu_request")]);
    m_cpu_request_isSet = !json[QString("cpu_request")].isNull() && m_cpu_request_isValid;

    m_instance_isValid = ::OpenAPI::fromJsonValue(m_instance, json[QString("instance")]);
    m_instance_isSet = !json[QString("instance")].isNull() && m_instance_isValid;

    m_memory_request_isValid = ::OpenAPI::fromJsonValue(m_memory_request, json[QString("memory_request")]);
    m_memory_request_isSet = !json[QString("memory_request")].isNull() && m_memory_request_isValid;

    m_multiplier_isValid = ::OpenAPI::fromJsonValue(m_multiplier, json[QString("multiplier")]);
    m_multiplier_isSet = !json[QString("multiplier")].isNull() && m_multiplier_isValid;

    m_node_selectors_isValid = ::OpenAPI::fromJsonValue(m_node_selectors, json[QString("node_selectors")]);
    m_node_selectors_isSet = !json[QString("node_selectors")].isNull() && m_node_selectors_isValid;

    m_resource_warning_isValid = ::OpenAPI::fromJsonValue(m_resource_warning, json[QString("resource_warning")]);
    m_resource_warning_isSet = !json[QString("resource_warning")].isNull() && m_resource_warning_isValid;

    m_tolerations_isValid = ::OpenAPI::fromJsonValue(m_tolerations, json[QString("tolerations")]);
    m_tolerations_isSet = !json[QString("tolerations")].isNull() && m_tolerations_isValid;

    m_workspace_request_isValid = ::OpenAPI::fromJsonValue(m_workspace_request, json[QString("workspace_request")]);
    m_workspace_request_isSet = !json[QString("workspace_request")].isNull() && m_workspace_request_isValid;
}

QString OAIModel_job_labels::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIModel_job_labels::asJsonObject() const {
    QJsonObject obj;
    if (m_cpu_request_isSet) {
        obj.insert(QString("cpu_request"), ::OpenAPI::toJsonValue(m_cpu_request));
    }
    if (m_instance_isSet) {
        obj.insert(QString("instance"), ::OpenAPI::toJsonValue(m_instance));
    }
    if (m_memory_request_isSet) {
        obj.insert(QString("memory_request"), ::OpenAPI::toJsonValue(m_memory_request));
    }
    if (m_multiplier_isSet) {
        obj.insert(QString("multiplier"), ::OpenAPI::toJsonValue(m_multiplier));
    }
    if (m_node_selectors.size() > 0) {
        obj.insert(QString("node_selectors"), ::OpenAPI::toJsonValue(m_node_selectors));
    }
    if (m_resource_warning_isSet) {
        obj.insert(QString("resource_warning"), ::OpenAPI::toJsonValue(m_resource_warning));
    }
    if (m_tolerations.size() > 0) {
        obj.insert(QString("tolerations"), ::OpenAPI::toJsonValue(m_tolerations));
    }
    if (m_workspace_request_isSet) {
        obj.insert(QString("workspace_request"), ::OpenAPI::toJsonValue(m_workspace_request));
    }
    return obj;
}

double OAIModel_job_labels::getCpuRequest() const {
    return m_cpu_request;
}
void OAIModel_job_labels::setCpuRequest(const double &cpu_request) {
    m_cpu_request = cpu_request;
    m_cpu_request_isSet = true;
}

bool OAIModel_job_labels::is_cpu_request_Set() const{
    return m_cpu_request_isSet;
}

bool OAIModel_job_labels::is_cpu_request_Valid() const{
    return m_cpu_request_isValid;
}

QString OAIModel_job_labels::getInstance() const {
    return m_instance;
}
void OAIModel_job_labels::setInstance(const QString &instance) {
    m_instance = instance;
    m_instance_isSet = true;
}

bool OAIModel_job_labels::is_instance_Set() const{
    return m_instance_isSet;
}

bool OAIModel_job_labels::is_instance_Valid() const{
    return m_instance_isValid;
}

qint32 OAIModel_job_labels::getMemoryRequest() const {
    return m_memory_request;
}
void OAIModel_job_labels::setMemoryRequest(const qint32 &memory_request) {
    m_memory_request = memory_request;
    m_memory_request_isSet = true;
}

bool OAIModel_job_labels::is_memory_request_Set() const{
    return m_memory_request_isSet;
}

bool OAIModel_job_labels::is_memory_request_Valid() const{
    return m_memory_request_isValid;
}

double OAIModel_job_labels::getMultiplier() const {
    return m_multiplier;
}
void OAIModel_job_labels::setMultiplier(const double &multiplier) {
    m_multiplier = multiplier;
    m_multiplier_isSet = true;
}

bool OAIModel_job_labels::is_multiplier_Set() const{
    return m_multiplier_isSet;
}

bool OAIModel_job_labels::is_multiplier_Valid() const{
    return m_multiplier_isValid;
}

QList<OAIGeneric_key_value_pair> OAIModel_job_labels::getNodeSelectors() const {
    return m_node_selectors;
}
void OAIModel_job_labels::setNodeSelectors(const QList<OAIGeneric_key_value_pair> &node_selectors) {
    m_node_selectors = node_selectors;
    m_node_selectors_isSet = true;
}

bool OAIModel_job_labels::is_node_selectors_Set() const{
    return m_node_selectors_isSet;
}

bool OAIModel_job_labels::is_node_selectors_Valid() const{
    return m_node_selectors_isValid;
}

QString OAIModel_job_labels::getResourceWarning() const {
    return m_resource_warning;
}
void OAIModel_job_labels::setResourceWarning(const QString &resource_warning) {
    m_resource_warning = resource_warning;
    m_resource_warning_isSet = true;
}

bool OAIModel_job_labels::is_resource_warning_Set() const{
    return m_resource_warning_isSet;
}

bool OAIModel_job_labels::is_resource_warning_Valid() const{
    return m_resource_warning_isValid;
}

QList<OAIGeneric_key_value_pair> OAIModel_job_labels::getTolerations() const {
    return m_tolerations;
}
void OAIModel_job_labels::setTolerations(const QList<OAIGeneric_key_value_pair> &tolerations) {
    m_tolerations = tolerations;
    m_tolerations_isSet = true;
}

bool OAIModel_job_labels::is_tolerations_Set() const{
    return m_tolerations_isSet;
}

bool OAIModel_job_labels::is_tolerations_Valid() const{
    return m_tolerations_isValid;
}

qint32 OAIModel_job_labels::getWorkspaceRequest() const {
    return m_workspace_request;
}
void OAIModel_job_labels::setWorkspaceRequest(const qint32 &workspace_request) {
    m_workspace_request = workspace_request;
    m_workspace_request_isSet = true;
}

bool OAIModel_job_labels::is_workspace_request_Set() const{
    return m_workspace_request_isSet;
}

bool OAIModel_job_labels::is_workspace_request_Valid() const{
    return m_workspace_request_isValid;
}

bool OAIModel_job_labels::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_cpu_request_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_instance_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_memory_request_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_multiplier_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_node_selectors.size() > 0) {
            isObjectUpdated = true;
            break;
        }

        if (m_resource_warning_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_tolerations.size() > 0) {
            isObjectUpdated = true;
            break;
        }

        if (m_workspace_request_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIModel_job_labels::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
