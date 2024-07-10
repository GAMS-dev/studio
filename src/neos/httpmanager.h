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
#ifndef GAMS_STUDIO_NEOS_HTTPMANAGER_H
#define GAMS_STUDIO_NEOS_HTTPMANAGER_H

#include <QtNetwork>

namespace gams {
namespace studio {
namespace neos {

class HttpManager: public QObject
{
    Q_OBJECT
public:
    HttpManager(QObject *parent = nullptr);
    void setUrl(const QString &url);
    void setIgnoreSslErrors();
    bool ignoreSslErrors();

signals:
    void received(QString name, QVariant data);
    void error(QString errorString, QNetworkReply::NetworkError error);
    void sslErrors(const QStringList &errorList);

public slots:
    void submitCall(const QString &method, const QVariantList &params = QVariantList());

private slots:
    void prepareReply(QNetworkReply *reply);
    void convertSslErrors(QNetworkReply *reply, const QList<QSslError> &errorList);

private:
    QNetworkRequest mRawRequest;
    QNetworkAccessManager mManager;
    bool mIgnoreSslErrors = false;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_HTTPMANAGER_H
