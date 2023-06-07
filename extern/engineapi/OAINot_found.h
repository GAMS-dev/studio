/**
 * GAMS Engine
 * With GAMS Engine you can register and solve GAMS models. It has a namespace management system, so you can restrict your users to certain models.
 *
 * The version of the OpenAPI document: 23.06.02
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

/*
 * OAINot_found.h
 *
 * 
 */

#ifndef OAINot_found_H
#define OAINot_found_H

#include <QJsonObject>

#include <QList>
#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAINot_found : public OAIObject {
public:
    OAINot_found();
    OAINot_found(QString json);
    ~OAINot_found() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getEntity() const;
    void setEntity(const QString &entity);
    bool is_entity_Set() const;
    bool is_entity_Valid() const;

    QString getMessage() const;
    void setMessage(const QString &message);
    bool is_message_Set() const;
    bool is_message_Valid() const;

    QList<QString> getNames() const;
    void setNames(const QList<QString> &names);
    bool is_names_Set() const;
    bool is_names_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString entity;
    bool m_entity_isSet;
    bool m_entity_isValid;

    QString message;
    bool m_message_isSet;
    bool m_message_isValid;

    QList<QString> names;
    bool m_names_isSet;
    bool m_names_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAINot_found)

#endif // OAINot_found_H
