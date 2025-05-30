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

#include "OAIModel_instance_pool_info.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIModel_instance_pool_info::OAIModel_instance_pool_info(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIModel_instance_pool_info::OAIModel_instance_pool_info() {
    this->initializeModel();
}

OAIModel_instance_pool_info::~OAIModel_instance_pool_info() {}

void OAIModel_instance_pool_info::initializeModel() {

    m_cancelling_isSet = false;
    m_cancelling_isValid = false;

    m_instance_isSet = false;
    m_instance_isValid = false;

    m_label_isSet = false;
    m_label_isValid = false;

    m_owner_isSet = false;
    m_owner_isValid = false;

    m_size_isSet = false;
    m_size_isValid = false;

    m_size_active_isSet = false;
    m_size_active_isValid = false;

    m_size_busy_isSet = false;
    m_size_busy_isValid = false;
}

void OAIModel_instance_pool_info::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIModel_instance_pool_info::fromJsonObject(QJsonObject json) {

    m_cancelling_isValid = ::OpenAPI::fromJsonValue(m_cancelling, json[QString("cancelling")]);
    m_cancelling_isSet = !json[QString("cancelling")].isNull() && m_cancelling_isValid;

    m_instance_isValid = ::OpenAPI::fromJsonValue(m_instance, json[QString("instance")]);
    m_instance_isSet = !json[QString("instance")].isNull() && m_instance_isValid;

    m_label_isValid = ::OpenAPI::fromJsonValue(m_label, json[QString("label")]);
    m_label_isSet = !json[QString("label")].isNull() && m_label_isValid;

    m_owner_isValid = ::OpenAPI::fromJsonValue(m_owner, json[QString("owner")]);
    m_owner_isSet = !json[QString("owner")].isNull() && m_owner_isValid;

    m_size_isValid = ::OpenAPI::fromJsonValue(m_size, json[QString("size")]);
    m_size_isSet = !json[QString("size")].isNull() && m_size_isValid;

    m_size_active_isValid = ::OpenAPI::fromJsonValue(m_size_active, json[QString("size_active")]);
    m_size_active_isSet = !json[QString("size_active")].isNull() && m_size_active_isValid;

    m_size_busy_isValid = ::OpenAPI::fromJsonValue(m_size_busy, json[QString("size_busy")]);
    m_size_busy_isSet = !json[QString("size_busy")].isNull() && m_size_busy_isValid;
}

QString OAIModel_instance_pool_info::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIModel_instance_pool_info::asJsonObject() const {
    QJsonObject obj;
    if (m_cancelling_isSet) {
        obj.insert(QString("cancelling"), ::OpenAPI::toJsonValue(m_cancelling));
    }
    if (m_instance.isSet()) {
        obj.insert(QString("instance"), ::OpenAPI::toJsonValue(m_instance));
    }
    if (m_label_isSet) {
        obj.insert(QString("label"), ::OpenAPI::toJsonValue(m_label));
    }
    if (m_owner.isSet()) {
        obj.insert(QString("owner"), ::OpenAPI::toJsonValue(m_owner));
    }
    if (m_size_isSet) {
        obj.insert(QString("size"), ::OpenAPI::toJsonValue(m_size));
    }
    if (m_size_active_isSet) {
        obj.insert(QString("size_active"), ::OpenAPI::toJsonValue(m_size_active));
    }
    if (m_size_busy_isSet) {
        obj.insert(QString("size_busy"), ::OpenAPI::toJsonValue(m_size_busy));
    }
    return obj;
}

bool OAIModel_instance_pool_info::isCancelling() const {
    return m_cancelling;
}
void OAIModel_instance_pool_info::setCancelling(const bool &cancelling) {
    m_cancelling = cancelling;
    m_cancelling_isSet = true;
}

bool OAIModel_instance_pool_info::is_cancelling_Set() const{
    return m_cancelling_isSet;
}

bool OAIModel_instance_pool_info::is_cancelling_Valid() const{
    return m_cancelling_isValid;
}

OAIModel_instance_info OAIModel_instance_pool_info::getInstance() const {
    return m_instance;
}
void OAIModel_instance_pool_info::setInstance(const OAIModel_instance_info &instance) {
    m_instance = instance;
    m_instance_isSet = true;
}

bool OAIModel_instance_pool_info::is_instance_Set() const{
    return m_instance_isSet;
}

bool OAIModel_instance_pool_info::is_instance_Valid() const{
    return m_instance_isValid;
}

QString OAIModel_instance_pool_info::getLabel() const {
    return m_label;
}
void OAIModel_instance_pool_info::setLabel(const QString &label) {
    m_label = label;
    m_label_isSet = true;
}

bool OAIModel_instance_pool_info::is_label_Set() const{
    return m_label_isSet;
}

bool OAIModel_instance_pool_info::is_label_Valid() const{
    return m_label_isValid;
}

OAIModel_user OAIModel_instance_pool_info::getOwner() const {
    return m_owner;
}
void OAIModel_instance_pool_info::setOwner(const OAIModel_user &owner) {
    m_owner = owner;
    m_owner_isSet = true;
}

bool OAIModel_instance_pool_info::is_owner_Set() const{
    return m_owner_isSet;
}

bool OAIModel_instance_pool_info::is_owner_Valid() const{
    return m_owner_isValid;
}

qint32 OAIModel_instance_pool_info::getSize() const {
    return m_size;
}
void OAIModel_instance_pool_info::setSize(const qint32 &size) {
    m_size = size;
    m_size_isSet = true;
}

bool OAIModel_instance_pool_info::is_size_Set() const{
    return m_size_isSet;
}

bool OAIModel_instance_pool_info::is_size_Valid() const{
    return m_size_isValid;
}

qint32 OAIModel_instance_pool_info::getSizeActive() const {
    return m_size_active;
}
void OAIModel_instance_pool_info::setSizeActive(const qint32 &size_active) {
    m_size_active = size_active;
    m_size_active_isSet = true;
}

bool OAIModel_instance_pool_info::is_size_active_Set() const{
    return m_size_active_isSet;
}

bool OAIModel_instance_pool_info::is_size_active_Valid() const{
    return m_size_active_isValid;
}

qint32 OAIModel_instance_pool_info::getSizeBusy() const {
    return m_size_busy;
}
void OAIModel_instance_pool_info::setSizeBusy(const qint32 &size_busy) {
    m_size_busy = size_busy;
    m_size_busy_isSet = true;
}

bool OAIModel_instance_pool_info::is_size_busy_Set() const{
    return m_size_busy_isSet;
}

bool OAIModel_instance_pool_info::is_size_busy_Valid() const{
    return m_size_busy_isValid;
}

bool OAIModel_instance_pool_info::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_cancelling_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_instance.isSet()) {
            isObjectUpdated = true;
            break;
        }

        if (m_label_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_owner.isSet()) {
            isObjectUpdated = true;
            break;
        }

        if (m_size_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_size_active_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_size_busy_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIModel_instance_pool_info::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
