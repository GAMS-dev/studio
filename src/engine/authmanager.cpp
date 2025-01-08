/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "authmanager.h"
#include "enginemanager.h"
#include "exception.h"
#include "networkmanager.h"
#include "logger.h"

#include <QNetworkAccessManager>
#include <QMutexLocker>
#include <QHash>

namespace gams {
namespace studio {
namespace engine {

///
/// \brief AuthProvider::AuthProvider Gathers data of authentification provider
/// \param authProvider a variant hash with the initial provider data from \ref AuthManager::listProvider
///
AuthProvider::AuthProvider(const QHash<QString, QVariant> &authProvider)
{
    name = authProvider.value("name").toString();
    label = authProvider.value("label").toString();
    authUrl = authProvider.value("authEndpoint").toString();
    tokenUrl = authProvider.value("tokenEndpoint").toString();
    clientId = authProvider.value("clientId").toString();
    scopes = authProvider.value("scopes").toStringList();
    hasSecret = authProvider.value("hasSecret").toBool();
    pollInterval = 5;
}

///
/// \brief AuthManager::AuthManager construct
/// \param engineManager the backend connection, \see EngineManager
/// \param parent the parent passed to the inherited \ref QObject
///
AuthManager::AuthManager(EngineManager *engineManager, QObject *parent)
    : QObject(parent)
    , mEngineManager(engineManager)
{
    if (!mEngineManager) EXCEPT() << "AuthManager: the EngineManager must not be null";
    mNetworkManager = NetworkManager::manager();
    if (!mNetworkManager) EXCEPT() << "AuthManager: the NetworkManager must not be null";
    mTimeoutTimer.setSingleShot(true);
    mTimeoutTimer.setInterval(10000);
    connect(mEngineManager, &EngineManager::reListProvider, this, &AuthManager::processProviderList);
    connect(mEngineManager, &EngineManager::reListProviderError, this, &AuthManager::reListProviderError);
    connect(mEngineManager, &EngineManager::reFetchOAuth2Token, this, &AuthManager::reFetchOAuth2Token);
    connect(mEngineManager, &EngineManager::reFetchOAuth2TokenError, this, &AuthManager::reFetchOAuth2TokenError);
}

///
/// \brief AuthManager::~AuthManager destructor
///
AuthManager::~AuthManager()
{
//    disconnect(&mTimeoutTimer, &QTimer::timeout);
    mTimeoutTimer.stop();
}

///
/// \brief AuthManager::listProvider lists all visible providers or a particular invisible one
/// \param ssoName the name of a particular provider
///
void AuthManager::listProvider(const QString &ssoName)
{
    mExplicitProvider = ssoName;
    mEngineManager->listProvider(ssoName);
}

///
/// \brief AuthManager::processProviderList processes the reply of \ref listProvider
/// \param allProvider
///
/// The provider list is passed to the signal \ref reListProvider if no explicit ssoName has been set with the request.
/// Otherwise an hidden explicit provider has been selected. In this case a \ref deviceAuthRequest will be send.
///
void AuthManager::processProviderList(const QList<QHash<QString, QVariant> > &allProvider)
{
    mProviderList = allProvider;
    if (mExplicitProvider.isEmpty()) {
        // trigger getting initially visible providers
        emit reListProvider(allProvider);
        return;
    }

    // Hidden explicit provider - must be only ONE
    if (allProvider.count() != 1) {
        if (allProvider.isEmpty())
            emit reListProviderError("No provider returned for " + mExplicitProvider);
        else
            emit reListProviderError("More than one provider returned for " + mExplicitProvider);
        return;
    }
    deviceAuthRequest(0);
}

///
/// \brief AuthManager::deviceAuthRequest posts an "Device Authorization Request" to the selected provider authorization endpoint
/// \param authName the provider name in the latest request of \ref listProvider
///
/// The POST packet contains:
///
/// - client_id: received from \see listProvider
///
void AuthManager::deviceAuthRequest(const QString &authName)
{
    int index = -1;
    for (int i = 0; i < mProviderList.count(); ++i) {
        auto provider = mProviderList.at(i);
        if (provider.value("name").toString().compare(authName) != 0)
            continue;
        index = i;
        break;
    }
    if (index < 0) return;
    deviceAuthRequest(index);
}

///
/// \brief AuthManager::deviceAuthRequest posts an "Device Authorization Request" to the selected provider authorization endpoint
/// \param providerIndex the provider index in the latest request of \ref listProvider
///
/// The POST packet contains:
///
/// - client_id: received from \see listProvider
///
void AuthManager::deviceAuthRequest(int providerIndex)
{
    if (providerIndex < 0 || providerIndex >= mProviderList.size()) {
        EXCEPT() << "Invalid provider index";
    }
    if (mCurrentReply)
        freeReply(); // frees and disconnects the signal

    mAuthProvider = mProviderList.at(providerIndex);
    QNetworkRequest request = createRequest(mAuthProvider.authUrl);
    QByteArray content = createContent({{"client_id", mAuthProvider.clientId}, {"scope", mAuthProvider.scopes}});

    QNetworkReply *reply = mNetworkManager->post(request, content);

    { // frame mutexlocker lifetime
        QMutexLocker locker(&mReplyMutex);
        mCurrentReply = reply;
        mCurrentReply->setParent(this);
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        mTimeoutTimer.stop();
        reDeviceAuthRequest(reply);
    });
    if (mTimeoutTimer.interval() > 0) {
        connect(&mTimeoutTimer, &QTimer::timeout, this, [this] {
            freeReply();
            emit error("Timeout during device authorization request for '" + mAuthProvider.name + "'");
        });
        mTimeoutTimer.start();
    }
}

///
/// \brief AuthManager::reDeviceAuthRequest process reply from POST request \ref AuthManager::deviceAuthRequest
/// \param reply the reply data contains
///        - device_code
///        - user_code
///        - verification_uri
///        - verification_uri_complete
///        - interval
///
void AuthManager::reDeviceAuthRequest(QNetworkReply *reply)
{
    int responseCode;
    QJsonObject content;
    processReply(reply, responseCode, content);
    freeReply();
    if (responseCode != 200) {
        if (responseCode < 0) {
            QString msg = QString("Communication to server failed: %1");
            msg = msg.arg(responseCode == -1 ? "Invalid response code"
                                             : responseCode == -2 ? "Multipart message unsupported"
                                                                  : "Packed content unsupported");
            emit error(msg);
        } else {
            emit error(QString("Server response code: %1").arg(responseCode));
        }
        return;
    }

    mAuthProvider.deviceCode = content.value("device_code").toString();
    if (mAuthProvider.deviceCode.isEmpty()) {
        emit error(QString("Invalid device authentification reply: missing device code"));
        return;
    }

    QString userCode = content.value("user_code").toString();
    QString verifyUri = content.contains("verification_uri") ? content.value("verification_uri").toString()
                                                             : content.value("verification_url").toString();
    QString verifyUriC = content.contains("verification_uri_complete") ? content.value("verification_uri_complete").toString()
                                                                       : content.value("verification_url_complete").toString();
    emit showVerificationCode(userCode, verifyUri, verifyUriC);
    mAuthProvider.pollInterval = content.value("interval").toInt(5);
    mAuthProvider.expires = content.value("expires_in").toInt(600);
    mTimeoutTimer.setInterval(mAuthProvider.expires * 1000);
    if (mTimeoutTimer.interval() > 0) {
        connect(&mTimeoutTimer, &QTimer::timeout, this, [this] {
            freeReply();
            emit error("Timeout during device access token request for '" + mAuthProvider.name + "'");
        });
        mTimeoutTimer.start();
    }
    checkVerification();
}

///
/// \brief AuthManager::checkVerification requests the access token if the verification has been successful (POLL)
///
/// Depending on the flag "hasSecret" the request is handled differently:
///
/// - If "hasSecret" is false, the POST request is send directly to the authorization provider to get the access_token.
///
/// - If "hasSecret" is true, this requests is send to the Engine server as a proxy to enhance the request by the
/// secret which is not known by Studio
///
/// To succeed, this request needs a successful user verification on an external side. Thus it polls in timed cycles
/// according to the "interval" field of the device authorisation request (defaults to 5 seconds).
///
void AuthManager::checkVerification()
{
    if (mAuthProvider.hasSecret) {
        freeReply();
        mEngineManager->fetchOAuth2Token(mAuthProvider.name, mAuthProvider.deviceCode);
    } else {
        deviceAccessTokenRequest();
    }
}

///
/// \brief AuthManager::reFetchOAuth2Token
/// \param idToken
///
void AuthManager::reFetchOAuth2Token(const QString &idToken)
{
    mTimeoutTimer.stop();
    DEB() << "Device connected";
    emit reDeviceAccessToken(idToken);
}

///
/// \brief AuthManager::reFetchOAuth2TokenError
/// \param error
///
void AuthManager::reFetchOAuth2TokenError(int responseCode, const QString &errorMsg)
{
    if (responseCode == 400 || responseCode == 428) {
        // retry
        QTimer::singleShot(1000 * mAuthProvider.pollInterval, this, &AuthManager::checkVerification);
        return;
    }
    DEB() << "response " << responseCode << ": " << errorMsg;
    emit error(errorMsg);
}

///
/// \brief AuthManager::deviceAccessTokenRequest posts a "" to the selected providers token endpoint
///
/// The POST packet contains:
///
/// - grant_type: fixed value 'urn:ietf:params:oauth:grant-type:device_code'
/// - device_code: received from \ref deviceAuthRequest
/// - client_id: received from \see listProvider
///
///
void AuthManager::deviceAccessTokenRequest()
{
    QByteArray content = createContent({
        {"grant_type", "urn:ietf:params:oauth:grant-type:device_code"},
        {"device_code", mAuthProvider.deviceCode},
        {"client_id", mAuthProvider.clientId},
    });
    QNetworkRequest request = createRequest(mAuthProvider.tokenUrl);
    QNetworkReply *reply = mNetworkManager->post(request, content);

    { // frame mutexlocker lifetime
        QMutexLocker locker(&mReplyMutex);
        mCurrentReply = reply;
        mCurrentReply->setParent(this);
    }
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        mTimeoutTimer.stop();
        reDeviceAccessTokenRequest(reply);
    });
}

