/**
 * GAMS Engine
 * GAMS Engine let's you register, solve and get results of GAMS Models. It has namespace management system so you can restrict your users to certain set of models.
 *
 * The version of the OpenAPI document: latest
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#include "OAIAuthApi.h"
#include "OAIHelpers.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace OpenAPI {

OAIAuthApi::OAIAuthApi(const QString &scheme, const QString &host, int port, const QString &basePath, const int timeOut)
    : _scheme(scheme),
      _host(host),
      _port(port),
      _basePath(basePath),
      _timeOut(timeOut),
      _manager(nullptr),
      isResponseCompressionEnabled(false),
      isRequestCompressionEnabled(false) {}

OAIAuthApi::~OAIAuthApi() {
}

void OAIAuthApi::setScheme(const QString &scheme) {
    _scheme = scheme;
}

void OAIAuthApi::setHost(const QString &host) {
    _host = host;
}

void OAIAuthApi::setPort(int port) {
    _port = port;
}

void OAIAuthApi::setBasePath(const QString &basePath) {
    _basePath = basePath;
}

void OAIAuthApi::setTimeOut(const int timeOut) {
    _timeOut = timeOut;
}

void OAIAuthApi::setWorkingDirectory(const QString &path) {
    _workingDirectory = path;
}

void OAIAuthApi::setNetworkAccessManager(QNetworkAccessManager* manager) {
    _manager = manager;  
}

void OAIAuthApi::addHeaders(const QString &key, const QString &value) {
    defaultHeaders.insert(key, value);
}

void OAIAuthApi::enableRequestCompression() {
    isRequestCompressionEnabled = true;
}

void OAIAuthApi::enableResponseCompression() {
    isResponseCompressionEnabled = true;
}

void OAIAuthApi::abortRequests(){
    emit abortRequestsSignal();
}

void OAIAuthApi::createJWTToken() {
    QString fullPath = QString("%1://%2%3%4%5")
                           .arg(_scheme)
                           .arg(_host)
                           .arg(_port ? ":" + QString::number(_port) : "")
                           .arg(_basePath)
                           .arg("/auth/");

    OAIHttpRequestWorker *worker = new OAIHttpRequestWorker(this, _manager);
    worker->setTimeOut(_timeOut);
    worker->setWorkingDirectory(_workingDirectory);
    OAIHttpRequestInput input(fullPath, "POST");

    foreach (QString key, this->defaultHeaders.keys()) { input.headers.insert(key, this->defaultHeaders.value(key)); }

    connect(worker, &OAIHttpRequestWorker::on_execution_finished, this, &OAIAuthApi::createJWTTokenCallback);
    connect(this, &OAIAuthApi::abortRequestsSignal, worker, &QObject::deleteLater); 
    worker->execute(&input);
}

void OAIAuthApi::createJWTTokenCallback(OAIHttpRequestWorker *worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    } else {
        msg = "Error: " + worker->error_str;
        error_str = QString("%1, %2").arg(worker->error_str).arg(QString(worker->response));
    }
    OAIModel_auth_token output(QString(worker->response));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit createJWTTokenSignal(output);
        emit createJWTTokenSignalFull(worker, output);
    } else {
        emit createJWTTokenSignalE(output, error_type, error_str);
        emit createJWTTokenSignalEFull(worker, error_type, error_str);
    }
}

void OAIAuthApi::postLoginInterface(const QString &username, const QString &password) {
    QString fullPath = QString("%1://%2%3%4%5")
                           .arg(_scheme)
                           .arg(_host)
                           .arg(_port ? ":" + QString::number(_port) : "")
                           .arg(_basePath)
                           .arg("/auth/login");

    if (fullPath.indexOf("?") > 0)
        fullPath.append("&");
    else
        fullPath.append("?");
    fullPath.append(QUrl::toPercentEncoding("username")).append("=").append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(username)));

    if (fullPath.indexOf("?") > 0)
        fullPath.append("&");
    else
        fullPath.append("?");
    fullPath.append(QUrl::toPercentEncoding("password")).append("=").append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(password)));

    OAIHttpRequestWorker *worker = new OAIHttpRequestWorker(this, _manager);
    worker->setTimeOut(_timeOut);
    worker->setWorkingDirectory(_workingDirectory);
    OAIHttpRequestInput input(fullPath, "POST");

    foreach (QString key, this->defaultHeaders.keys()) { input.headers.insert(key, this->defaultHeaders.value(key)); }

    connect(worker, &OAIHttpRequestWorker::on_execution_finished, this, &OAIAuthApi::postLoginInterfaceCallback);
    connect(this, &OAIAuthApi::abortRequestsSignal, worker, &QObject::deleteLater); 
    worker->execute(&input);
}

void OAIAuthApi::postLoginInterfaceCallback(OAIHttpRequestWorker *worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    } else {
        msg = "Error: " + worker->error_str;
        error_str = QString("%1, %2").arg(worker->error_str).arg(QString(worker->response));
    }
    OAIModel_auth_token output(QString(worker->response));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit postLoginInterfaceSignal(output);
        emit postLoginInterfaceSignalFull(worker, output);
    } else {
        emit postLoginInterfaceSignalE(output, error_type, error_str);
        emit postLoginInterfaceSignalEFull(worker, error_type, error_str);
    }
}

} // namespace OpenAPI
