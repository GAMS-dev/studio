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
#include "httpmanager.h"
#include "xmlrpc.h"

namespace gams {
namespace studio {
namespace neos {

HttpManager::HttpManager(QObject *parent): QObject(parent)
{
    connect(&mManager, &QNetworkAccessManager::finished, this, &HttpManager::prepareReply);
    connect(&mManager, &QNetworkAccessManager::sslErrors, this, &HttpManager::convertSslErrors);
    mRawRequest.setRawHeader("User-Agent", "neos/1.0");
    mRawRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");
    QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
    conf.setBackendConfigurationOption("MinProtocol", "TLSv1.2");
    mRawRequest.setSslConfiguration(conf);
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
    mManager.post(request, xml);
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
