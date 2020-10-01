/**
 * GAMS Engine
 * GAMS Engine let's you register, solve and get results of GAMS Models. It has namespace management system so you can restrict your users to certain set of models.
 *
 * The version of the OpenAPI document: dev
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#include "OAIStatus_code_meaning.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIStatus_code_meaning::OAIStatus_code_meaning(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIStatus_code_meaning::OAIStatus_code_meaning() {
    this->initializeModel();
}

OAIStatus_code_meaning::~OAIStatus_code_meaning() {}

void OAIStatus_code_meaning::initializeModel() {

    m_status_code_isSet = false;
    m_status_code_isValid = false;

    m_description_isSet = false;
    m_description_isValid = false;
}

void OAIStatus_code_meaning::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIStatus_code_meaning::fromJsonObject(QJsonObject json) {

    m_status_code_isValid = ::OpenAPI::fromJsonValue(status_code, json[QString("status_code")]);
    m_status_code_isSet = !json[QString("status_code")].isNull() && m_status_code_isValid;

    m_description_isValid = ::OpenAPI::fromJsonValue(description, json[QString("description")]);
    m_description_isSet = !json[QString("description")].isNull() && m_description_isValid;
}

QString OAIStatus_code_meaning::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIStatus_code_meaning::asJsonObject() const {
    QJsonObject obj;
    if (m_status_code_isSet) {
        obj.insert(QString("status_code"), ::OpenAPI::toJsonValue(status_code));
    }
    if (m_description_isSet) {
        obj.insert(QString("description"), ::OpenAPI::toJsonValue(description));
    }
    return obj;
}

qint32 OAIStatus_code_meaning::getStatusCode() const {
    return status_code;
}
void OAIStatus_code_meaning::setStatusCode(const qint32 &status_code) {
    this->status_code = status_code;
    this->m_status_code_isSet = true;
}

QString OAIStatus_code_meaning::getDescription() const {
    return description;
}
void OAIStatus_code_meaning::setDescription(const QString &description) {
    this->description = description;
    this->m_description_isSet = true;
}

bool OAIStatus_code_meaning::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_status_code_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_description_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIStatus_code_meaning::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