///
/// \brief AuthManager::reDeviceAccessTokenRequest handles reply for access token request
/// \param reply
///
void AuthManager::reDeviceAccessTokenRequest(QNetworkReply *reply)
{
    int responseCode;
    QJsonObject content;
    processReply(reply, responseCode, content);
    freeReply();
    mTimeoutTimer.stop();

    if (responseCode != 200) {
        if (responseCode < 0) {
            QString msg = QString("Communication to server failed: %1");
            msg = msg.arg(responseCode == -1 ? "Invalid response code"
                          : responseCode == -2 ? "Multipart message unsupported"
                                               : "Packed content unsupported");
            emit error(msg);
        } else if (responseCode == 400) {
            QString errorCode = content.value("error").toString();
            if (errorCode.compare("slow_down") == 0 || errorCode.compare("authorization_pending") == 0) {
                if (errorCode.compare("slow_down") == 0)
                    mAuthProvider.pollInterval += 5;
                QTimer::singleShot(1000 * mAuthProvider.pollInterval, this, &AuthManager::deviceAccessTokenRequest);
            } else {
                QString errorText = content.value("error_description").toString();
                emit error("Error for '" + mAuthProvider.name + "': " + errorText);
                // TODO(JM) we may also show the "error_uri" link for more information
            }

        } else {
            emit error(QString("Server response code: %1").arg(responseCode));
        }
        return;
    }

    QString accessToken = content.value("access_token").toString();
    emit reDeviceAccessToken(accessToken);
    // access_token
    // token_type
    // expires_in
}

