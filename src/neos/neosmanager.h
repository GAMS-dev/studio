#ifndef GAMS_STUDIO_NEOS_NEOSMANAGER_H
#define GAMS_STUDIO_NEOS_NEOSMANAGER_H

#include <QHash>
#include "httpmanager.h"

namespace gams {
namespace studio {
namespace neos {

class NeosManager: public QObject
{
    Q_OBJECT
public:
    enum NeosCall {
        _ping,
        _version,
        _submitJob,
        _getJobStatus,
        _getCompletionCode,
        _getJobInfo,
        _killJob,
        _getIntermediateResultsNonBlocking,
        _getFinalResultsNonBlocking,
        _getOutputFile
    };
    Q_ENUM(NeosCall)

public:
    NeosManager(QObject *parent = nullptr);
    void setUrl(const QString &url);
    void setIgnoreSslErrors();
    bool ignoreSslErrors();

    void ping();
    void version();
    void submitJob(QString fileName, QString params = QString(), bool prioShort = true, bool wantGdx = true);
    void watchJob(int jobNumber, QString password);
    void getJobStatus();
    void getCompletionCode();
    void getJobInfo();
    void killJob(bool &ok);
    void getIntermediateResultsNonBlocking();
    void getFinalResultsNonBlocking();
    void getOutputFile(QString fileName);

    void setDebug(bool debug = true);

signals:
    void submitCall(const QString &method, const QVariantList &params = QVariantList());
    void rePing(const QString &value);
    void reVersion(const QString &value);
    void reSubmitJob(const int &jobNumber, const QString &jobPassword);
    void reGetJobStatus(const QString &value);
    void reGetCompletionCode(const QString &value);
    void reGetJobInfo(const QStringList &info);
    void reKillJob(const QString &text);
    void reGetIntermediateResultsNonBlocking(const QByteArray &data);
    void reGetFinalResultsNonBlocking(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);
    void sslErrors(const QStringList &errors);

private slots:
    void received(QString name, QVariant data);
    void debugReceived(QString name, QVariant data);
private:
    HttpManager mHttp;
    QHash<QString, NeosCall> neosCalls;
    int mJobNumber = 0;
    QString mPassword;
    int mLogOffset = 0;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSMANAGER_H
