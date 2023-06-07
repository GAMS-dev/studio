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

#include "OAIModel_configuration.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIModel_configuration::OAIModel_configuration(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIModel_configuration::OAIModel_configuration() {
    this->initializeModel();
}

OAIModel_configuration::~OAIModel_configuration() {}

void OAIModel_configuration::initializeModel() {

    m_hostname_isSet = false;
    m_hostname_isValid = false;

    m_instance_pool_access_isSet = false;
    m_instance_pool_access_isValid = false;

    m_text_entries_max_size_isSet = false;
    m_text_entries_max_size_isValid = false;

    m_webhook_access_isSet = false;
    m_webhook_access_isValid = false;
}

void OAIModel_configuration::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIModel_configuration::fromJsonObject(QJsonObject json) {

    m_hostname_isValid = ::OpenAPI::fromJsonValue(hostname, json[QString("hostname")]);
    m_hostname_isSet = !json[QString("hostname")].isNull() && m_hostname_isValid;

    m_instance_pool_access_isValid = ::OpenAPI::fromJsonValue(instance_pool_access, json[QString("instance_pool_access")]);
    m_instance_pool_access_isSet = !json[QString("instance_pool_access")].isNull() && m_instance_pool_access_isValid;

    m_text_entries_max_size_isValid = ::OpenAPI::fromJsonValue(text_entries_max_size, json[QString("text_entries_max_size")]);
    m_text_entries_max_size_isSet = !json[QString("text_entries_max_size")].isNull() && m_text_entries_max_size_isValid;

    m_webhook_access_isValid = ::OpenAPI::fromJsonValue(webhook_access, json[QString("webhook_access")]);
    m_webhook_access_isSet = !json[QString("webhook_access")].isNull() && m_webhook_access_isValid;
}

QString OAIModel_configuration::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIModel_configuration::asJsonObject() const {
    QJsonObject obj;
    if (m_hostname_isSet) {
        obj.insert(QString("hostname"), ::OpenAPI::toJsonValue(hostname));
    }
    if (m_instance_pool_access_isSet) {
        obj.insert(QString("instance_pool_access"), ::OpenAPI::toJsonValue(instance_pool_access));
    }
    if (m_text_entries_max_size_isSet) {
        obj.insert(QString("text_entries_max_size"), ::OpenAPI::toJsonValue(text_entries_max_size));
    }
    if (m_webhook_access_isSet) {
        obj.insert(QString("webhook_access"), ::OpenAPI::toJsonValue(webhook_access));
    }
    return obj;
}

QString OAIModel_configuration::getHostname() const {
    return hostname;
}
void OAIModel_configuration::setHostname(const QString &hostname) {
    this->hostname = hostname;
    this->m_hostname_isSet = true;
}

bool OAIModel_configuration::is_hostname_Set() const{
    return m_hostname_isSet;
}

bool OAIModel_configuration::is_hostname_Valid() const{
    return m_hostname_isValid;
}

QString OAIModel_configuration::getInstancePoolAccess() const {
    return instance_pool_access;
}
void OAIModel_configuration::setInstancePoolAccess(const QString &instance_pool_access) {
    this->instance_pool_access = instance_pool_access;
    this->m_instance_pool_access_isSet = true;
}

bool OAIModel_configuration::is_instance_pool_access_Set() const{
    return m_instance_pool_access_isSet;
}

bool OAIModel_configuration::is_instance_pool_access_Valid() const{
    return m_instance_pool_access_isValid;
}

qint32 OAIModel_configuration::getTextEntriesMaxSize() const {
    return text_entries_max_size;
}
void OAIModel_configuration::setTextEntriesMaxSize(const qint32 &text_entries_max_size) {
    this->text_entries_max_size = text_entries_max_size;
    this->m_text_entries_max_size_isSet = true;
}

bool OAIModel_configuration::is_text_entries_max_size_Set() const{
    return m_text_entries_max_size_isSet;
}

bool OAIModel_configuration::is_text_entries_max_size_Valid() const{
    return m_text_entries_max_size_isValid;
}

QString OAIModel_configuration::getWebhookAccess() const {
    return webhook_access;
}
void OAIModel_configuration::setWebhookAccess(const QString &webhook_access) {
    this->webhook_access = webhook_access;
    this->m_webhook_access_isSet = true;
}

bool OAIModel_configuration::is_webhook_access_Set() const{
    return m_webhook_access_isSet;
}

bool OAIModel_configuration::is_webhook_access_Valid() const{
    return m_webhook_access_isValid;
}

bool OAIModel_configuration::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_hostname_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_instance_pool_access_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_text_entries_max_size_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_webhook_access_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIModel_configuration::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
