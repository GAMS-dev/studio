#include "httpmanager.h"
#include "xmlrpc.h"
#include "networkmanager.h"

namespace gams {
namespace studio {
namespace neos {

HttpManager::HttpManager(QObject *parent): QObject(parent)
{
    mManager = NetworkManager::manager();
    connect(mManager, &QNetworkAccessManager::finished, this, &HttpManager::prepareReply);
    connect(mManager, &QNetworkAccessManager::sslErrors, this, &HttpManager::convertSslErrors);
    mRawRequest.setRawHeader("User-Agent", "neos/1.0");
    mRawRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");
}

void HttpManager::setUrl(const QString &url)
{
    mRawRequest.setUrl(url);
}

void HttpManager::setIgnoreSslErrors()
{
    mIgnoreSslErrors = true;
}

bool HttpManager::ignoreSslErrors()
{
    return mIgnoreSslErrors;
}

void HttpManager::submitCall(const QString &method, const QVariantList &params)
{
    QByteArray xml = XmlRpc::prepareCall(method, params);
    QNetworkRequest request(mRawRequest);
    request.setAttribute(QNetworkRequest::User, method);
    mManager->post(request, xml);
}

void HttpManager::prepareReply(QNetworkReply *reply)
{
    QString name;
    if (reply->error()) {
        emit error(reply->errorString(), reply->error());
    } else {
        bool isReply = name.isEmpty();
        QVariant result = XmlRpc::parseParams(reply, name);
        if (isReply) name = reply->request().attribute(QNetworkRequest::User).toString();
        emit received(name, result);
    }
    reply->deleteLater();
}

void HttpManager::convertSslErrors(QNetworkReply *reply, const QList<QSslError> &errorList)
{
    Q_UNUSED(reply)
    QStringList errors;
    for (const QSslError &se : errorList) {
        errors << se.errorString();
    }
    if (mIgnoreSslErrors)
        reply->ignoreSslErrors();
    emit sslErrors(errors);
}

} // namespace neos
} // namespace studio
} // namespace gams
