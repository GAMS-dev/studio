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
    connect(mEngineManager, &EngineManager::reListProvider, this, &AuthManager::processProviderList);
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

void AuthManager::processProviderList(const QList<QHash<QString, QVariant> > &allProvider)
{
    if (!mExplicitProvider.isEmpty()) {
        // Check for direct connection
        const QHash<QString, QVariant> &ip = allProvider.first();
        QUrl url = ip.value("endpoint").toString();
        authorize(url, ip.value("clientId").toString());
        return;
    }

    emit reListProvider(allProvider);
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
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reAuthorize(reply);
    });
    if (mTimeoutTimer.interval() > 0) {
        connect(&mTimeoutTimer, &QTimer::timeout, this, [this, reply] {
            emit authorizeTimeout(reply);
        });
        mTimeoutTimer.start();
    }
}

void AuthManager::reAuthorize(QNetworkReply *reply)
{
    QString contentTypeHdr;
    QString contentEncodingHdr;
    if (reply->rawHeaderPairs().count() > 0) {
        for (const QPair<QString, QString> &item : reply->rawHeaderPairs()) {
            if(item.first.compare(QString("Content-Type"), Qt::CaseInsensitive) == 0){
                contentTypeHdr = item.second;
            }
            if(item.first.compare(QString("Content-Encoding"), Qt::CaseInsensitive) == 0){
                contentEncodingHdr = item.second;
            }
        }
    }
    bool isValid;
    int responseCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&isValid);
    if(!isValid) responseCode = -1;

    if (!contentTypeHdr.isEmpty()) {
        auto contentType = contentTypeHdr.split(QString(";"), Qt::SkipEmptyParts);
        if ((contentType.count() > 0) && (contentType.first() == QString("multipart/form-data"))) {
            // TODO : Multipart responses not supported
        } else {
            QByteArray response;
            if(!contentEncodingHdr.isEmpty()) {
                QStringList encoding = contentEncodingHdr.split(QString(";"), Qt::SkipEmptyParts);
                if (encoding.count() > 0) {
                    QStringList compressionTypes = encoding.first().split(',', Qt::SkipEmptyParts);
                    if (compressionTypes.contains("gzip", Qt::CaseInsensitive) || compressionTypes.contains("deflate", Qt::CaseInsensitive)) {
                        // not supported
                        response = "";
                    } else if (compressionTypes.contains("identity", Qt::CaseInsensitive)) {
                        response = reply->readAll();
                    }
                }
            }
            else {
                response = reply->readAll();
            }
            if (responseCode == 200) {
                // Valid answer

                // TODO(JM) extract foreign token from response
                // TODO(JM) POST foreign token to EngineManager to get engineToken
                ;
            }
            else
                // Invalid answer

                // TODO(JM) emit error
                ;
        }
    }

}

} // namespace engine
} // namespace studio
} // namespace gams
