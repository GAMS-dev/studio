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
#include "checkforupdate.h"
#include "commonpaths.h"
#include "gamslicenseinfo.h"
#include "versioninfoloader.h"

#include <QDir>

namespace gams {
namespace studio {
namespace support {

CheckForUpdate::CheckForUpdate(bool downloadVersions, QObject* parent)
    : QObject(parent)
    , mLicenseInfo(new GamsLicenseInfo)
    , mVersionInfoLoader(new VersionInfoLoader(this))
    , mDistribRegEx("^\\[(\\d+),\\d+]$")
    , mStudioRegEx("^\\[(\\d+)\\.(\\d+\\.\\d+\\.\\d+),\\d+]$")
{
    connect(mVersionInfoLoader.get(), &VersionInfoLoader::finished,
            this, &CheckForUpdate::updateVersionInformation);
    connect(mVersionInfoLoader.get(), &VersionInfoLoader::newErrorMessage,
            this, &CheckForUpdate::newErrorMessage);
    connect(mVersionInfoLoader.get(), &VersionInfoLoader::newInfoMessage,
            this, &CheckForUpdate::newInfoMessage);
    checkForUpdate(downloadVersions);
}

CheckForUpdate::~CheckForUpdate()
{

}

QString CheckForUpdate::versionInformation() const
{
    return mDistribVersionInfo + mStudioVersionInfo;
}

QString CheckForUpdate::versionInformationShort() const
{
    if (mLocalDistribVersion < mVersionInfoLoader->remoteDistribVersion())
        return QString("<html><body>There is a new version of GAMS (%1) available for download at <a href=\"https://www.gams.com/download/\">www.gams.com/download/</a>.</body></html>").arg(mVersionInfoLoader->remoteDistribVersionString());
    return QString();
}

int CheckForUpdate::localDistribVersion() const
{
    return mLocalDistribVersion;
}

QString CheckForUpdate::localDistribVersionShort() const
{
    auto version = mLocalDistribVersionString;
    auto index = version.lastIndexOf('.');
    return version.remove(index, version.size());
}

int CheckForUpdate::remoteDistribVersion() const
{
    return mVersionInfoLoader->remoteDistribVersion();
}

bool CheckForUpdate::isLocalDistribLatest()
{
    return localDistribVersion() == remoteDistribVersion();
}

int CheckForUpdate::localStudioVersion() const
{
    return mLocalStudioVersion;
}

void CheckForUpdate::checkForUpdate(bool downloadVersions)
{
    mLicenseInfo.reset(new GamsLicenseInfo);
    mToday = mLicenseInfo->today();
    mMainDate = mLicenseInfo->licenseData();
    mEvalDate = mLicenseInfo->evaluationLicenseData();
    mValidLicense = mLicenseInfo->isLicenseValid();
    mLocalDistribVersion = mLicenseInfo->localDistribVersion();
    mLocalDistribVersionString = mLicenseInfo->localDistribVersionString();
    mVersionInfoLoader->setRemoteDistribVersion(mLocalDistribVersion);
    mVersionInfoLoader->setRemoteDistribVersionString(mLocalDistribVersionString);
    mLocalStudioVersion = QString(STUDIO_VERSION_STRING).replace('.', "").toInt();
    mVersionInfoLoader->setRemoteStudioVersionString(STUDIO_VERSION_STRING);
    mVersionInfoLoader->setRemoteStudioVersion(mLocalStudioVersion);
    mDistribVersionInfo = "";
    mStudioVersionInfo = "";
    if (downloadVersions) {
        mVersionInfoLoader->requestRemoteData();
    }
}

void CheckForUpdate::updateVersionInformation()
{
    if (!mVersionInfoLoader->errorString().isEmpty()) {
        mStudioVersionInfo = QString();
        mDistribVersionInfo = "<b>An error occured while checking for updates:</b><br/>";
        mDistribVersionInfo += mVersionInfoLoader->errorString();
        emit versionInformationAvailable();
        return;
    }
    getDistribLicenseInfo();
    mStudioVersionInfo = "<h1>GAMS Studio</h1>";
    mStudioVersionInfo += "<ul>";
    if (mLocalStudioVersion >= mVersionInfoLoader->remoteStudioVersion()) {
        mStudioVersionInfo += "<li>You are using the latest GAMS Studio version</li>";
    } else {
        mStudioVersionInfo += addLiTag(QString("The version of your GAMS Studio is %1")
                                           .arg(STUDIO_VERSION_STRING));
        mStudioVersionInfo += addLiTag("There is an update available");
        mStudioVersionInfo += addLiTag(QString("To download the most recent version of GAMS Studio (%1),<br/>please visit, <a href=\"https://github.com/GAMS-dev/studio/releases\">https://github.com/GAMS-dev/studio/releases</a>")
                                           .arg(mVersionInfoLoader->remoteStudioVersionString()));
    }
    mStudioVersionInfo += "</ul>";
    emit versionInformationAvailable();
}

void CheckForUpdate::getDistribLicenseInfo()
{
    mDistribVersionInfo = "<h1>GAMS Distribution</h1>";
    mDistribVersionInfo += "<ul>";
    mDistribVersionInfo += addLiTag(QString("Your GAMS system directory is <a href=\"%1\">%1</a>")
                                        .arg(CommonPaths::systemDir()));
    if (QFileInfo::exists(mLicenseInfo->licesneFilePath())) {
        mDistribVersionInfo += addLiTag(QString("Your license file is at <a href=\"%1\">%1</a>")
                                            .arg(mLicenseInfo->licesneFilePath()));
    }
    if (!mValidLicense) {
        mDistribVersionInfo += addLiTag("No valid license file found, checking for updates anyway:");
        if (mLocalDistribVersion >= mVersionInfoLoader->remoteDistribVersion()) {
            mDistribVersionInfo += addLiTag("You are using the latest GAMS version");
        } else {
            setDistribVersionDetails();
        }
        mDistribVersionInfo += "</ul>";
        return;
    }
    int evalLeft = 0;
    auto evalDate = mEvalDate;
    auto mainDate = mMainDate;
    auto maxint = std::numeric_limits<int>::max();
    if ((evalDate == maxint && mainDate == maxint) || (evalDate < maxint && mainDate < maxint)) {
        mDistribVersionInfo += addLiTag("Invalid license file found, please contatct <a href=\"mailto:sales@gams.com\">sales@gams.com</a>");
    } else {
        setGenericLicenseText();
        if (mainDate == maxint) { // eval license
            evalDate += GraceDays;
            evalLeft = evalDate - mToday;
            if (!mLicenseInfo->isCurrentEvaluation(evalDate)) {
                if (evalLeft == -1) {
                    mDistribVersionInfo += addLiTag("This evaluation license expired yesterday");
                } else {
                    mDistribVersionInfo += addLiTag(QString("<li>This evaluation license expired %1 days ago").arg(-1*evalLeft));
                    mDistribVersionInfo += addLiTag("Please contatct <a href=\"mailto:sales@gams.com\">sales@gams.com</a>");
                }
            } else {
                if (evalLeft == 0) {
                    mDistribVersionInfo += addLiTag("This evaluation license expires today");
                } else if (evalLeft == 1) {
                    mDistribVersionInfo += addLiTag("This evaluation license expires tomorrow");
                } else {
                    mDistribVersionInfo += addLiTag(QString("This evaluation license expires in %1 days").arg(evalLeft));
                }
                if (mLocalDistribVersion >= mVersionInfoLoader->remoteDistribVersion()) {
                    mDistribVersionInfo += addLiTag("You are using the latest GAMS version");
                } else {
                    setDistribVersionDetails();
                }
            }
        } else { // maintenance license
            mainDate += GraceDays;
            int lastReleaseLicense = 0;
            QString lastReleaseLicenseString;
            auto keys = mVersionInfoLoader->distribVersions().keys();
            for (auto iter = keys.rbegin(); iter!=keys.rend(); ++iter) {
                if (mainDate < mVersionInfoLoader->distribVersions().value(*iter)) {
                    continue;
                } else {
                    lastReleaseLicense = *iter;
                    auto str = QString::number(*iter);
                    str = str.insert(str.size()-2, '.');
                    str = str.insert(str.size()-1, '.');
                    lastReleaseLicenseString = str;
                    break;
                }
            }
            if (!mLicenseInfo->isCurrentMaintenance(mainDate)) {
                if ((mLicenseInfo->julian() - mainDate) == 1) {
                    if (mLicenseInfo->isAlpha() || mLicenseInfo->isBeta()) {
                        mDistribVersionInfo += addLiTag("Your license is 1 day too old to run with the GOLD release of this system");
                    } else {
                        mDistribVersionInfo += addLiTag("Your license is 1 day too old to run with this system");
                    }
                } else {
                    if (mLicenseInfo->isAlpha() || mLicenseInfo->isBeta()) {
                        mDistribVersionInfo += addLiTag(QString("Your license is %1 days too old to run with the GOLD release of this system")
                                                            .arg(mLicenseInfo->julian()-mainDate));
                    } else {
                        mDistribVersionInfo += addLiTag(QString("Your license is %1 days too old to run with this system")
                                                            .arg(mLicenseInfo->julian()-mainDate));
                    }
                }
                mDistribVersionInfo += addLiTag(QString("The last GAMS version you can use is %1").arg(lastReleaseLicenseString));
                mDistribVersionInfo += addLiTag(QString("The version of your GAMS system is %1").arg(mLocalDistribVersionString));
                if (!(mLicenseInfo->isAlpha() || mLicenseInfo->isBeta())) {
                    mDistribVersionInfo += addLiTag("Please request the download of an older version from <a href=\"mailto:sales@gams.com\">sales@gams.com</a>");
                }
                if (lastReleaseLicense == mVersionInfoLoader->remoteDistribVersion() && (mLicenseInfo->isAlpha() || mLicenseInfo->isBeta())) {
                    mDistribVersionInfo += addLiTag("For ordering an update to use the GOLD release of this version please contact GAMS or your distributor");
                } else {
                    mDistribVersionInfo += addLiTag(QString("For ordering an update to use the most recent version (%1) please contact GAMS or your distributor")
                                                        .arg(mVersionInfoLoader->remoteDistribVersionString()));
                }
            } else {
                if (mLocalDistribVersion >= mVersionInfoLoader->remoteDistribVersion()) {
                    mDistribVersionInfo += addLiTag("You are using the latest GAMS version");
                } else {
                    if (lastReleaseLicense == mVersionInfoLoader->remoteDistribVersion()) {
                        setDistribVersionDetails();
                    } else {
                        if (lastReleaseLicense == mLocalDistribVersion) {
                            mDistribVersionInfo += addLiTag(QString("Your GAMS version %1 is the most recent version you can use with your license")
                                                                .arg(mLocalDistribVersionString));
                            mDistribVersionInfo += addLiTag(QString("For ordering an update to use the most recent version (%1) please contact GAMS or your distributor")
                                                                .arg(lastReleaseLicenseString));
                        } else {
                            auto reldate = mVersionInfoLoader->distribVersions().value(mVersionInfoLoader->remoteDistribVersion());
                            if ((reldate - mainDate) == 1) {
                                mDistribVersionInfo += addLiTag("Your license is 1 day too old to run with the most recent system");
                            } else {
                                mDistribVersionInfo += addLiTag(QString("Your license is %1 days too old to run with the most recent system")
                                                                    .arg(reldate - mainDate));
                            }
                            mDistribVersionInfo += addLiTag(QString("The version of your GAMS system is %1").arg(mLocalDistribVersionString));
                            mDistribVersionInfo += addLiTag(QString("The last GAMS version you can use is %1").arg(lastReleaseLicenseString));
                            mDistribVersionInfo += addLiTag("Please request the download of an older version from <a href=\"mailto:sales@gams.com\">sales@gams.com</a>");
                            mDistribVersionInfo += addLiTag(QString("For ordering an update to use the most recent version (%1) please contact GAMS or your distributor")
                                                                .arg(lastReleaseLicenseString));
                        }
                    }
                }
            }
        }
    }
    mDistribVersionInfo += "</ul>";
}

void CheckForUpdate::setGenericLicenseText()
{
    // this has been reduced... missing PAL functions
    if (mLicenseInfo->isLicenseValidationSuccessful()) {
        if (mLicenseInfo->isGenericLicense()) {
            mDistribVersionInfo += addLiTag("You have a generic license. It can be used on all major platforms");
        }
    }
}

void CheckForUpdate::setDistribVersionDetails()
{
    mDistribVersionInfo += addLiTag("There is an update available for your system");
    mDistribVersionInfo += addLiTag(QString("You can download and use the most recent version of GAMS (%1)")
                                        .arg(mVersionInfoLoader->remoteDistribVersionString()));
    mDistribVersionInfo += addLiTag("Please visit, <a href=\"https://www.gams.com/download/\">https://www.gams.com/download/</a>");
}

QString CheckForUpdate::addLiTag(const QString &text)
{
    return "<li>" + text + "</li>";
}

}
}
}
