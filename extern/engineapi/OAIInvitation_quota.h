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
 * OAIInvitation_quota.h
 *
 * 
 */

#ifndef OAIInvitation_quota_H
#define OAIInvitation_quota_H

#include <QJsonObject>


#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIInvitation_quota : public OAIObject {
public:
    OAIInvitation_quota();
    OAIInvitation_quota(QString json);
    ~OAIInvitation_quota() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    qint32 getDiskQuota() const;
    void setDiskQuota(const qint32 &disk_quota);
    bool is_disk_quota_Set() const;
    bool is_disk_quota_Valid() const;

    double getParallelQuota() const;
    void setParallelQuota(const double &parallel_quota);
    bool is_parallel_quota_Set() const;
    bool is_parallel_quota_Valid() const;

    double getVolumeQuota() const;
    void setVolumeQuota(const double &volume_quota);
    bool is_volume_quota_Set() const;
    bool is_volume_quota_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    qint32 disk_quota;
    bool m_disk_quota_isSet;
    bool m_disk_quota_isValid;

    double parallel_quota;
    bool m_parallel_quota_isSet;
    bool m_parallel_quota_isValid;

    double volume_quota;
    bool m_volume_quota_isSet;
    bool m_volume_quota_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIInvitation_quota)

#endif // OAIInvitation_quota_H
