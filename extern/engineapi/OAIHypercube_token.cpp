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

#include "OAIHypercube_token.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIHypercube_token::OAIHypercube_token(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIHypercube_token::OAIHypercube_token() {
    this->initializeModel();
}

OAIHypercube_token::~OAIHypercube_token() {}

void OAIHypercube_token::initializeModel() {

    m_hypercube_token_isSet = false;
    m_hypercube_token_isValid = false;

    m_quota_warning_isSet = false;
    m_quota_warning_isValid = false;
}

void OAIHypercube_token::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIHypercube_token::fromJsonObject(QJsonObject json) {

    m_hypercube_token_isValid = ::OpenAPI::fromJsonValue(m_hypercube_token, json[QString("hypercube_token")]);
    m_hypercube_token_isSet = !json[QString("hypercube_token")].isNull() && m_hypercube_token_isValid;

    m_quota_warning_isValid = ::OpenAPI::fromJsonValue(m_quota_warning, json[QString("quota_warning")]);
    m_quota_warning_isSet = !json[QString("quota_warning")].isNull() && m_quota_warning_isValid;
}

QString OAIHypercube_token::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIHypercube_token::asJsonObject() const {
    QJsonObject obj;
    if (m_hypercube_token_isSet) {
        obj.insert(QString("hypercube_token"), ::OpenAPI::toJsonValue(m_hypercube_token));
    }
    if (m_quota_warning.size() > 0) {
        obj.insert(QString("quota_warning"), ::OpenAPI::toJsonValue(m_quota_warning));
    }
    return obj;
}

QString OAIHypercube_token::getHypercubeToken() const {
    return m_hypercube_token;
}
void OAIHypercube_token::setHypercubeToken(const QString &hypercube_token) {
    m_hypercube_token = hypercube_token;
    m_hypercube_token_isSet = true;
}

bool OAIHypercube_token::is_hypercube_token_Set() const{
    return m_hypercube_token_isSet;
}

bool OAIHypercube_token::is_hypercube_token_Valid() const{
    return m_hypercube_token_isValid;
}

QList<OAIQuota> OAIHypercube_token::getQuotaWarning() const {
    return m_quota_warning;
}
void OAIHypercube_token::setQuotaWarning(const QList<OAIQuota> &quota_warning) {
    m_quota_warning = quota_warning;
    m_quota_warning_isSet = true;
}

bool OAIHypercube_token::is_quota_warning_Set() const{
    return m_quota_warning_isSet;
}

bool OAIHypercube_token::is_quota_warning_Valid() const{
    return m_quota_warning_isValid;
}

bool OAIHypercube_token::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_hypercube_token_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_quota_warning.size() > 0) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIHypercube_token::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
