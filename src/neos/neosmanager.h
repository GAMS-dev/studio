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
    bool submitJob(const QString &fileName, const QString &eMail, const QString &params = QString(),
                   bool prioShort = true, bool wantGdx = true);
    void watchJob(int jobNumber, const QString &password);
    void getJobStatus();
    void getCompletionCode();
    void getJobInfo();
    void killJob(bool &ok);
    void getIntermediateResultsNonBlocking();
    void getFinalResultsNonBlocking();
    void getOutputFile(const QString &fileName);

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
    void received(const QString &name, const QVariant &data);
    void debugReceived(const QString &name, const QVariant &data);
private:
    static QString mRawJob;
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
