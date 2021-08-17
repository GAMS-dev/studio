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

#include "OAIQuota_exceeded.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIQuota_exceeded::OAIQuota_exceeded(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIQuota_exceeded::OAIQuota_exceeded() {
    this->initializeModel();
}

OAIQuota_exceeded::~OAIQuota_exceeded() {}

void OAIQuota_exceeded::initializeModel() {

    m_exceeded_quotas_isSet = false;
    m_exceeded_quotas_isValid = false;

    m_message_isSet = false;
    m_message_isValid = false;
}

void OAIQuota_exceeded::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIQuota_exceeded::fromJsonObject(QJsonObject json) {

    m_exceeded_quotas_isValid = ::OpenAPI::fromJsonValue(exceeded_quotas, json[QString("exceeded_quotas")]);
    m_exceeded_quotas_isSet = !json[QString("exceeded_quotas")].isNull() && m_exceeded_quotas_isValid;

    m_message_isValid = ::OpenAPI::fromJsonValue(message, json[QString("message")]);
    m_message_isSet = !json[QString("message")].isNull() && m_message_isValid;
}

QString OAIQuota_exceeded::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIQuota_exceeded::asJsonObject() const {
    QJsonObject obj;
    if (exceeded_quotas.size() > 0) {
        obj.insert(QString("exceeded_quotas"), ::OpenAPI::toJsonValue(exceeded_quotas));
    }
    if (m_message_isSet) {
        obj.insert(QString("message"), ::OpenAPI::toJsonValue(message));
    }
    return obj;
}

QList<OAIQuota> OAIQuota_exceeded::getExceededQuotas() const {
    return exceeded_quotas;
}
void OAIQuota_exceeded::setExceededQuotas(const QList<OAIQuota> &exceeded_quotas) {
    this->exceeded_quotas = exceeded_quotas;
    this->m_exceeded_quotas_isSet = true;
}

bool OAIQuota_exceeded::is_exceeded_quotas_Set() const{
    return m_exceeded_quotas_isSet;
}

bool OAIQuota_exceeded::is_exceeded_quotas_Valid() const{
    return m_exceeded_quotas_isValid;
}

QString OAIQuota_exceeded::getMessage() const {
    return message;
}
void OAIQuota_exceeded::setMessage(const QString &message) {
    this->message = message;
    this->m_message_isSet = true;
}

bool OAIQuota_exceeded::is_message_Set() const{
    return m_message_isSet;
}

bool OAIQuota_exceeded::is_message_Valid() const{
    return m_message_isValid;
}

bool OAIQuota_exceeded::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (exceeded_quotas.size() > 0) {
            isObjectUpdated = true;
            break;
        }

        if (m_message_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIQuota_exceeded::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI