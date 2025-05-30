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

/*
 * OAISystem_wide_license.h
 *
 * 
 */

#ifndef OAISystem_wide_license_H
#define OAISystem_wide_license_H

#include <QJsonObject>

#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAISystem_wide_license : public OAIObject {
public:
    OAISystem_wide_license();
    OAISystem_wide_license(QString json);
    ~OAISystem_wide_license() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getLicense() const;
    void setLicense(const QString &license);
    bool is_license_Set() const;
    bool is_license_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString m_license;
    bool m_license_isSet;
    bool m_license_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAISystem_wide_license)

#endif // OAISystem_wide_license_H
