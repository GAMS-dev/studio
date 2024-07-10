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
#ifndef GAMS_STUDIO_ENGINE_AUTHMANAGER_H
#define GAMS_STUDIO_ENGINE_AUTHMANAGER_H

#include <QObject>
#include <QVariant>
#include <QTimer>
#include <QUrl>
#include <QMutex>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

namespace gams {
namespace studio {
namespace engine {

class EngineManager;

struct AuthProvider {
    AuthProvider() {}
    AuthProvider(const QHash<QString, QVariant> &authProvider);
    // initial data
    QString name;
    QString label;
    QUrl authUrl;
    QUrl tokenUrl;
    QString clientId;
    QStringList scopes;
    bool hasSecret = false;
    // added data
    int pollInterval = 5;
    int expires = 600;
    QString deviceCode;
};

class AuthManager: public QObject
{
    Q_OBJECT
public:
    explicit AuthManager(EngineManager *engineManager, QObject *parent = nullptr);
    ~AuthManager() override;
    void listProvider(const QString &ssoName);
    void deviceAuthRequest(const QString &authName);
    void deviceAuthRequest(int providerIndex);
    void checkVerification();
    void deviceAccessTokenRequest();

signals:
    void showVerificationCode(const QString &userCode, const QString &verifyUri, const QString &verifyUriComplete);
    void reListProvider(const QList<QHash<QString, QVariant> > &allProvider);
    void reListProviderError(const QString &message);
    void error(const QString &message);
    void reDeviceAccessToken(const QString &accessToken);

private slots:
    void reDeviceAuthRequest(QNetworkReply *reply);
    void reDeviceAccessTokenRequest(QNetworkReply *reply);
    void processProviderList(const QList<QHash<QString, QVariant> > &allProvider);
    void reFetchOAuth2Token(const QString &idToken);
    void reFetchOAuth2TokenError(int responseCode, const QString &errorMsg);

private:
    QNetworkRequest createRequest(const QUrl &uri);
    QByteArray createContent(const QHash<QString, QVariant> &data);
    void freeReply();
    void processReply(QNetworkReply *reply, int &responseCode, QJsonObject &content);
    QJsonObject toJsonObject(const QByteArray &jsonString);

private:
    QNetworkAccessManager *mNetworkManager;
    EngineManager *mEngineManager = nullptr;
    QList<QHash<QString, QVariant>> mProviderList;
    QString mExplicitProvider;
    AuthProvider mAuthProvider;
    QNetworkReply *mCurrentReply = nullptr;
    QMutex mReplyMutex;
    QTimer mTimeoutTimer;
    QTimer mPingTimer;

};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ENGINE_AUTHMANAGER_H
