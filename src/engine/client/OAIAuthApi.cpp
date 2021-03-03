/**
 * GAMS Engine
 * With GAMS Engine you can register and solve GAMS models. It has a namespace management system, so you can restrict your users to certain models.
 *
 * The version of the OpenAPI document: latest
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#include "OAIAuthApi.h"
#include "OAIHelpers.h"
#include "OAIServerConfiguration.h"
#include <QJsonArray>
#include <QJsonDocument>

namespace OpenAPI {

OAIAuthApi::OAIAuthApi(const int timeOut)
    : _timeOut(timeOut),
      _manager(nullptr),
      isResponseCompressionEnabled(false),
      isRequestCompressionEnabled(false) {
      initializeServerConfigs();
      }

OAIAuthApi::~OAIAuthApi() {
}

void OAIAuthApi::initializeServerConfigs(){

//Default server
QList<OAIServerConfiguration> defaultConf = QList<OAIServerConfiguration>();
//varying endpoint server 
QList<OAIServerConfiguration> serverConf = QList<OAIServerConfiguration>();
defaultConf.append(OAIServerConfiguration(
    QUrl("//localhost/"),
    "No description provided",
    QMap<QString, OAIServerVariable>()));
_serverConfigs.insert("createJWTToken",defaultConf);
_serverIndices.insert("createJWTToken",0);

_serverConfigs.insert("postW",defaultConf);
_serverIndices.insert("postW",0);


}

/**
* returns 0 on success and -1, -2 or -3 on failure.
* -1 when the variable does not exist and -2 if the value is not defined in the enum and -3 if the operation or server index is not found 
*/
int OAIAuthApi::setDefaultServerValue(int serverIndex, const QString &operation, const QString &variable, const QString &value){
    auto it = _serverConfigs.find(operation);
    if(it != _serverConfigs.end() && serverIndex < it.value().size() ){
      return _serverConfigs[operation][serverIndex].setDefaultValue(variable,value);
    }
    return -3;
}
void OAIAuthApi::setServerIndex(const QString &operation, int serverIndex){
    if(_serverIndices.contains(operation) && serverIndex < _serverConfigs.find(operation).value().size() )
        _serverIndices[operation] = serverIndex;
}

void OAIAuthApi::setApiKey(const QString &apiKeyName, const QString &apiKey){
    _apiKeys.insert(apiKeyName,apiKey);
}

void OAIAuthApi::setBearerToken(const QString &token){
    _bearerToken = token;
}

void OAIAuthApi::setUsername(const QString &username) {
    _username = username;
}

