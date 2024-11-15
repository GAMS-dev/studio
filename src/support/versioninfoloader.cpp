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
#include "versioninfoloader.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commandlineparser.h"

#include <QtConcurrent>
#include <yaml-cpp/yaml.h>

namespace gams {
namespace studio {
namespace support {

const QString VersionInfoLoader::DistribVersionFile = "https://www.gams.com/docs/release/gams-release-history.yaml";
const QString VersionInfoLoader::StudioVersionFile = "https://www.gams.com/docs/release/studio.txt";

VersionInfoLoader::VersionInfoLoader(QObject *parent)
    : QObject(parent)
    , mWebCtrlDistrib(new QNetworkAccessManager(this))
    , mWebCtrlStudio(new QNetworkAccessManager(this))
    , mStudioRegEx("^\\[(\\d+)\\.(\\d+\\.\\d+\\.\\d+),\\d+]$")
{
    mSslConf = QSslConfiguration::defaultConfiguration();
    mSslConf.setBackendConfigurationOption("MinProtocol", "TLSv1.2");

    mWebCtrlDistrib->setStrictTransportSecurityEnabled(true);
    mWebCtrlDistrib->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(mWebCtrlDistrib, &QNetworkAccessManager::finished,
            this, &VersionInfoLoader::distribDownloadFinished);
    connect(mWebCtrlDistrib, &QNetworkAccessManager::sslErrors,
            this, &VersionInfoLoader::sslErrors);
    mWebCtrlStudio->setStrictTransportSecurityEnabled(true);
    mWebCtrlStudio->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(mWebCtrlStudio, &QNetworkAccessManager::finished,
            this, &VersionInfoLoader::studioDownloadFinished);
    connect(mWebCtrlStudio, &QNetworkAccessManager::sslErrors,
            this, &VersionInfoLoader::sslErrors);
    connect(this, &VersionInfoLoader::continueProcessing,
            this, &VersionInfoLoader::requestStudioInfo);
}

VersionInfoLoader::~VersionInfoLoader()
{
    if (mResult.isRunning())
        mResult.waitForFinished();
    delete mWebCtrlDistrib;
    delete mWebCtrlStudio;
}

void VersionInfoLoader::requestRemoteData()
{
    mErrorStrings.clear();
    QNetworkRequest request(DistribVersionFile);
    request.setSslConfiguration(mSslConf);
    mWebCtrlDistrib->get(request);
}

int VersionInfoLoader::remoteDistribVersion() const
{
    return mRemoteDistribVersion;
}

void VersionInfoLoader::setRemoteDistribVersion(int value)
{
    mRemoteDistribVersion = value;
}

QString VersionInfoLoader::remoteDistribVersionString() const
{
    return mRemoteDistribVersionString;
}

void VersionInfoLoader::setRemoteDistribVersionString(const QString &value)
{
    mRemoteDistribVersionString = value;
}

int VersionInfoLoader::remoteStudioVersion() const
{
    return mRemoteStudioVersion;
}

void VersionInfoLoader::setRemoteStudioVersion(int value)
{
    mRemoteStudioVersion = value;
}

QString VersionInfoLoader::remoteStudioVersionString() const
{
    return mRemoteStudioVersionString;
}

void VersionInfoLoader::setRemoteStudioVersionString(const QString &value)
{
    mRemoteStudioVersionString = value;
}

QMap<int, qint64> VersionInfoLoader::distribVersions() const
{
    return mDistribVersions;
}

QString VersionInfoLoader::errorString()
{
    return mErrorStrings.join("<br/>");
}

void VersionInfoLoader::requestStudioInfo()
{
    QNetworkRequest request(StudioVersionFile);
    request.setSslConfiguration(mSslConf);
    mWebCtrlStudio->get(request);
}

void VersionInfoLoader::distribDownloadFinished(QNetworkReply *reply)
{
    auto process = [this, reply]{
        mErrorMessages.clear();
        mDistribVersions.clear();
        if (reply->error() == QNetworkReply::NoError) {
            writeDataToLog("No error. Processing GAMS distrib YAML...");
            auto data = reply->readAll();
            writeDataToLog(data);
            YAML::Node root;
            try {
                root = YAML::Load(data.toStdString());
                YAML::Node last = root[root.size()-1];
                mRemoteDistribVersion = last["distrotxt-id"].as<int>();
                mRemoteDistribVersionString = QString::fromStdString(last["version"].as<std::string>());
                for (size_t i=0; i<root.size(); ++i) {
                    int id;
                    qint64 date;
                    if (root[i]["distrotxt-id"].IsDefined()) {
                        id = root[i]["distrotxt-id"].as<int>();
                    } else {
                        continue;
                    }
                    if (root[i]["distrotxt-id"].IsDefined()) {
                        auto str = QString::fromStdString(root[i]["license-date"].as<std::string>());
                        QDate startDate(1900, 1, 1);
                        date = startDate.daysTo(QDate::fromString(str, Qt::ISODate));
                    } else {
                        continue;
                    }
                    mDistribVersions[id] = date;
                }
            } catch (const YAML::ParserException& e) {
                mErrorMessages << QString("Error while checking for updates : %1 : when loading : %2")
                                      .arg(e.what(), DistribVersionFile);
            } catch (const std::string& e) {
                mErrorMessages << QString("Error while checking for updates : %1 : when loading : %2")
                                    .arg(QString::fromStdString(e), DistribVersionFile);
            }
        } else {
            mErrorStrings << reply->errorString();
            mErrorMessages << "Error while checking the GAMS distrib YAML: " + reply->errorString() + "\n";
        }
        reply->deleteLater();
        if (mErrorMessages.isEmpty()) {
            emit continueProcessing();
        } else {
            writeDataToLog(mErrorMessages.join("\n").toLatin1());
            writeDataToLog("Processing stopped due to previous error.");
        }
    };
    mResult = QtConcurrent::run(process);
}

void VersionInfoLoader::studioDownloadFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        writeDataToLog("No error. Processing GAMS Studio txt...\n");
        auto data = reply->readLine();
        writeDataToLog(data);
        auto match = mStudioRegEx.match(data);
        if (match.hasMatch()) {
            mRemoteStudioVersion = match.captured(1).toInt();
            mRemoteStudioVersionString = match.captured(2);
        }
    } else {
        mErrorStrings << reply->errorString();
        QString error = "Error while checking for GAMS Studio updates: " + reply->errorString();
        mErrorMessages << error;
        writeDataToLog(error.toLatin1());
    }
    reply->deleteLater();
    emit finished();
}

void VersionInfoLoader::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_UNUSED(reply)
    for (const auto& error : errors) {
        mErrorStrings << reply->errorString();
        SysLogLocator::systemLog()->append("Error while checking for updates : " + error.errorString(), LogMsgType::Error);
    }
}

void VersionInfoLoader::writeDataToLog(const QByteArray& data)
{
    if (CommandLineParser::c4uLog().isEmpty())
        return;
    QFile logFile(CommandLineParser::c4uLog());
    if (!logFile.open(QIODevice::Append | QIODevice::Text))
        return;
    logFile.write(data);
    logFile.write("\n");
    logFile.close();
}

}
}
}
