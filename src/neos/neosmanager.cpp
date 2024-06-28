/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "neosmanager.h"
#include "xmlrpc.h"
#include <QString>
#include <iostream>

namespace gams {
namespace studio {
namespace neos {

QString NeosManager::mRawJob =
R"s1(
<document>
<category>milp</category>
<solver>cbc</solver>
<inputType>GAMS</inputType>
<email>%5</email>
<priority>%3</priority>
<model><![CDATA[]]></model>
<options><![CDATA[]]></options>
<parameters><![CDATA[%2]]></parameters>
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

    connect(&mHttp, &HttpManager::received, this, &NeosManager::received, Qt::QueuedConnection);
    connect(&mHttp, &HttpManager::sslErrors, this, &NeosManager::sslErrors, Qt::QueuedConnection);
    connect(&mHttp, &HttpManager::error, this, &NeosManager::reError, Qt::QueuedConnection);
    connect(this, &NeosManager::submitCall, &mHttp, &HttpManager::submitCall);
}

void NeosManager::setUrl(const QString &url)
{
    mHttp.setUrl(url);
}

void NeosManager::setIgnoreSslErrors()
{
    mHttp.setIgnoreSslErrors();
}

bool NeosManager::ignoreSslErrors()
{
    return mHttp.ignoreSslErrors();
}

void NeosManager::ping()
{
    emit submitCall("ping");
}

void NeosManager::version()
{
    emit submitCall("version");
}

bool NeosManager::submitJob(const QString &fileName, const QString &eMail, const QString &params, bool prioShort, bool wantGdx)
{
    QFile f(fileName);
    if (!f.exists() || !f.open(QFile::ReadOnly)) {
        emit reGetIntermediateResultsNonBlocking(QString("Can't open %1").arg(fileName).toUtf8());
        return false;
    }
    QByteArray data = f.readAll();
    f.close();
    mLogOffset = 0;
    QString sData = data.toBase64();
    QString prio = (prioShort?"short":"long");
    QString jobData = mRawJob.arg(sData, params, prio, wantGdx?"yes":"", eMail);
    emit submitCall("submitJob", QVariantList() << jobData);
    return true;
}

void NeosManager::watchJob(int jobNumber, const QString &password)
{
    mJobNumber = jobNumber;
    mPassword = password;
    getJobStatus();
}

void NeosManager::getJobStatus()
{
    //  "Done", "Running", "Waiting", "Unknown Job", or "Bad Password"
    emit submitCall("getJobStatus", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::getCompletionCode()
{
    // Only if Job is "Done":
    //  "Normal", "Out of memory", "Timed out", "Disk Space", "Server error", "Unknown Job", "Bad Password"
    emit submitCall("getCompletionCode", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::getJobInfo()
{
    // tuple (category, solver_name, input, status, completion_code)
    emit submitCall("getJobInfo", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::killJob(bool &ok)
{
    if ((ok = mJobNumber))
        emit submitCall("killJob", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::getIntermediateResultsNonBlocking()
{
    emit submitCall("getIntermediateResultsNonBlocking", QVariantList() << mJobNumber << mPassword << mLogOffset);
}

void NeosManager::getFinalResultsNonBlocking()
{
    emit submitCall("getFinalResultsNonBlocking", QVariantList() << mJobNumber << mPassword);
}

void NeosManager::getOutputFile(const QString &fileName)
{
    emit submitCall("getOutputFile", QVariantList() << mJobNumber << mPassword << fileName);
}

void NeosManager::setDebug(bool debug)
{
    if (debug)
        connect(&mHttp, &HttpManager::received, this, &NeosManager::debugReceived, Qt::UniqueConnection);
    else
        disconnect(&mHttp, &HttpManager::received, this, &NeosManager::debugReceived);
}

void NeosManager::received(const QString &name, const QVariant &data)
{
    NeosCall c = neosCalls.value(name);
    QVariantList list = data.toList();
    switch (c) {
    case _ping:
        if (list.size() > 0) {
            emit rePing(list.at(0).toString());
        } break;
    case _version:
        if (list.size() > 0) {
            emit reVersion(list.at(0).toString());
        } break;
    case _submitJob:
        if (list.size() > 0) {
            QVariantList dat = list.at(0).toList();
            watchJob(dat.at(0).toInt(), dat.at(1).toString());
            emit reSubmitJob(dat.at(0).toInt(), dat.at(1).toString());
            getJobInfo();
        } break;
    case _getJobStatus:
        if (list.size() > 0) {
            QString status = list.at(0).toString().toLower();
            emit reGetJobStatus(status);
        } break;
    case _getCompletionCode:
        if (list.size() > 0) {
            emit reGetCompletionCode(list.at(0).toString());
        } break;
    case _getJobInfo:
        if (list.size() > 0) {
            QStringList dat;
            const auto vars = list.at(0).toList();
            for (const QVariant &var : vars) {
                dat << var.toString();
            }
            if (dat.size() > 4) {
                emit reGetJobInfo(dat);
            }
        } break;
    case _killJob:
        qDebug() << "result from KillJob " << data;
        if (list.size() > 0) {
            emit reKillJob(list.at(0).toString());
        }   break;
    case _getIntermediateResultsNonBlocking:
        if (list.size() > 0) {
            QVariantList dat = list.at(0).toList();
            if (dat.size() > 1) {
                mLogOffset = dat.at(1).toInt();
                emit reGetIntermediateResultsNonBlocking(dat.at(0).toByteArray());
            }
        } break;
    case _getFinalResultsNonBlocking:
        if (list.size() > 0) {
            emit reGetFinalResultsNonBlocking(list.at(0).toByteArray());
        } break;
    case _getOutputFile:
        if (list.size() > 0) {
            emit reGetOutputFile(list.at(0).toByteArray());
        } break;
    }
}

void NeosManager::debugReceived(const QString &name, const QVariant &data)
{
    qDebug() << "\nResult from " << name << ":\n" << data;
}


} // namespace neos
} // namespace studio
} // namespace gams
