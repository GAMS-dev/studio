/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "licensefetcher.h"
#include "process/gamsaboutprocess.h"
#include "gamslicenseinfo.h"
#include "file/uncpath.h"

#include <QStandardPaths>

namespace gams {
namespace studio {
namespace support {

LicenseFetcher::LicenseFetcher(QObject *parent)
    : QObject{parent}
    , mGamsAboutProc(new GamsAboutProcess(this))
{
    mGamsAboutProc->setCurDir(getCurdirForAboutProcess());
    connect(mGamsAboutProc.get(), &GamsAboutProcess::finished, this, &LicenseFetcher::analyzeContent);
}

FetcherState LicenseFetcher::state()
{
    return mFetcherState;
}

void LicenseFetcher::fetchGamsLicense()
{
    stopFetching();

    mFetcherState = fsFetching;
    mFormattedContent = "Fetching GAMS system information. Please wait...";
    mContent.clear();
    mLicense.clear();
    mAccessCode.clear();
    emit changed();

    mGamsAboutProc->clearState();
    mGamsAboutProc->execute();
}

void LicenseFetcher::stopFetching()
{
    mGamsAboutProc->kill();
}

void LicenseFetcher::analyzeContent(int exitCode)
{
    mLastExitCode = exitCode;
    mFetcherState = exitCode ? fsInvalid : fsValid;
    if (exitCode) {
        mFormattedContent = "Error: Fetching GAMS system information. Please check the system log.";
        mLastErrorMessage = mGamsAboutProc->logMessages();
    }
    QStringList about;
    bool licenseLines = false;
    mContent = mGamsAboutProc->content();
    auto lines = mContent.split('\n', Qt::SkipEmptyParts, Qt::CaseInsensitive);
    if (!exitCode && lines.size() >= 3) {
        lines.removeFirst();
        lines.removeLast();
        lines.removeLast();
    }

    QStringList licenseText;
    int licLineCount = -1;
    for(const auto &line : std::as_const(lines)) {
        if (licenseLines) {
            if (licLineCount >= 0) {
                ++licLineCount;
                // extract access code from line 5
                if (licLineCount == 5 && line.startsWith("DC")) {
                    int from = -1;
                    for (int i = line.indexOf('_') ; i > 0 && i < line.length() ; ++i) {
                        if (from < 0) {
                            if (line.at(i) != '_')
                                from = i;
                        } else {
                            if (line.at(i) == '_') {
                                mAccessCode = line.mid(from, i-from);
                                break;
                            }
                        }
                    }
                }
            }
            if (line.startsWith("#L")) {
                // license text start/end marker
                licLineCount = (licLineCount < 0) ? 0 : -1;
                if (licLineCount < 0)
                    mLicense = licenseText;
                continue;
            } else if (line.startsWith("Licensed platform:")) {
                licenseLines = false;
                about << "</pre>" " \r""<br/>" << line << "<br/>";
            } else {
                about << line + "\n";
            }
        } else if (line.startsWith("License ")) {
            about << line << "<br/>";
            licenseLines = true;
            about << "<pre style=\"font-family:'Courier New',monospace\">";
        } else if (line.contains("License file not found")) {
            about << "<br/>" << line  << "<br/>";
        } else if (line.contains("gamslice.txt")) {
            about << line;
        } else {
            about << line << "<br/>";
        }
        if (licLineCount >= 0)
            licenseText << line;
    }

    mFormattedContent = about.join("");
    mLastErrorMessage.clear();
    emit changed();
}

QString LicenseFetcher::getCurdirForAboutProcess()
{
    QString curdir;
#ifdef _WIN64
    GamsLicenseInfo liceInfo;
    auto dataLocations = liceInfo.gamsDataLocations();
    if (dataLocations.isEmpty())
        curdir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    else {
        curdir = dataLocations.first();
        if (curdir.startsWith("\\\\")) {
            curdir = file::UncPath::unc()->toMappedPath(curdir, true);
            if (curdir.isEmpty())
                curdir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        }
    }
#else
    curdir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#endif
    mCurDir = curdir;
    // SysLogLocator::systemLog()->append("CURDIR: "+curdir, LogMsgType::Info);
    return curdir;
}

QString LicenseFetcher::lastErrorMessage() const
{
    return mLastErrorMessage;
}

int LicenseFetcher::lastExitCode() const
{
    return mLastExitCode;
}

QString LicenseFetcher::formattedContent() const
{
    return mFormattedContent;
}

} // namespace support
} // namespace studio
} // namespace gams