///
/// \brief AuthManager::createRequest prepares a common \ref QNetworkRequest
/// \param uri the target uri of the request
/// \return the prepared \ref QNetworkRequest
///
QNetworkRequest AuthManager::createRequest(const QUrl &uri)
{
    QNetworkRequest request = QNetworkRequest(uri);
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setRawHeader("User-Agent", "GamsStudio/1.0.0/cpp-qt");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept-Encoding", "identity");
    return request;
}
///
/// \brief AuthManager::createJsonContent formats the QHash into a JSON message
/// \param data the source hash data
/// \return the resulting JSON message
///
QByteArray AuthManager::createContent(const QHash<QString, QVariant> &data)
{
    QByteArray res;
    for (auto it =  data.constBegin() ; it != data.constEnd() ; ++it) {
        if (!it.value().isValid()) continue;
        QString value;
        switch (it.value().metaType().id()) {
        case QMetaType::Bool:
            value = it.value().toBool()  ? "TRUE" : "FALSE";
            break;
        case QMetaType::Int:
            value = QString::number(it.value().toInt());
            break;
        case QMetaType::QString:
            value = it.value().toString();
            break;
        case QMetaType::QStringList:
            value = it.value().toStringList().join(" ");
            break;
        default:
            break;
        }
        if (!res.isEmpty())
            res += "&";
        res += QString("%1=%2").arg(QUrl::toPercentEncoding(it.key()), QUrl::toPercentEncoding(value)).toUtf8();
    }
    return res;
}

