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

#include "OAILicense.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAILicense::OAILicense(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAILicense::OAILicense() {
    this->initializeModel();
}

OAILicense::~OAILicense() {}

void OAILicense::initializeModel() {

    m_inherited_from_isSet = false;
    m_inherited_from_isValid = false;

    m_license_isSet = false;
    m_license_isValid = false;

    m_user_isSet = false;
    m_user_isValid = false;
}

void OAILicense::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAILicense::fromJsonObject(QJsonObject json) {

    m_inherited_from_isValid = ::OpenAPI::fromJsonValue(inherited_from, json[QString("inherited_from")]);
    m_inherited_from_isSet = !json[QString("inherited_from")].isNull() && m_inherited_from_isValid;

    m_license_isValid = ::OpenAPI::fromJsonValue(license, json[QString("license")]);
    m_license_isSet = !json[QString("license")].isNull() && m_license_isValid;

    m_user_isValid = ::OpenAPI::fromJsonValue(user, json[QString("user")]);
    m_user_isSet = !json[QString("user")].isNull() && m_user_isValid;
}

QString OAILicense::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAILicense::asJsonObject() const {
    QJsonObject obj;
    if (m_inherited_from_isSet) {
        obj.insert(QString("inherited_from"), ::OpenAPI::toJsonValue(inherited_from));
    }
    if (m_license_isSet) {
        obj.insert(QString("license"), ::OpenAPI::toJsonValue(license));
    }
    if (m_user_isSet) {
        obj.insert(QString("user"), ::OpenAPI::toJsonValue(user));
    }
    return obj;
}

QString OAILicense::getInheritedFrom() const {
    return inherited_from;
}
void OAILicense::setInheritedFrom(const QString &inherited_from) {
    this->inherited_from = inherited_from;
    this->m_inherited_from_isSet = true;
}

bool OAILicense::is_inherited_from_Set() const{
    return m_inherited_from_isSet;
}

bool OAILicense::is_inherited_from_Valid() const{
    return m_inherited_from_isValid;
}

QString OAILicense::getLicense() const {
    return license;
}
void OAILicense::setLicense(const QString &license) {
    this->license = license;
    this->m_license_isSet = true;
}

bool OAILicense::is_license_Set() const{
    return m_license_isSet;
}

bool OAILicense::is_license_Valid() const{
    return m_license_isValid;
}

QString OAILicense::getUser() const {
    return user;
}
void OAILicense::setUser(const QString &user) {
    this->user = user;
    this->m_user_isSet = true;
}

bool OAILicense::is_user_Set() const{
    return m_user_isSet;
}

bool OAILicense::is_user_Valid() const{
    return m_user_isValid;
}

bool OAILicense::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_inherited_from_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_license_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_user_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAILicense::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
