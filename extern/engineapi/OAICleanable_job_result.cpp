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

#include "OAICleanable_job_result.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAICleanable_job_result::OAICleanable_job_result(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAICleanable_job_result::OAICleanable_job_result() {
    this->initializeModel();
}

OAICleanable_job_result::~OAICleanable_job_result() {}

void OAICleanable_job_result::initializeModel() {

    m_filename_isSet = false;
    m_filename_isValid = false;

    m_length_isSet = false;
    m_length_isValid = false;

    m_r_namespace_isSet = false;
    m_r_namespace_isValid = false;

    m_token_isSet = false;
    m_token_isValid = false;

    m_type_isSet = false;
    m_type_isValid = false;

    m_upload_date_isSet = false;
    m_upload_date_isValid = false;

    m_user_isSet = false;
    m_user_isValid = false;
}

void OAICleanable_job_result::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAICleanable_job_result::fromJsonObject(QJsonObject json) {

    m_filename_isValid = ::OpenAPI::fromJsonValue(m_filename, json[QString("filename")]);
    m_filename_isSet = !json[QString("filename")].isNull() && m_filename_isValid;

    m_length_isValid = ::OpenAPI::fromJsonValue(m_length, json[QString("length")]);
    m_length_isSet = !json[QString("length")].isNull() && m_length_isValid;

    m_r_namespace_isValid = ::OpenAPI::fromJsonValue(m_r_namespace, json[QString("namespace")]);
    m_r_namespace_isSet = !json[QString("namespace")].isNull() && m_r_namespace_isValid;

    m_token_isValid = ::OpenAPI::fromJsonValue(m_token, json[QString("token")]);
    m_token_isSet = !json[QString("token")].isNull() && m_token_isValid;

    m_type_isValid = ::OpenAPI::fromJsonValue(m_type, json[QString("type")]);
    m_type_isSet = !json[QString("type")].isNull() && m_type_isValid;

    m_upload_date_isValid = ::OpenAPI::fromJsonValue(m_upload_date, json[QString("upload_date")]);
    m_upload_date_isSet = !json[QString("upload_date")].isNull() && m_upload_date_isValid;

    m_user_isValid = ::OpenAPI::fromJsonValue(m_user, json[QString("user")]);
    m_user_isSet = !json[QString("user")].isNull() && m_user_isValid;
}

QString OAICleanable_job_result::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAICleanable_job_result::asJsonObject() const {
    QJsonObject obj;
    if (m_filename_isSet) {
        obj.insert(QString("filename"), ::OpenAPI::toJsonValue(m_filename));
    }
    if (m_length_isSet) {
        obj.insert(QString("length"), ::OpenAPI::toJsonValue(m_length));
    }
    if (m_r_namespace_isSet) {
        obj.insert(QString("namespace"), ::OpenAPI::toJsonValue(m_r_namespace));
    }
    if (m_token_isSet) {
        obj.insert(QString("token"), ::OpenAPI::toJsonValue(m_token));
    }
    if (m_type_isSet) {
        obj.insert(QString("type"), ::OpenAPI::toJsonValue(m_type));
    }
    if (m_upload_date_isSet) {
        obj.insert(QString("upload_date"), ::OpenAPI::toJsonValue(m_upload_date));
    }
    if (m_user.isSet()) {
        obj.insert(QString("user"), ::OpenAPI::toJsonValue(m_user));
    }
    return obj;
}

QString OAICleanable_job_result::getFilename() const {
    return m_filename;
}
void OAICleanable_job_result::setFilename(const QString &filename) {
    m_filename = filename;
    m_filename_isSet = true;
}

bool OAICleanable_job_result::is_filename_Set() const{
    return m_filename_isSet;
}

bool OAICleanable_job_result::is_filename_Valid() const{
    return m_filename_isValid;
}

qint32 OAICleanable_job_result::getLength() const {
    return m_length;
}
void OAICleanable_job_result::setLength(const qint32 &length) {
    m_length = length;
    m_length_isSet = true;
}

bool OAICleanable_job_result::is_length_Set() const{
    return m_length_isSet;
}

bool OAICleanable_job_result::is_length_Valid() const{
    return m_length_isValid;
}

QString OAICleanable_job_result::getRNamespace() const {
    return m_r_namespace;
}
void OAICleanable_job_result::setRNamespace(const QString &r_namespace) {
    m_r_namespace = r_namespace;
    m_r_namespace_isSet = true;
}

bool OAICleanable_job_result::is_r_namespace_Set() const{
    return m_r_namespace_isSet;
}

bool OAICleanable_job_result::is_r_namespace_Valid() const{
    return m_r_namespace_isValid;
}

QString OAICleanable_job_result::getToken() const {
    return m_token;
}
void OAICleanable_job_result::setToken(const QString &token) {
    m_token = token;
    m_token_isSet = true;
}

bool OAICleanable_job_result::is_token_Set() const{
    return m_token_isSet;
}

bool OAICleanable_job_result::is_token_Valid() const{
    return m_token_isValid;
}

QString OAICleanable_job_result::getType() const {
    return m_type;
}
void OAICleanable_job_result::setType(const QString &type) {
    m_type = type;
    m_type_isSet = true;
}

bool OAICleanable_job_result::is_type_Set() const{
    return m_type_isSet;
}

bool OAICleanable_job_result::is_type_Valid() const{
    return m_type_isValid;
}

QDateTime OAICleanable_job_result::getUploadDate() const {
    return m_upload_date;
}
void OAICleanable_job_result::setUploadDate(const QDateTime &upload_date) {
    m_upload_date = upload_date;
    m_upload_date_isSet = true;
}

bool OAICleanable_job_result::is_upload_date_Set() const{
    return m_upload_date_isSet;
}

bool OAICleanable_job_result::is_upload_date_Valid() const{
    return m_upload_date_isValid;
}

OAIModel_user OAICleanable_job_result::getUser() const {
    return m_user;
}
void OAICleanable_job_result::setUser(const OAIModel_user &user) {
    m_user = user;
    m_user_isSet = true;
}

bool OAICleanable_job_result::is_user_Set() const{
    return m_user_isSet;
}

bool OAICleanable_job_result::is_user_Valid() const{
    return m_user_isValid;
}

bool OAICleanable_job_result::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_filename_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_length_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_r_namespace_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_token_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_type_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_upload_date_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_user.isSet()) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAICleanable_job_result::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
