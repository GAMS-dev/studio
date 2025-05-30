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

#include "OAIJob_no_text_entry.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIJob_no_text_entry::OAIJob_no_text_entry(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIJob_no_text_entry::OAIJob_no_text_entry() {
    this->initializeModel();
}

OAIJob_no_text_entry::~OAIJob_no_text_entry() {}

void OAIJob_no_text_entry::initializeModel() {

    m_arguments_isSet = false;
    m_arguments_isValid = false;

    m_finished_at_isSet = false;
    m_finished_at_isValid = false;

    m_is_data_provided_isSet = false;
    m_is_data_provided_isValid = false;

    m_is_temporary_model_isSet = false;
    m_is_temporary_model_isValid = false;

    m_model_isSet = false;
    m_model_isValid = false;

    m_r_namespace_isSet = false;
    m_r_namespace_isValid = false;

    m_process_status_isSet = false;
    m_process_status_isValid = false;

    m_status_isSet = false;
    m_status_isValid = false;

    m_stdout_filename_isSet = false;
    m_stdout_filename_isValid = false;

    m_stream_entries_isSet = false;
    m_stream_entries_isValid = false;

    m_submitted_at_isSet = false;
    m_submitted_at_isValid = false;

    m_tag_isSet = false;
    m_tag_isValid = false;

    m_token_isSet = false;
    m_token_isValid = false;

    m_user_isSet = false;
    m_user_isValid = false;
}

void OAIJob_no_text_entry::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIJob_no_text_entry::fromJsonObject(QJsonObject json) {

    m_arguments_isValid = ::OpenAPI::fromJsonValue(m_arguments, json[QString("arguments")]);
    m_arguments_isSet = !json[QString("arguments")].isNull() && m_arguments_isValid;

    m_finished_at_isValid = ::OpenAPI::fromJsonValue(m_finished_at, json[QString("finished_at")]);
    m_finished_at_isSet = !json[QString("finished_at")].isNull() && m_finished_at_isValid;

    m_is_data_provided_isValid = ::OpenAPI::fromJsonValue(m_is_data_provided, json[QString("is_data_provided")]);
    m_is_data_provided_isSet = !json[QString("is_data_provided")].isNull() && m_is_data_provided_isValid;

    m_is_temporary_model_isValid = ::OpenAPI::fromJsonValue(m_is_temporary_model, json[QString("is_temporary_model")]);
    m_is_temporary_model_isSet = !json[QString("is_temporary_model")].isNull() && m_is_temporary_model_isValid;

    m_model_isValid = ::OpenAPI::fromJsonValue(m_model, json[QString("model")]);
    m_model_isSet = !json[QString("model")].isNull() && m_model_isValid;

    m_r_namespace_isValid = ::OpenAPI::fromJsonValue(m_r_namespace, json[QString("namespace")]);
    m_r_namespace_isSet = !json[QString("namespace")].isNull() && m_r_namespace_isValid;

    m_process_status_isValid = ::OpenAPI::fromJsonValue(m_process_status, json[QString("process_status")]);
    m_process_status_isSet = !json[QString("process_status")].isNull() && m_process_status_isValid;

    m_status_isValid = ::OpenAPI::fromJsonValue(m_status, json[QString("status")]);
    m_status_isSet = !json[QString("status")].isNull() && m_status_isValid;

    m_stdout_filename_isValid = ::OpenAPI::fromJsonValue(m_stdout_filename, json[QString("stdout_filename")]);
    m_stdout_filename_isSet = !json[QString("stdout_filename")].isNull() && m_stdout_filename_isValid;

    m_stream_entries_isValid = ::OpenAPI::fromJsonValue(m_stream_entries, json[QString("stream_entries")]);
    m_stream_entries_isSet = !json[QString("stream_entries")].isNull() && m_stream_entries_isValid;

    m_submitted_at_isValid = ::OpenAPI::fromJsonValue(m_submitted_at, json[QString("submitted_at")]);
    m_submitted_at_isSet = !json[QString("submitted_at")].isNull() && m_submitted_at_isValid;

    m_tag_isValid = ::OpenAPI::fromJsonValue(m_tag, json[QString("tag")]);
    m_tag_isSet = !json[QString("tag")].isNull() && m_tag_isValid;

    m_token_isValid = ::OpenAPI::fromJsonValue(m_token, json[QString("token")]);
    m_token_isSet = !json[QString("token")].isNull() && m_token_isValid;

    m_user_isValid = ::OpenAPI::fromJsonValue(m_user, json[QString("user")]);
    m_user_isSet = !json[QString("user")].isNull() && m_user_isValid;
}