void OAIAuthApi::setPassword(const QString &password) {
    _password = password;
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

    /**
     * Appends a new ServerConfiguration to the config map for a specific operation.
     * @param operation The id to the target operation.
     * @param url A string that contains the URL of the server
     * @param description A String that describes the server
     * @param variables A map between a variable name and its value. The value is used for substitution in the server's URL template.
     * returns the index of the new server config on success and -1 if the operation is not found
     */
int OAIAuthApi::addServerConfiguration(const QString &operation, const QUrl &url, const QString &description, const QMap<QString, OAIServerVariable> &variables){
    if(_serverConfigs.contains(operation)){
        _serverConfigs[operation].append(OAIServerConfiguration(
                    url,
                    description,
                    variables));
        return _serverConfigs[operation].size()-1;
    }else{
        return -1;
    }
}

    /**
     * Appends a new ServerConfiguration to the config map for a all operations and sets the index to that server.
     * @param url A string that contains the URL of the server
     * @param description A String that describes the server
     * @param variables A map between a variable name and its value. The value is used for substitution in the server's URL template.
     */
void OAIAuthApi::setNewServerForAllOperations(const QUrl &url, const QString &description, const QMap<QString, OAIServerVariable> &variables){
        for(auto e : _serverIndices.keys()){
            setServerIndex(e, addServerConfiguration(e, url, description, variables));
        }
} 
    /**
     * Appends a new ServerConfiguration to the config map for an operations and sets the index to that server.
     * @param URL A string that contains the URL of the server
     * @param description A String that describes the server
     * @param variables A map between a variable name and its value. The value is used for substitution in the server's URL template.
     */
void OAIAuthApi::setNewServer(const QString &operation, const QUrl &url, const QString &description, const QMap<QString, OAIServerVariable> &variables){

    setServerIndex(operation, addServerConfiguration(operation, url, description, variables));

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

QString OAIAuthApi::getParamStylePrefix(QString style){

        if(style == "matrix"){ 
            return ";";
        }else if(style == "label"){
            return ".";
        }else if(style == "form"){
            return "&"; 
        }else if(style == "simple"){
            return "";
        }else if(style == "spaceDelimited"){
            return "&"; 
        }else if(style == "pipeDelimited"){
            return "&"; 
        }else
            return "none";
}

QString OAIAuthApi::getParamStyleSuffix(QString style){

        if(style == "matrix"){ 
            return "=";
        }else if(style == "label"){
            return "";
        }else if(style == "form"){
            return "=";
        }else if(style == "simple"){
            return "";
        }else if(style == "spaceDelimited"){
            return "=";
        }else if(style == "pipeDelimited"){
            return "=";
        }else
            return "none";
}

QString OAIAuthApi::getParamStyleDelimiter(QString style, QString name, bool isExplode){

        if(style == "matrix"){ 
            return (isExplode) ? ";" + name + "=" : ",";

        }else if(style == "label"){
            return (isExplode) ? "." : ",";

        }else if(style == "form"){
            return (isExplode) ? "&" + name + "=" : ","; 

        }else if(style == "simple"){
            return ",";
        }else if(style == "spaceDelimited"){
            return (isExplode) ? "&" + name + "=" : " ";

        }else if(style == "pipeDelimited"){
            return (isExplode) ? "&" + name + "=" : "|";

        }else if(style == "deepObject"){
            return (isExplode) ? "&" : "none";

        }else
            return "none";
}

void OAIAuthApi::createJWTToken(const QVariant &expires_in) {
    QString fullPath = QString(_serverConfigs["createJWTToken"][_serverIndices.value("createJWTToken")].URL()+"/auth/");
    
    if(!_username.isEmpty() && !_password.isEmpty()){
        QByteArray b64;
        b64.append(_username.toUtf8() + ":" + _password.toUtf8());
        addHeaders("Authorization","Basic " + b64.toBase64());
    }

    QString queryPrefix, querySuffix, queryDelimiter, queryStyle;
    if(!expires_in.isNull())
    {
        queryStyle = "";
        if(queryStyle == "") 
            queryStyle = "form";
        queryPrefix = getParamStylePrefix(queryStyle);
        querySuffix = getParamStyleSuffix(queryStyle);
        queryDelimiter = getParamStyleDelimiter(queryStyle, "expires_in", false); 
        if (fullPath.indexOf("?") > 0)
            fullPath.append(queryPrefix);
        else
            fullPath.append("?");

        fullPath.append(QUrl::toPercentEncoding("expires_in")).append(querySuffix).append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(expires_in.value<qint32>())));
    }

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

void OAIAuthApi::postW(const QString &username, const QString &password, const QVariant &expires_in) {
    QString fullPath = QString(_serverConfigs["postW"][_serverIndices.value("postW")].URL()+"/auth/login");
    

    QString queryPrefix, querySuffix, queryDelimiter, queryStyle;
    
    {
        queryStyle = "";
        if(queryStyle == "") 
            queryStyle = "form";
        queryPrefix = getParamStylePrefix(queryStyle);
        querySuffix = getParamStyleSuffix(queryStyle);
        queryDelimiter = getParamStyleDelimiter(queryStyle, "username", false); 
        if (fullPath.indexOf("?") > 0)
            fullPath.append(queryPrefix);
        else
            fullPath.append("?");

        fullPath.append(QUrl::toPercentEncoding("username")).append(querySuffix).append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(username)));
    }

    
    {
        queryStyle = "";
        if(queryStyle == "") 
            queryStyle = "form";
        queryPrefix = getParamStylePrefix(queryStyle);
        querySuffix = getParamStyleSuffix(queryStyle);
        queryDelimiter = getParamStyleDelimiter(queryStyle, "password", false); 
        if (fullPath.indexOf("?") > 0)
            fullPath.append(queryPrefix);
        else
            fullPath.append("?");

        fullPath.append(QUrl::toPercentEncoding("password")).append(querySuffix).append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(password)));
    }

    if(!expires_in.isNull())
    {
        queryStyle = "";
        if(queryStyle == "") 
            queryStyle = "form";
        queryPrefix = getParamStylePrefix(queryStyle);
        querySuffix = getParamStyleSuffix(queryStyle);
        queryDelimiter = getParamStyleDelimiter(queryStyle, "expires_in", false); 
        if (fullPath.indexOf("?") > 0)
            fullPath.append(queryPrefix);
        else
            fullPath.append("?");

        fullPath.append(QUrl::toPercentEncoding("expires_in")).append(querySuffix).append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(expires_in.value<qint32>())));
    }

    OAIHttpRequestWorker *worker = new OAIHttpRequestWorker(this, _manager);
    worker->setTimeOut(_timeOut);
    worker->setWorkingDirectory(_workingDirectory);
    OAIHttpRequestInput input(fullPath, "POST");

    foreach (QString key, this->defaultHeaders.keys()) { input.headers.insert(key, this->defaultHeaders.value(key)); }

    connect(worker, &OAIHttpRequestWorker::on_execution_finished, this, &OAIAuthApi::postWCallback);
    connect(this, &OAIAuthApi::abortRequestsSignal, worker, &QObject::deleteLater); 
    worker->execute(&input);
}

void OAIAuthApi::postWCallback(OAIHttpRequestWorker *worker) {
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
        emit postWSignal(output);
        emit postWSignalFull(worker, output);
    } else {
        emit postWSignalE(output, error_type, error_str);
        emit postWSignalEFull(worker, error_type, error_str);
    }
}

} // namespace OpenAPI
