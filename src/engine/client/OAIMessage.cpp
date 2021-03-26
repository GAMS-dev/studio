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

#include "OAIMessage.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIMessage::OAIMessage(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIMessage::OAIMessage() {
    this->initializeModel();
}

OAIMessage::~OAIMessage() {}

void OAIMessage::initializeModel() {

    m_message_isSet = false;
    m_message_isValid = false;
}

void OAIMessage::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIMessage::fromJsonObject(QJsonObject json) {

    m_message_isValid = ::OpenAPI::fromJsonValue(message, json[QString("message")]);
    m_message_isSet = !json[QString("message")].isNull() && m_message_isValid;
}

QString OAIMessage::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIMessage::asJsonObject() const {
    QJsonObject obj;
    if (m_message_isSet) {
        obj.insert(QString("message"), ::OpenAPI::toJsonValue(message));
    }
    return obj;
}

QString OAIMessage::getMessage() const {
    return message;
}
void OAIMessage::setMessage(const QString &message) {
    this->message = message;
    this->m_message_isSet = true;
}

bool OAIMessage::is_message_Set() const{
    return m_message_isSet;
}

bool OAIMessage::is_message_Valid() const{
    return m_message_isValid;
}

bool OAIMessage::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_message_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIMessage::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