QString OAIJob_no_text_entry::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIJob_no_text_entry::asJsonObject() const {
    QJsonObject obj;
    if (m_arguments.size() > 0) {
        obj.insert(QString("arguments"), ::OpenAPI::toJsonValue(m_arguments));
    }
    if (m_finished_at_isSet) {
        obj.insert(QString("finished_at"), ::OpenAPI::toJsonValue(m_finished_at));
    }
    if (m_is_data_provided_isSet) {
        obj.insert(QString("is_data_provided"), ::OpenAPI::toJsonValue(m_is_data_provided));
    }
    if (m_is_temporary_model_isSet) {
        obj.insert(QString("is_temporary_model"), ::OpenAPI::toJsonValue(m_is_temporary_model));
    }
    if (m_model_isSet) {
        obj.insert(QString("model"), ::OpenAPI::toJsonValue(m_model));
    }
    if (m_r_namespace_isSet) {
        obj.insert(QString("namespace"), ::OpenAPI::toJsonValue(m_r_namespace));
    }
    if (m_process_status_isSet) {
        obj.insert(QString("process_status"), ::OpenAPI::toJsonValue(m_process_status));
    }
    if (m_status_isSet) {
        obj.insert(QString("status"), ::OpenAPI::toJsonValue(m_status));
    }
    if (m_stdout_filename_isSet) {
        obj.insert(QString("stdout_filename"), ::OpenAPI::toJsonValue(m_stdout_filename));
    }
    if (m_stream_entries.size() > 0) {
        obj.insert(QString("stream_entries"), ::OpenAPI::toJsonValue(m_stream_entries));
    }
    if (m_submitted_at_isSet) {
        obj.insert(QString("submitted_at"), ::OpenAPI::toJsonValue(m_submitted_at));
    }
    if (m_tag_isSet) {
        obj.insert(QString("tag"), ::OpenAPI::toJsonValue(m_tag));
    }
    if (m_token_isSet) {
        obj.insert(QString("token"), ::OpenAPI::toJsonValue(m_token));
    }
    if (m_user.isSet()) {
        obj.insert(QString("user"), ::OpenAPI::toJsonValue(m_user));
    }
    return obj;
}

QList<QString> OAIJob_no_text_entry::getArguments() const {
    return m_arguments;
}
void OAIJob_no_text_entry::setArguments(const QList<QString> &arguments) {
    m_arguments = arguments;
    m_arguments_isSet = true;
}

bool OAIJob_no_text_entry::is_arguments_Set() const{
    return m_arguments_isSet;
}

bool OAIJob_no_text_entry::is_arguments_Valid() const{
    return m_arguments_isValid;
}

QDateTime OAIJob_no_text_entry::getFinishedAt() const {
    return m_finished_at;
}
void OAIJob_no_text_entry::setFinishedAt(const QDateTime &finished_at) {
    m_finished_at = finished_at;
    m_finished_at_isSet = true;
}

bool OAIJob_no_text_entry::is_finished_at_Set() const{
    return m_finished_at_isSet;
}

bool OAIJob_no_text_entry::is_finished_at_Valid() const{
    return m_finished_at_isValid;
}

bool OAIJob_no_text_entry::isIsDataProvided() const {
    return m_is_data_provided;
}
void OAIJob_no_text_entry::setIsDataProvided(const bool &is_data_provided) {
    m_is_data_provided = is_data_provided;
    m_is_data_provided_isSet = true;
}

bool OAIJob_no_text_entry::is_is_data_provided_Set() const{
    return m_is_data_provided_isSet;
}

bool OAIJob_no_text_entry::is_is_data_provided_Valid() const{
    return m_is_data_provided_isValid;
}

bool OAIJob_no_text_entry::isIsTemporaryModel() const {
    return m_is_temporary_model;
}
void OAIJob_no_text_entry::setIsTemporaryModel(const bool &is_temporary_model) {
    m_is_temporary_model = is_temporary_model;
    m_is_temporary_model_isSet = true;
}

bool OAIJob_no_text_entry::is_is_temporary_model_Set() const{
    return m_is_temporary_model_isSet;
}

bool OAIJob_no_text_entry::is_is_temporary_model_Valid() const{
    return m_is_temporary_model_isValid;
}

QString OAIJob_no_text_entry::getModel() const {
    return m_model;
}
void OAIJob_no_text_entry::setModel(const QString &model) {
    m_model = model;
    m_model_isSet = true;
}

bool OAIJob_no_text_entry::is_model_Set() const{
    return m_model_isSet;
}

bool OAIJob_no_text_entry::is_model_Valid() const{
    return m_model_isValid;
}

