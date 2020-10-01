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

/*
 * OAIText_entry.h
 *
 * 
 */

#ifndef OAIText_entry_H
#define OAIText_entry_H

#include <QJsonObject>

#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIText_entry : public OAIObject {
public:
    OAIText_entry();
    OAIText_entry(QString json);
    ~OAIText_entry() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getEntryName() const;
    void setEntryName(const QString &entry_name);

    QString getEntryValue() const;
    void setEntryValue(const QString &entry_value);

    qint32 getEntrySize() const;
    void setEntrySize(const qint32 &entry_size);

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString entry_name;
    bool m_entry_name_isSet;
    bool m_entry_name_isValid;

    QString entry_value;
    bool m_entry_value_isSet;
    bool m_entry_value_isValid;

    qint32 entry_size;
    bool m_entry_size_isSet;
    bool m_entry_size_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIText_entry)

#endif // OAIText_entry_H
