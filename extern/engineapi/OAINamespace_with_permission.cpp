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

#include "OAINamespace_with_permission.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAINamespace_with_permission::OAINamespace_with_permission(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAINamespace_with_permission::OAINamespace_with_permission() {
    this->initializeModel();
}

OAINamespace_with_permission::~OAINamespace_with_permission() {}

void OAINamespace_with_permission::initializeModel() {

    m_disk_quota_isSet = false;
    m_disk_quota_isValid = false;

    m_name_isSet = false;
    m_name_isValid = false;

    m_permission_isSet = false;
    m_permission_isValid = false;
}

void OAINamespace_with_permission::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAINamespace_with_permission::fromJsonObject(QJsonObject json) {

    m_disk_quota_isValid = ::OpenAPI::fromJsonValue(m_disk_quota, json[QString("disk_quota")]);
    m_disk_quota_isSet = !json[QString("disk_quota")].isNull() && m_disk_quota_isValid;

    m_name_isValid = ::OpenAPI::fromJsonValue(m_name, json[QString("name")]);
    m_name_isSet = !json[QString("name")].isNull() && m_name_isValid;

    m_permission_isValid = ::OpenAPI::fromJsonValue(m_permission, json[QString("permission")]);
    m_permission_isSet = !json[QString("permission")].isNull() && m_permission_isValid;
}

QString OAINamespace_with_permission::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAINamespace_with_permission::asJsonObject() const {
    QJsonObject obj;
    if (m_disk_quota_isSet) {
        obj.insert(QString("disk_quota"), ::OpenAPI::toJsonValue(m_disk_quota));
    }
    if (m_name_isSet) {
        obj.insert(QString("name"), ::OpenAPI::toJsonValue(m_name));
    }
    if (m_permission_isSet) {
        obj.insert(QString("permission"), ::OpenAPI::toJsonValue(m_permission));
    }
    return obj;
}

qint32 OAINamespace_with_permission::getDiskQuota() const {
    return m_disk_quota;
}
void OAINamespace_with_permission::setDiskQuota(const qint32 &disk_quota) {
    m_disk_quota = disk_quota;
    m_disk_quota_isSet = true;
}

bool OAINamespace_with_permission::is_disk_quota_Set() const{
    return m_disk_quota_isSet;
}

bool OAINamespace_with_permission::is_disk_quota_Valid() const{
    return m_disk_quota_isValid;
}

QString OAINamespace_with_permission::getName() const {
    return m_name;
}
void OAINamespace_with_permission::setName(const QString &name) {
    m_name = name;
    m_name_isSet = true;
}

bool OAINamespace_with_permission::is_name_Set() const{
    return m_name_isSet;
}

bool OAINamespace_with_permission::is_name_Valid() const{
    return m_name_isValid;
}

qint32 OAINamespace_with_permission::getPermission() const {
    return m_permission;
}
void OAINamespace_with_permission::setPermission(const qint32 &permission) {
    m_permission = permission;
    m_permission_isSet = true;
}

bool OAINamespace_with_permission::is_permission_Set() const{
    return m_permission_isSet;
}

bool OAINamespace_with_permission::is_permission_Valid() const{
    return m_permission_isValid;
}

bool OAINamespace_with_permission::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_disk_quota_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_name_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_permission_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAINamespace_with_permission::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
