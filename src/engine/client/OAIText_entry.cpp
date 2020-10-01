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

#include "OAIText_entry.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIText_entry::OAIText_entry(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIText_entry::OAIText_entry() {
    this->initializeModel();
}

OAIText_entry::~OAIText_entry() {}

void OAIText_entry::initializeModel() {

    m_entry_name_isSet = false;
    m_entry_name_isValid = false;

    m_entry_value_isSet = false;
    m_entry_value_isValid = false;

    m_entry_size_isSet = false;
    m_entry_size_isValid = false;
}

void OAIText_entry::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIText_entry::fromJsonObject(QJsonObject json) {

    m_entry_name_isValid = ::OpenAPI::fromJsonValue(entry_name, json[QString("entry_name")]);
    m_entry_name_isSet = !json[QString("entry_name")].isNull() && m_entry_name_isValid;

    m_entry_value_isValid = ::OpenAPI::fromJsonValue(entry_value, json[QString("entry_value")]);
    m_entry_value_isSet = !json[QString("entry_value")].isNull() && m_entry_value_isValid;

    m_entry_size_isValid = ::OpenAPI::fromJsonValue(entry_size, json[QString("entry_size")]);
    m_entry_size_isSet = !json[QString("entry_size")].isNull() && m_entry_size_isValid;
}

QString OAIText_entry::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIText_entry::asJsonObject() const {
    QJsonObject obj;
    if (m_entry_name_isSet) {
        obj.insert(QString("entry_name"), ::OpenAPI::toJsonValue(entry_name));
    }
    if (m_entry_value_isSet) {
        obj.insert(QString("entry_value"), ::OpenAPI::toJsonValue(entry_value));
    }
    if (m_entry_size_isSet) {
        obj.insert(QString("entry_size"), ::OpenAPI::toJsonValue(entry_size));
    }
    return obj;
}

QString OAIText_entry::getEntryName() const {
    return entry_name;
}
void OAIText_entry::setEntryName(const QString &entry_name) {
    this->entry_name = entry_name;
    this->m_entry_name_isSet = true;
}

QString OAIText_entry::getEntryValue() const {
    return entry_value;
}
void OAIText_entry::setEntryValue(const QString &entry_value) {
    this->entry_value = entry_value;
    this->m_entry_value_isSet = true;
}

qint32 OAIText_entry::getEntrySize() const {
    return entry_size;
}
void OAIText_entry::setEntrySize(const qint32 &entry_size) {
    this->entry_size = entry_size;
    this->m_entry_size_isSet = true;
}

bool OAIText_entry::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_entry_name_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_entry_value_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_entry_size_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIText_entry::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
