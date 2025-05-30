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

#include "OAIModel_hypercube_usage.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIModel_hypercube_usage::OAIModel_hypercube_usage(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIModel_hypercube_usage::OAIModel_hypercube_usage() {
    this->initializeModel();
}

OAIModel_hypercube_usage::~OAIModel_hypercube_usage() {}

void OAIModel_hypercube_usage::initializeModel() {

    m_completed_isSet = false;
    m_completed_isValid = false;

    m_finished_isSet = false;
    m_finished_isValid = false;

    m_job_count_isSet = false;
    m_job_count_isValid = false;

    m_jobs_isSet = false;
    m_jobs_isValid = false;

    m_labels_isSet = false;
    m_labels_isValid = false;

    m_model_isSet = false;
    m_model_isValid = false;

    m_r_namespace_isSet = false;
    m_r_namespace_isValid = false;

    m_status_isSet = false;
    m_status_isValid = false;

    m_submitted_isSet = false;
    m_submitted_isValid = false;

    m_token_isSet = false;
    m_token_isValid = false;

    m_username_isSet = false;
    m_username_isValid = false;
}

void OAIModel_hypercube_usage::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIModel_hypercube_usage::fromJsonObject(QJsonObject json) {

    m_completed_isValid = ::OpenAPI::fromJsonValue(m_completed, json[QString("completed")]);
    m_completed_isSet = !json[QString("completed")].isNull() && m_completed_isValid;

    m_finished_isValid = ::OpenAPI::fromJsonValue(m_finished, json[QString("finished")]);
    m_finished_isSet = !json[QString("finished")].isNull() && m_finished_isValid;

    m_job_count_isValid = ::OpenAPI::fromJsonValue(m_job_count, json[QString("job_count")]);
    m_job_count_isSet = !json[QString("job_count")].isNull() && m_job_count_isValid;

    m_jobs_isValid = ::OpenAPI::fromJsonValue(m_jobs, json[QString("jobs")]);
    m_jobs_isSet = !json[QString("jobs")].isNull() && m_jobs_isValid;

    m_labels_isValid = ::OpenAPI::fromJsonValue(m_labels, json[QString("labels")]);
    m_labels_isSet = !json[QString("labels")].isNull() && m_labels_isValid;

    m_model_isValid = ::OpenAPI::fromJsonValue(m_model, json[QString("model")]);
    m_model_isSet = !json[QString("model")].isNull() && m_model_isValid;

    m_r_namespace_isValid = ::OpenAPI::fromJsonValue(m_r_namespace, json[QString("namespace")]);
    m_r_namespace_isSet = !json[QString("namespace")].isNull() && m_r_namespace_isValid;

    m_status_isValid = ::OpenAPI::fromJsonValue(m_status, json[QString("status")]);
    m_status_isSet = !json[QString("status")].isNull() && m_status_isValid;

    m_submitted_isValid = ::OpenAPI::fromJsonValue(m_submitted, json[QString("submitted")]);
    m_submitted_isSet = !json[QString("submitted")].isNull() && m_submitted_isValid;

    m_token_isValid = ::OpenAPI::fromJsonValue(m_token, json[QString("token")]);
    m_token_isSet = !json[QString("token")].isNull() && m_token_isValid;

    m_username_isValid = ::OpenAPI::fromJsonValue(m_username, json[QString("username")]);
    m_username_isSet = !json[QString("username")].isNull() && m_username_isValid;
}

QString OAIModel_hypercube_usage::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIModel_hypercube_usage::asJsonObject() const {
    QJsonObject obj;
    if (m_completed_isSet) {
        obj.insert(QString("completed"), ::OpenAPI::toJsonValue(m_completed));
    }
    if (m_finished_isSet) {
        obj.insert(QString("finished"), ::OpenAPI::toJsonValue(m_finished));
    }
    if (m_job_count_isSet) {
        obj.insert(QString("job_count"), ::OpenAPI::toJsonValue(m_job_count));
    }
    if (m_jobs.size() > 0) {
        obj.insert(QString("jobs"), ::OpenAPI::toJsonValue(m_jobs));
    }
    if (m_labels.isSet()) {
        obj.insert(QString("labels"), ::OpenAPI::toJsonValue(m_labels));
    }
    if (m_model_isSet) {
        obj.insert(QString("model"), ::OpenAPI::toJsonValue(m_model));
    }
    if (m_r_namespace_isSet) {
        obj.insert(QString("namespace"), ::OpenAPI::toJsonValue(m_r_namespace));
    }
    if (m_status_isSet) {
        obj.insert(QString("status"), ::OpenAPI::toJsonValue(m_status));
    }
    if (m_submitted_isSet) {
        obj.insert(QString("submitted"), ::OpenAPI::toJsonValue(m_submitted));
    }
    if (m_token_isSet) {
        obj.insert(QString("token"), ::OpenAPI::toJsonValue(m_token));
    }
    if (m_username_isSet) {
        obj.insert(QString("username"), ::OpenAPI::toJsonValue(m_username));
    }
    return obj;
}