///
/// \brief AuthManager::freeReply frees the memory of the reply under mutex protection
///
void AuthManager::freeReply()
{
    if (mCurrentReply) {
        QMutexLocker locker(&mReplyMutex);
        mCurrentReply->deleteLater();
        mCurrentReply = nullptr;
    }
}

///
/// \brief AuthManager::processReply extracts the data from a \ref QNetworkReply
/// \param reply [in] the source reply
/// \param responseCode [out] the response code of the reply
/// \param content [out] the content of the reply in JSON format
///
void AuthManager::processReply(QNetworkReply *reply, int &responseCode, QJsonObject &content)
{
    QMutexLocker locker(&mReplyMutex);
    QString typeHeader;
    QString encodingHeader;
    if (reply->rawHeaderPairs().count() > 0) {
        for (const auto &item : reply->rawHeaderPairs()) {
            QString key = item.first;
            if(key.compare(QString("Content-Type"), Qt::CaseInsensitive) == 0)
                typeHeader = item.second;
            if(key.compare(QString("Content-Encoding"), Qt::CaseInsensitive) == 0)
                encodingHeader = item.second;
        }
    }
    bool ok;
    responseCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    if(!ok) responseCode = -1;
    if (responseCode == 400) {
        content = toJsonObject(reply->readAll());
        return;
    }

    if (!typeHeader.isEmpty()) {
        QStringList typeData = typeHeader.split(QString(";"), Qt::SkipEmptyParts);
        if ((typeData.count() > 0) && (typeData.first() == QString("multipart/form-data"))) {
            responseCode = -2;
        } else {
            QByteArray data;
            if(!encodingHeader.isEmpty()) {
                QStringList encoding = encodingHeader.split(QString(";"), Qt::SkipEmptyParts);
                if (encoding.count() > 0) {
                    QStringList compressTypes = encoding.first().split(',', Qt::SkipEmptyParts);
                    if (compressTypes.contains("gzip", Qt::CaseInsensitive) || compressTypes.contains("deflate", Qt::CaseInsensitive))
                        responseCode = -3;
                    else if (compressTypes.contains("identity", Qt::CaseInsensitive))
                        data = reply->readAll();
                }
            } else data = reply->readAll();

            if (data.isEmpty())
                content = QJsonObject();
            else
                content = toJsonObject(data);
        }
    }
}

///
/// \brief AuthManager::toJsonObject converts a JSON message to a \ref QJsonObject
/// \param jsonString the source JSON message
/// \return the resulting \ref QJsonObject
///
QJsonObject AuthManager::toJsonObject(const QByteArray &jsonString)
{
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    return doc.object();
}


} // namespace engine
} // namespace studio
} // namespace gams
