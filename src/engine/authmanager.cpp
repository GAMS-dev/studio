#include "authmanager.h"
#include "enginemanager.h"
#include "exception.h"

#include <QNetworkAccessManager>
#include <networkmanager.h>

namespace gams {
namespace studio {
namespace engine {

AuthManager::AuthManager(EngineManager *engineManager, QObject *parent): QObject(parent), mEngineManager(engineManager)
{
    if (!mEngineManager) EXCEPT() << "AuthManager: the EngineManager must not be null";
    mNetworkManager = NetworkManager::manager();
    if (!mNetworkManager) EXCEPT() << "AuthManager: the NetworkManager must not be null";
    mTimeoutTimer.setSingleShot(true);
    mTimeoutTimer.setInterval(10000);
    connect(mEngineManager, &EngineManager::reListProvider, this, &AuthManager::reListProvider);
}

AuthManager::~AuthManager()
{
//    disconnect(&mTimeoutTimer, &QTimer::timeout);
    mTimeoutTimer.stop();
}

void AuthManager::listProvider(const QString &name)
{
    mExplicitProvider = name;
    mEngineManager->listProvider(name);
}

void AuthManager::authorize(QUrl authUrl, const QString &clientId)
{
    QByteArray content = "client_id=" + clientId.toUtf8();

    QNetworkRequest request = QNetworkRequest(authUrl);
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setRawHeader("User-Agent", "GamsStudio/1.0.0/cpp-qt");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept-Encoding", "identity");

    QNetworkReply *reply = mNetworkManager->post(request, content);
    reply->setParent(this);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        emit authorizeSignal(reply);
    });
    if (mTimeoutTimer.interval() > 0) {
        connect(&mTimeoutTimer, &QTimer::timeout, this, [this, reply] {
            emit authorizeTimeout(reply);
        });
        mTimeoutTimer.start();
    }
}

void AuthManager::reListProvider(const QList<QHash<QString, QVariant> > &allProvider)
{
    if (allProvider.isEmpty()) return;

    if (!mExplicitProvider.isEmpty()) {
        // Check for direct connection
        const QHash<QString, QVariant> &ip = allProvider.first();
        QUrl url = ip.value("endpoint").toString();
        authorize(url, ip.value("clientId").toString());
    }

    emit reProviderList(allProvider);
}

} // namespace engine
} // namespace studio
} // namespace gams
