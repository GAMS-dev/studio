/**
 * GAMS Studio
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
#ifndef VERSIONINFOLOADER_H
#define VERSIONINFOLOADER_H

#include <QObject>
#include <QByteArray>
#include <QFuture>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>

namespace gams {
namespace studio {
namespace support {

///
/// \brief Fetches GAMS and GAMS Studio version information from
///        <a href="https://gams.com">https://gams.com</a>.
///
class VersionInfoLoader : public QObject
{
    Q_OBJECT

public:
    explicit VersionInfoLoader(QObject* parent = nullptr);

    virtual ~VersionInfoLoader();

    void requestRemoteData();

    int remoteDistribVersion() const;

    void setRemoteDistribVersion(int value);

    QString remoteDistribVersionString() const;

    void setRemoteDistribVersionString(const QString& value);

    int remoteStudioVersion() const;

    void setRemoteStudioVersion(int value);

    QString remoteStudioVersionString() const;

    void setRemoteStudioVersionString(const QString& value);

    QMap<int, qint64> distribVersions() const;

    QString errorString();

signals:
    void continueProcessing();

    void finished();

    void newErrorMessage(const QString&);

private slots:
    void requestStudioInfo();
    void distribDownloadFinished(QNetworkReply* reply);
    void studioDownloadFinished(QNetworkReply* reply);
    void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

private:
    void writeDataToLog(const QByteArray &data);

private:
    static const QString DistribVersionFile;
    static const QString StudioVersionFile;

    QNetworkAccessManager* mWebCtrlDistrib;
    QNetworkAccessManager* mWebCtrlStudio;

    QSslConfiguration mSslConf;

    QRegularExpression mStudioRegEx;

    int mRemoteDistribVersion = 0;
    QString mRemoteDistribVersionString;
    int mRemoteStudioVersion = 0;
    QString mRemoteStudioVersionString;
    QStringList mErrorStrings;
    QMap<int, qint64> mDistribVersions;

    QFuture<void> mResult;
};

}
}
}

#endif // VERSIONINFOLOADER_H
