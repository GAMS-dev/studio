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
        _getIntermediateResults,
        _getFinalResultsNonBlocking,
        _getOutputFile
    };
    Q_ENUM(NeosCall)

public:
    NeosManager(QObject *parent = nullptr);
    void setUrl(const QString &url);

    void ping();
    void version();
    void submitJob(QString fileName, QString params = QString(), bool prioShort = true, bool wantGdx = true);
    void watchJob(int jobNumber, QString password);
    void getJobStatus();
    void getCompletionCode();
    void getJobInfo();
    void killJob();
    void getIntermediateResults();
    void getFinalResultsNonBlocking();
    void getOutputFile(QString fileName);

    void setDebug(bool debug = true);

signals:
    void rePing(const QString &value);
    void reVersion(const QString &value);
    void reSubmitJob(const int &jobNumber, const QString &jobPassword);
    void reGetJobStatus(const QString &value);
    void reGetCompletionCode(const QString &value);
    void reGetJobInfo(const QString &category, const QString &solverName, const QString &input, const QString &status,
                      const QString &completionCode);
    void reKillJob();
    void reGetIntermediateResults(const QByteArray &data);
    void reGetFinalResultsNonBlocking(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);

private slots:
    void sslErrors(const QStringList &errors);
    void received(QString name, QVariant data, bool isReply);
    void debugReceived(QString name, QVariant data, bool isReply);
    void pull();
private:
    HttpManager mHttp;
    QHash<QString, NeosCall> neosCalls;
    QTimer mPullTimer;
    int mJobNumber = 0;
    QString mPassword;
    int mLogOffset = 0;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSMANAGER_H
