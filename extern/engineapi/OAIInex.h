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
 * OAIInex.h
 *
 * 
 */

#ifndef OAIInex_H
#define OAIInex_H

#include <QJsonObject>

#include <QList>
#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIInex : public OAIObject {
public:
    OAIInex();
    OAIInex(QString json);
    ~OAIInex() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QList<QString> getFiles() const;
    void setFiles(const QList<QString> &files);
    bool is_files_Set() const;
    bool is_files_Valid() const;

    QString getType() const;
    void setType(const QString &type);
    bool is_type_Set() const;
    bool is_type_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QList<QString> files;
    bool m_files_isSet;
    bool m_files_isValid;

    QString type;
    bool m_type_isSet;
    bool m_type_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIInex)

#endif // OAIInex_H