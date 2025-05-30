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

#include "OAIVapid_info.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIVapid_info::OAIVapid_info(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIVapid_info::OAIVapid_info() {
    this->initializeModel();
}

OAIVapid_info::~OAIVapid_info() {}

void OAIVapid_info::initializeModel() {

    m_application_server_key_isSet = false;
    m_application_server_key_isValid = false;
}

void OAIVapid_info::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIVapid_info::fromJsonObject(QJsonObject json) {

    m_application_server_key_isValid = ::OpenAPI::fromJsonValue(m_application_server_key, json[QString("application_server_key")]);
    m_application_server_key_isSet = !json[QString("application_server_key")].isNull() && m_application_server_key_isValid;
}

QString OAIVapid_info::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIVapid_info::asJsonObject() const {
    QJsonObject obj;
    if (m_application_server_key_isSet) {
        obj.insert(QString("application_server_key"), ::OpenAPI::toJsonValue(m_application_server_key));
    }
    return obj;
}

QString OAIVapid_info::getApplicationServerKey() const {
    return m_application_server_key;
}
void OAIVapid_info::setApplicationServerKey(const QString &application_server_key) {
    m_application_server_key = application_server_key;
    m_application_server_key_isSet = true;
}

bool OAIVapid_info::is_application_server_key_Set() const{
    return m_application_server_key_isSet;
}

bool OAIVapid_info::is_application_server_key_Valid() const{
    return m_application_server_key_isValid;
}

bool OAIVapid_info::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_application_server_key_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIVapid_info::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
