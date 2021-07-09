/**
 * GAMS Engine
 * With GAMS Engine you can register and solve GAMS models. It has a namespace management system, so you can restrict your users to certain models.
 *
 * The version of the OpenAPI document: latest
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

/*
 * OAIJob.h
 *
 * 
 */

#ifndef OAIJob_H
#define OAIJob_H

#include <QJsonObject>

#include "OAIModel_job_labels.h"
#include "OAIResult_user.h"
#include "OAIText_entry.h"
#include <QDateTime>
#include <QList>
#include <QString>

#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIJob : public OAIObject {
public:
    OAIJob();
    OAIJob(QString json);
    ~OAIJob() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QList<QString> getArguments() const;
    void setArguments(const QList<QString> &arguments);
    bool is_arguments_Set() const;
    bool is_arguments_Valid() const;

    QList<QString> getDepTokens() const;
    void setDepTokens(const QList<QString> &dep_tokens);
    bool is_dep_tokens_Set() const;
    bool is_dep_tokens_Valid() const;

    QDateTime getFinishedAt() const;
    void setFinishedAt(const QDateTime &finished_at);
    bool is_finished_at_Set() const;
    bool is_finished_at_Valid() const;

    bool isIsDataProvided() const;
    void setIsDataProvided(const bool &is_data_provided);
    bool is_is_data_provided_Set() const;
    bool is_is_data_provided_Valid() const;

    bool isIsTemporaryModel() const;
    void setIsTemporaryModel(const bool &is_temporary_model);
    bool is_is_temporary_model_Set() const;
    bool is_is_temporary_model_Valid() const;

    OAIModel_job_labels getLabels() const;
    void setLabels(const OAIModel_job_labels &labels);
    bool is_labels_Set() const;
    bool is_labels_Valid() const;

    QString getModel() const;
    void setModel(const QString &model);
    bool is_model_Set() const;
    bool is_model_Valid() const;

    QString getRNamespace() const;
    void setRNamespace(const QString &r_namespace);
    bool is_r_namespace_Set() const;
    bool is_r_namespace_Valid() const;

    qint32 getProcessStatus() const;
    void setProcessStatus(const qint32 &process_status);
    bool is_process_status_Set() const;
    bool is_process_status_Valid() const;

    bool isResultExists() const;
    void setResultExists(const bool &result_exists);
    bool is_result_exists_Set() const;
    bool is_result_exists_Valid() const;

    qint32 getStatus() const;
    void setStatus(const qint32 &status);
    bool is_status_Set() const;
    bool is_status_Valid() const;

    QString getStdoutFilename() const;
    void setStdoutFilename(const QString &stdout_filename);
    bool is_stdout_filename_Set() const;
    bool is_stdout_filename_Valid() const;

    QList<QString> getStreamEntries() const;
    void setStreamEntries(const QList<QString> &stream_entries);
    bool is_stream_entries_Set() const;
    bool is_stream_entries_Valid() const;

    QDateTime getSubmittedAt() const;
    void setSubmittedAt(const QDateTime &submitted_at);
    bool is_submitted_at_Set() const;
    bool is_submitted_at_Valid() const;

    QList<OAIText_entry> getTextEntries() const;
    void setTextEntries(const QList<OAIText_entry> &text_entries);
    bool is_text_entries_Set() const;
    bool is_text_entries_Valid() const;

    QString getToken() const;
    void setToken(const QString &token);
    bool is_token_Set() const;
    bool is_token_Valid() const;

    OAIResult_user getUser() const;
    void setUser(const OAIResult_user &user);
    bool is_user_Set() const;
    bool is_user_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QList<QString> arguments;
    bool m_arguments_isSet;
    bool m_arguments_isValid;

    QList<QString> dep_tokens;
    bool m_dep_tokens_isSet;
    bool m_dep_tokens_isValid;

    QDateTime finished_at;
    bool m_finished_at_isSet;
    bool m_finished_at_isValid;

    bool is_data_provided;
    bool m_is_data_provided_isSet;
    bool m_is_data_provided_isValid;

    bool is_temporary_model;
    bool m_is_temporary_model_isSet;
    bool m_is_temporary_model_isValid;

    OAIModel_job_labels labels;
    bool m_labels_isSet;
    bool m_labels_isValid;

    QString model;
    bool m_model_isSet;
    bool m_model_isValid;

    QString r_namespace;
    bool m_r_namespace_isSet;
    bool m_r_namespace_isValid;

    qint32 process_status;
    bool m_process_status_isSet;
    bool m_process_status_isValid;

    bool result_exists;
    bool m_result_exists_isSet;
    bool m_result_exists_isValid;

    qint32 status;
    bool m_status_isSet;
    bool m_status_isValid;

    QString stdout_filename;
    bool m_stdout_filename_isSet;
    bool m_stdout_filename_isValid;

    QList<QString> stream_entries;
    bool m_stream_entries_isSet;
    bool m_stream_entries_isValid;

    QDateTime submitted_at;
    bool m_submitted_at_isSet;
    bool m_submitted_at_isValid;

    QList<OAIText_entry> text_entries;
    bool m_text_entries_isSet;
    bool m_text_entries_isValid;

    QString token;
    bool m_token_isSet;
    bool m_token_isValid;

    OAIResult_user user;
    bool m_user_isSet;
    bool m_user_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIJob)

#endif // OAIJob_H
