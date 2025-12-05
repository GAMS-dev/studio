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
#include "gamslicenseinfo.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"
#include "exception.h"
#include "common.h"
#include "palmcc.h"
#include "gclgms.h"

#include <QDir>
#include <QClipboard>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QStandardPaths>
#include <yaml-cpp/yaml.h>

namespace gams {
namespace studio {
namespace support {

GamsLicenseInfo::GamsLicenseInfo()
    : mRegExHome(R"(C:\\Users\\\?+)")
{
    auto logger = SysLogLocator::systemLog();
    char msg[GMS_SSSIZE];

    palSetExitIndicator(0); // switch of exit() call
    palSetScreenIndicator(0);
    palSetErrorCallback(GamsLicenseInfo::errorCallback);

    if (!palCreateD(&mPAL,
                    CommonPaths::systemDir().toStdString().c_str(),
                    msg,
                    sizeof(msg))) {
        logger->append(msg, LogMsgType::Error);
        EXCEPT() << "Could not create PAL object. " << msg;
    }

    auto configLice = gamsConfigYamlLicenseLocation();

    int rc; // additional return code, not used here
    auto dataPaths = gamsDataLocations();
    mLicenseFilePath = configLice.isEmpty() ? CommonPaths::gamsLicenseFilePath(dataPaths)
                                            : configLice;
    mLicenseAvailable = palLicenseReadU(mPAL,
                                        mLicenseFilePath.toStdString().c_str(),
                                        msg,
                                        &rc);
    palNetworkLicenseOKSet(mPAL, true);
}

GamsLicenseInfo::~GamsLicenseInfo()
{
    if (mPAL)
        palFree(&mPAL);
}

int GamsLicenseInfo::solvers() const
{
    return mSolverInfo.solvers();
}

int GamsLicenseInfo::solverId(const QString &name) const
{
    return mSolverInfo.solverId(name);
}

QString GamsLicenseInfo::solverName(int id) const
{
    return mSolverInfo.solverName(id);
}

QMap<int, QString> GamsLicenseInfo::solverNames()
{
    return mSolverInfo.solverNames();
}

QMap<int, int> GamsLicenseInfo::solverIndices()
{
    return mSolverInfo.solverIndices();
}

QMap<int, QString> GamsLicenseInfo::modelTypeNames()
{
    return mSolverInfo.modelTypeNames();
}

bool GamsLicenseInfo::solverCapability(int solver, int modelType) const
{
    return mSolverInfo.solverCapability(solver,modelType);
}

QString GamsLicenseInfo::solverLicense(const QString &name, int id) const
{
    int days = -1;
    auto codes = solverCodes(id);
    if (!mLicenseAvailable)
        return "None";
    if (0 == palLicenseLevel(mPAL))
        return "Demo";
    if (5 == palLicenseLevel(mPAL))
        return "Community";
    if (palLicenseCheckSubX(mPAL,
                            name.toStdString().c_str(),
                            codes.toStdString().c_str(),
                            &days)) {
        if (palLicenseIsAcademic(mPAL))
            return "Community";
        return "Demo";
    }
    if (days == 0)
        return "Full";
    if (days > 0)
        return "Evaluation";
    return "Expired";
}

QStringList GamsLicenseInfo::licenseFromClipboard()
{
    QString data = QGuiApplication::clipboard()->text();
    return processLicenseData(data);
}

QStringList GamsLicenseInfo::licenseFromFile(const QString &fileName)
{
    QString data;
    QFile file(fileName);
    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        data = file.readAll();
    }
    return processLicenseData(data);
}

bool GamsLicenseInfo::isLicenseValid() const
{
    return mLicenseAvailable;
}

bool GamsLicenseInfo::isLicenseValid(const QStringList &license)
{
    int i = 1;
    for (const auto& line: license) {
        palLicenseRegisterGAMS(mPAL, i++, line.trimmed().toStdString().c_str());
    }
    palLicenseRegisterGAMSDone(mPAL);
    bool ret = !palLicenseValidation(mPAL);
    addPalMessagesToSysLog();
    return ret;
}

QStringList GamsLicenseInfo::gamsDataLocations()
{
    char buffer[2048];
    const int nOffset = 8;
    int offset[nOffset];
    int numdirs;
    QStringList dataPaths;
    auto systemDir = QDir::toNativeSeparators(CommonPaths::systemDir().toStdString().c_str());
    if (!palDataDirs(mPAL, buffer, 2048, &numdirs, offset, nOffset,
                    systemDir.toStdString().c_str()))
        return dataPaths;

#ifdef _WIN64
    auto dir = QDir::toNativeSeparators(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    auto currentPath = QDir::toNativeSeparators(QDir::currentPath());
    for (int i=0; i<nOffset && i<numdirs; i++) {
        QString path(buffer+offset[i]);
        path = QDir::toNativeSeparators(path);
        if (path.contains(mRegExHome))
            continue; // skip path with special characters
        if (path == currentPath) {
            continue;
        } else if (path.startsWith(currentPath)) {
            path = path.replace(0, currentPath.length(), CommonPaths::systemDir());
            dataPaths << QDir::toNativeSeparators(path);
            continue;
        }
        dataPaths << path;
    }
#else
    for (int i=0; i<nOffset && i<numdirs; i++) {
        dataPaths << buffer+offset[i];
    }
#endif
    return dataPaths;
}

QStringList GamsLicenseInfo::gamsConfigLocations()
{
    char buffer[2048];
    const int nOffset = 8;
    int offset[nOffset];
    int numdirs;
    QStringList configPaths;
    auto systemDir = QDir::toNativeSeparators(CommonPaths::systemDir().toStdString().c_str());
    if (!palConfigDirs(mPAL, buffer, 2048, &numdirs, offset, nOffset,
                       systemDir.toStdString().c_str()))
        return configPaths;

    for (int i=0; i<nOffset && i<numdirs; i++) {
        configPaths << buffer+offset[i];
    }
    return configPaths;
}

int GamsLicenseInfo::localDistribVersion()
{
    char buffer[GMS_SSSIZE];
    palGetGold(mPAL, buffer);
    return 10*palGetVer(mPAL)+atoi(buffer);
}

QString GamsLicenseInfo::localDistribVersionString()
{
    char relbuf[GMS_SSSIZE];
    palGetRel(mPAL, relbuf);
    char goldbuf[GMS_SSSIZE];
    palGetGold(mPAL, goldbuf);
    QString postfix;
    if (palIsAlfa(mPAL))
        postfix = " Alpha";
    else if (palIsBeta(mPAL))
        postfix = " Beta";
    return QString("%1.%2%3").arg(relbuf, goldbuf, postfix);
}

QString GamsLicenseInfo::localDistribVersionStringShort()
{
    char relbuf[GMS_SSSIZE];
    palGetRel(mPAL, relbuf);
    char goldbuf[GMS_SSSIZE];
    palGetGold(mPAL, goldbuf);
    return QString("%1.%2").arg(relbuf, goldbuf);
}

QString GamsLicenseInfo::licenseFilePath() const
{
    return mLicenseFilePath;
}

bool GamsLicenseInfo::isAlpha() const
{
    return palIsAlpha(mPAL);
}

bool GamsLicenseInfo::isBeta() const
{
    return palIsBeta(mPAL);
}

bool GamsLicenseInfo::isCurrentEvaluation(int evalDate)
{
    return evalDate >= palGetJul(mPAL);
}

bool GamsLicenseInfo::isCurrentMaintenance(int mainDate)
{
    return mainDate >= palGetJul(mPAL);
}

bool GamsLicenseInfo::isLicenseValidationSuccessful() const
{
    return !palLicenseValidation(mPAL);
}

bool GamsLicenseInfo::isGamsLicense(const QStringList &license)
{
    int i = 1;
    for (const auto& line: license) {
        palLicenseRegisterGAMS(mPAL, i++, line.trimmed().toStdString().c_str());
    }
    palLicenseRegisterGAMSDone(mPAL);
    int result = !palLicenseCheckSubSys(mPAL, "07") || !palLicenseCheckSubSys(mPAL, "08");
    return !result;
}

bool GamsLicenseInfo::isGenericLicense() const
{
    char platform[GMS_SSSIZE];
    palLicenseGetPlatform(mPAL, platform);
    return !QString(platform).compare("GEN", Qt::CaseInsensitive);
}

int GamsLicenseInfo::evaluationLicenseData()
{
    return palLicenseGetEvalDate(mPAL);
}

int GamsLicenseInfo::licenseData()
{
    return palLicenseGetMaintDate(mPAL);
}

int GamsLicenseInfo::julian()
{
    return palGetJul(mPAL);
}

int GamsLicenseInfo::today()
{
    return palGetToday(mPAL);
}

QString GamsLicenseInfo::solverCodes(int solverId) const
{
    return mSolverInfo.solverCodes(solverId);
}

int GamsLicenseInfo::errorCallback(int count, const char *message)
{
    Q_UNUSED(count)
    auto logger = SysLogLocator::systemLog();
    logger->append(InvalidGAMS, LogMsgType::Error);
    logger->append(message, LogMsgType::Error);
    return 0;
}

void GamsLicenseInfo::addPalMessagesToSysLog()
{
    char msg[GMS_SSSIZE];
    auto logger = SysLogLocator::systemLog();
    while (palLicenseGetMessage(mPAL, msg, GMS_SSSIZE)) {
        QString message(msg);
        logger->append(message.replace('*', "").trimmed(), LogMsgType::Error);
    }
}

QStringList GamsLicenseInfo::processLicenseData(const QString &data)
{
    QStringList licenseLines;
    for (auto& entry : data.split('\n')) {
        auto line = entry.trimmed();
        if (line.isEmpty())
            continue;
        if (line.size() != 65)
            return QStringList();
        licenseLines << line;
    }
    // a GAMS license has 8 lines
    return (licenseLines.size() == 8) ? licenseLines : QStringList();
}

QString GamsLicenseInfo::gamsConfigYamlLicenseLocation()
{
    QString configData;
    QFile configFile(CommonPaths::defaultGamsUserConfigFile());
    if (configFile.open(QIODevice::ReadOnly)) {
        configData = configFile.readAll();
        configFile.close();
    }
    YAML::Node root;
    try {
        root = YAML::Load(configData.toStdString());
        auto cmdNode = root["commandLineParameters"];
        if (cmdNode.IsNull()) {
            return QString();
        }
        for (std::size_t i=0; i<cmdNode.size(); ++i) {
            for (const auto& node : cmdNode[i]) {
                auto name = QString::fromStdString(node.first.as<std::string>());
                if (name.compare("license", Qt::CaseInsensitive)) {
                    continue;
                }
                for (const auto& value : node.second) {
                    if (value.first.as<std::string>() == "value")
                        return QString::fromStdString(value.second.as<std::string>());
                }
            }
        }
    } catch (const YAML::ParserException& e) {
        auto error = QString("Error while fetching the license file location : %1 : when loading : %2")
                         .arg(e.what(), configFile.fileName());
        SysLogLocator::systemLog()->append(error, LogMsgType::Error);
    } catch (const std::string& e) {
        auto error = QString("Error while fetching the license file location : %1 : when loading : %2")
                         .arg(QString::fromStdString(e), configFile.fileName());
        SysLogLocator::systemLog()->append(error, LogMsgType::Error);
    }
    return QString();
}

QString GamsLicenseInfo::licenseDirectory()
{
    auto yamlPath = gamsConfigYamlLicenseLocation();
    if (!yamlPath.isEmpty()) {
        QFileInfo fi(yamlPath);
        return fi.dir().absolutePath();
    }
    GamsLicenseInfo licenseInfo;
    auto locations = licenseInfo.gamsDataLocations();
    auto dir = locations.count() ? locations.constFirst() : CommonPaths::systemDir();
    return dir;
}

QString GamsLicenseInfo::licenseLocation()
{
    auto liceFile = gamsConfigYamlLicenseLocation();
    if (liceFile.isEmpty()) {
        GamsLicenseInfo licenseInfo;
        auto locations = licenseInfo.gamsDataLocations();
        liceFile = locations.count() ? licenseInfo.gamsDataLocations().constFirst() + "/" + CommonPaths::licenseFile()
                                     : CommonPaths::systemDir() + "/" + CommonPaths::licenseFile();
    }
    return liceFile;
}

}
}
}
