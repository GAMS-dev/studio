#include "neosmanager.h"
#include "xmlrpc.h"
#include <QString>
#include <iostream>

namespace gams {
namespace studio {
namespace neos {

QString rawJob =
R"s1(
<document>
<category>lp</category>
<solver>BDMLP</solver>
<priority>%3</priority>
<inputType>GAMS</inputType>
<model><![CDATA[]]></model>
<options><![CDATA[]]></options>
<parameters><![CDATA[fw=1%2]]></parameters>
<restart><base64>%1</base64></restart>
<wantlog><![CDATA[yes]]></wantlog>
<wantlst><![CDATA[yes]]></wantlst>
<wantgdx><![CDATA[%4]]></wantgdx>
</document>
)s1";


NeosManager::NeosManager(QObject* parent)
    : QObject(parent), mHttp(this)
{
    QMetaEnum meta = QMetaEnum::fromType<NeosCall>();
    for (int i = 0; i < meta.keyCount(); ++i) {
        QString c = QString(meta.key(i));
        neosCalls.insert(c.right(c.length()-1), NeosCall(meta.value(i)));
    }

    connect(&mHttp, &HttpManager::received, this, &NeosManager::received);
    connect(&mHttp, &HttpManager::sslErrors, this, &NeosManager::sslErrors);

    mPullTimer.setInterval(1000);
    mPullTimer.setSingleShot(true);
    connect(&mPullTimer, &QTimer::timeout, this, &NeosManager::pull);
}

void NeosManager::setUrl(const QString &url)
{
    mHttp.setUrl(url);
}

void NeosManager::ping()
{
    mHttp.submitCall("ping");
}

void NeosManager::version()
{
    mHttp.submitCall("version");
}

void NeosManager::submitJob(QString fileName, QString params, bool prioShort, bool wantGdx)
{
    QFile f(fileName);
    if (!f.exists() || !f.open(QFile::ReadOnly)) return;
    QByteArray data = f.readAll();
    f.close();
    mLogOffset = 0;
    QString sData = data.toBase64();
    QString prio = (prioShort?"short":"long");
    QString jobData = rawJob.arg(sData).arg(params).arg(prio).arg(wantGdx?"yes":"no");
    mHttp.submitCall("submitJob", QVariantList() << jobData);
}

void NeosManager::watchJob(int jobNumber, QString password)
{
    mJobNumber = jobNumber;
    mPassword = password;
    getJobStatus();
}

void NeosManager::getJobStatus()
{
    //  "Done", "Running", "Waiting", "Unknown Job", or "Bad Password"
    mHttp.submitCall("getJobStatus", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::getCompletionCode()
{
    mHttp.submitCall("getCompletionCode", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::getJobInfo()
{
    mHttp.submitCall("getJobInfo", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::killJob()
{
    mHttp.submitCall("killJob", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::getIntermediateResults()
{
    mHttp.submitCall("getIntermediateResults", QVariantList() << mJobNumber << mPassword << mLogOffset);
}

void NeosManager::getFinalResultsNonBlocking()
{
    mHttp.submitCall("getFinalResultsNonBlocking", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::getOutputFile(QString fileName)
{
    mHttp.submitCall("getOutputFile", QVariantList() << mJobNumber << mPassword << fileName);
}

void NeosManager::setDebug(bool debug)
{
    if (debug)
        connect(&mHttp, &HttpManager::received, this, &NeosManager::debugReceived, Qt::UniqueConnection);
    else
        disconnect(&mHttp, &HttpManager::received, this, &NeosManager::debugReceived);
}

void NeosManager::sslErrors(const QStringList &errors)
{
    qDebug() << "SSL errors occurred:\n" << errors.join("\n").toLatin1().data();
}

void NeosManager::received(QString name, QVariant data, bool isReply)
{
    Q_UNUSED(isReply)
    NeosCall c = neosCalls.value(name);
    QVariantList list = data.toList();
    switch (c) {
    case _ping:
        emit rePing(list.size()>0 ? list.at(0).toString() : QString("%1: no data returned").arg(name));
        break;
    case _version:
        emit reVersion(list.size()>0 ? list.at(0).toString() : QString("%1: no data returned").arg(name));
        break;
    case _submitJob: {
        if (list.size() > 1) {
            watchJob(list.at(0).toInt(), list.at(1).toString());
        }
    }   break;
    case _getJobStatus: {
        if (list.size() > 0) {
            QString status = list.at(0).toString().toLower();
            if (status == "running" || status == "waiting") {
                if (!mPullTimer.isActive()) mPullTimer.start();
            } else {
                if (mPullTimer.isActive()) mPullTimer.stop();
            }
            emit reGetJobStatus(status);
        }
    }   break;
    case _getCompletionCode:
        emit reGetCompletionCode(list.size()>0 ? list.at(0).toString() : QString("%1: no data returned").arg(name));
        break;
    case _getJobInfo: {
        emit reGetJobInfo(list.size()>0 ? list.at(0).toString() : QString("%1: no data returned").arg(name),
                          list.size()>1 ? list.at(1).toString() : QString("%1: no data returned").arg(name),
                          list.size()>2 ? list.at(2).toString() : QString("%1: no data returned").arg(name),
                          list.size()>3 ? list.at(3).toString() : QString("%1: no data returned").arg(name),
                          list.size()>4 ? list.at(4).toString() : QString("%1: no data returned").arg(name));
    }   break;
    case _killJob:
        emit reKillJob();
        break;
    case _getIntermediateResults: {
        if (list.size()>1) mLogOffset = list.at(1).toInt();
        emit reGetIntermediateResults(list.size()>0 ? list.at(0).toByteArray() : QByteArray());
    }   break;
    case _getFinalResultsNonBlocking:
        emit reGetFinalResultsNonBlocking(list.size()>0 ? list.at(0).toByteArray() : QByteArray());
        break;
    case _getOutputFile: {
        emit reGetOutputFile(list.size()>0 ? list.at(0).toByteArray() : QByteArray());
    }   break;
    default:
        break;
    }
}

void NeosManager::pull()
{
    getIntermediateResults();
    getJobStatus();
}

void NeosManager::debugReceived(QString name, QVariant data, bool isReply)
{
    QString kind = isReply ? "Result from " : "Call to ";
    qDebug() << "\n" << kind.toLatin1().data() << name << ":\n" << data;
}


} // namespace neos
} // namespace studio
} // namespace gams
