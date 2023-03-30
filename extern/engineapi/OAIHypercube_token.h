/**
 * GAMS Engine
 * With GAMS Engine you can register and solve GAMS models. It has a namespace management system, so you can restrict your users to certain models.
 *
 * The version of the OpenAPI document: 23.02.18
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

/*
 * OAIHypercube_token.h
 *
 * 
 */

#ifndef OAIHypercube_token_H
#define OAIHypercube_token_H

#include <QJsonObject>

#include "OAIQuota.h"
#include <QList>
#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIHypercube_token : public OAIObject {
public:
    OAIHypercube_token();
    OAIHypercube_token(QString json);
    ~OAIHypercube_token() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getHypercubeToken() const;
    void setHypercubeToken(const QString &hypercube_token);
    bool is_hypercube_token_Set() const;
    bool is_hypercube_token_Valid() const;

    QList<OAIQuota> getQuotaWarning() const;
    void setQuotaWarning(const QList<OAIQuota> &quota_warning);
    bool is_quota_warning_Set() const;
    bool is_quota_warning_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString hypercube_token;
    bool m_hypercube_token_isSet;
    bool m_hypercube_token_isValid;

    QList<OAIQuota> quota_warning;
    bool m_quota_warning_isSet;
    bool m_quota_warning_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIHypercube_token)

#endif // OAIHypercube_token_H