qint32 OAIModel_hypercube_usage::getCompleted() const {
    return m_completed;
}
void OAIModel_hypercube_usage::setCompleted(const qint32 &completed) {
    m_completed = completed;
    m_completed_isSet = true;
}

bool OAIModel_hypercube_usage::is_completed_Set() const{
    return m_completed_isSet;
}

bool OAIModel_hypercube_usage::is_completed_Valid() const{
    return m_completed_isValid;
}

QDateTime OAIModel_hypercube_usage::getFinished() const {
    return m_finished;
}
void OAIModel_hypercube_usage::setFinished(const QDateTime &finished) {
    m_finished = finished;
    m_finished_isSet = true;
}

bool OAIModel_hypercube_usage::is_finished_Set() const{
    return m_finished_isSet;
}

bool OAIModel_hypercube_usage::is_finished_Valid() const{
    return m_finished_isValid;
}

qint32 OAIModel_hypercube_usage::getJobCount() const {
    return m_job_count;
}
void OAIModel_hypercube_usage::setJobCount(const qint32 &job_count) {
    m_job_count = job_count;
    m_job_count_isSet = true;
}

bool OAIModel_hypercube_usage::is_job_count_Set() const{
    return m_job_count_isSet;
}

bool OAIModel_hypercube_usage::is_job_count_Valid() const{
    return m_job_count_isValid;
}

QList<OAIModel_hypercube_job> OAIModel_hypercube_usage::getJobs() const {
    return m_jobs;
}
void OAIModel_hypercube_usage::setJobs(const QList<OAIModel_hypercube_job> &jobs) {
    m_jobs = jobs;
    m_jobs_isSet = true;
}

bool OAIModel_hypercube_usage::is_jobs_Set() const{
    return m_jobs_isSet;
}

bool OAIModel_hypercube_usage::is_jobs_Valid() const{
    return m_jobs_isValid;
}

OAIModel_job_labels OAIModel_hypercube_usage::getLabels() const {
    return m_labels;
}
void OAIModel_hypercube_usage::setLabels(const OAIModel_job_labels &labels) {
    m_labels = labels;
    m_labels_isSet = true;
}

bool OAIModel_hypercube_usage::is_labels_Set() const{
    return m_labels_isSet;
}

bool OAIModel_hypercube_usage::is_labels_Valid() const{
    return m_labels_isValid;
}

QString OAIModel_hypercube_usage::getModel() const {
    return m_model;
}
void OAIModel_hypercube_usage::setModel(const QString &model) {
    m_model = model;
    m_model_isSet = true;
}

bool OAIModel_hypercube_usage::is_model_Set() const{
    return m_model_isSet;
}

bool OAIModel_hypercube_usage::is_model_Valid() const{
    return m_model_isValid;
}

QString OAIModel_hypercube_usage::getRNamespace() const {
    return m_r_namespace;
}
void OAIModel_hypercube_usage::setRNamespace(const QString &r_namespace) {
    m_r_namespace = r_namespace;
    m_r_namespace_isSet = true;
}

bool OAIModel_hypercube_usage::is_r_namespace_Set() const{
    return m_r_namespace_isSet;
}

bool OAIModel_hypercube_usage::is_r_namespace_Valid() const{
    return m_r_namespace_isValid;
}

qint32 OAIModel_hypercube_usage::getStatus() const {
    return m_status;
}
void OAIModel_hypercube_usage::setStatus(const qint32 &status) {
    m_status = status;
    m_status_isSet = true;
}

bool OAIModel_hypercube_usage::is_status_Set() const{
    return m_status_isSet;
}

bool OAIModel_hypercube_usage::is_status_Valid() const{
    return m_status_isValid;
}

QDateTime OAIModel_hypercube_usage::getSubmitted() const {
    return m_submitted;
}
void OAIModel_hypercube_usage::setSubmitted(const QDateTime &submitted) {
    m_submitted = submitted;
    m_submitted_isSet = true;
}

bool OAIModel_hypercube_usage::is_submitted_Set() const{
    return m_submitted_isSet;
}

bool OAIModel_hypercube_usage::is_submitted_Valid() const{
    return m_submitted_isValid;
}

QString OAIModel_hypercube_usage::getToken() const {
    return m_token;
}
void OAIModel_hypercube_usage::setToken(const QString &token) {
    m_token = token;
    m_token_isSet = true;
}

bool OAIModel_hypercube_usage::is_token_Set() const{
    return m_token_isSet;
}

bool OAIModel_hypercube_usage::is_token_Valid() const{
    return m_token_isValid;
}

QString OAIModel_hypercube_usage::getUsername() const {
    return m_username;
}
void OAIModel_hypercube_usage::setUsername(const QString &username) {
    m_username = username;
    m_username_isSet = true;
}

bool OAIModel_hypercube_usage::is_username_Set() const{
    return m_username_isSet;
}

bool OAIModel_hypercube_usage::is_username_Valid() const{
    return m_username_isValid;
}

bool OAIModel_hypercube_usage::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_completed_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_finished_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_job_count_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_jobs.size() > 0) {
            isObjectUpdated = true;
            break;
        }

        if (m_labels.isSet()) {
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

        if (m_status_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_submitted_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_token_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_username_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIModel_hypercube_usage::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