QString OAIJob_no_text_entry::getRNamespace() const {
    return m_r_namespace;
}
void OAIJob_no_text_entry::setRNamespace(const QString &r_namespace) {
    m_r_namespace = r_namespace;
    m_r_namespace_isSet = true;
}

bool OAIJob_no_text_entry::is_r_namespace_Set() const{
    return m_r_namespace_isSet;
}

bool OAIJob_no_text_entry::is_r_namespace_Valid() const{
    return m_r_namespace_isValid;
}

qint32 OAIJob_no_text_entry::getProcessStatus() const {
    return m_process_status;
}
void OAIJob_no_text_entry::setProcessStatus(const qint32 &process_status) {
    m_process_status = process_status;
    m_process_status_isSet = true;
}

bool OAIJob_no_text_entry::is_process_status_Set() const{
    return m_process_status_isSet;
}

bool OAIJob_no_text_entry::is_process_status_Valid() const{
    return m_process_status_isValid;
}

qint32 OAIJob_no_text_entry::getStatus() const {
    return m_status;
}
void OAIJob_no_text_entry::setStatus(const qint32 &status) {
    m_status = status;
    m_status_isSet = true;
}

bool OAIJob_no_text_entry::is_status_Set() const{
    return m_status_isSet;
}

bool OAIJob_no_text_entry::is_status_Valid() const{
    return m_status_isValid;
}

QString OAIJob_no_text_entry::getStdoutFilename() const {
    return m_stdout_filename;
}
void OAIJob_no_text_entry::setStdoutFilename(const QString &stdout_filename) {
    m_stdout_filename = stdout_filename;
    m_stdout_filename_isSet = true;
}

bool OAIJob_no_text_entry::is_stdout_filename_Set() const{
    return m_stdout_filename_isSet;
}

bool OAIJob_no_text_entry::is_stdout_filename_Valid() const{
    return m_stdout_filename_isValid;
}

QList<QString> OAIJob_no_text_entry::getStreamEntries() const {
    return m_stream_entries;
}
void OAIJob_no_text_entry::setStreamEntries(const QList<QString> &stream_entries) {
    m_stream_entries = stream_entries;
    m_stream_entries_isSet = true;
}

bool OAIJob_no_text_entry::is_stream_entries_Set() const{
    return m_stream_entries_isSet;
}

bool OAIJob_no_text_entry::is_stream_entries_Valid() const{
    return m_stream_entries_isValid;
}

QDateTime OAIJob_no_text_entry::getSubmittedAt() const {
    return m_submitted_at;
}
void OAIJob_no_text_entry::setSubmittedAt(const QDateTime &submitted_at) {
    m_submitted_at = submitted_at;
    m_submitted_at_isSet = true;
}

bool OAIJob_no_text_entry::is_submitted_at_Set() const{
    return m_submitted_at_isSet;
}

bool OAIJob_no_text_entry::is_submitted_at_Valid() const{
    return m_submitted_at_isValid;
}

QString OAIJob_no_text_entry::getTag() const {
    return m_tag;
}
void OAIJob_no_text_entry::setTag(const QString &tag) {
    m_tag = tag;
    m_tag_isSet = true;
}

bool OAIJob_no_text_entry::is_tag_Set() const{
    return m_tag_isSet;
}

bool OAIJob_no_text_entry::is_tag_Valid() const{
    return m_tag_isValid;
}

QString OAIJob_no_text_entry::getToken() const {
    return m_token;
}
void OAIJob_no_text_entry::setToken(const QString &token) {
    m_token = token;
    m_token_isSet = true;
}

bool OAIJob_no_text_entry::is_token_Set() const{
    return m_token_isSet;
}

bool OAIJob_no_text_entry::is_token_Valid() const{
    return m_token_isValid;
}

OAIModel_user OAIJob_no_text_entry::getUser() const {
    return m_user;
}
void OAIJob_no_text_entry::setUser(const OAIModel_user &user) {
    m_user = user;
    m_user_isSet = true;
}

bool OAIJob_no_text_entry::is_user_Set() const{
    return m_user_isSet;
}

bool OAIJob_no_text_entry::is_user_Valid() const{
    return m_user_isValid;
}

bool OAIJob_no_text_entry::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_arguments.size() > 0) {
            isObjectUpdated = true;
            break;
        }

        if (m_finished_at_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_is_data_provided_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_is_temporary_model_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_model_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_r_namespace_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_process_status_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_status_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_stdout_filename_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_stream_entries.size() > 0) {
            isObjectUpdated = true;
            break;
        }

        if (m_submitted_at_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_tag_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_token_isSet) {
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

bool OAIJob_no_text_entry::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
