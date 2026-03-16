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
#include <QRegularExpression>

namespace gams {
namespace studio {
namespace support {

const QRegularExpression CRexBaseDate(R"(G(\d{6})-)");    // using raw string avoids double "\\"

LicenseFetcher::LicenseFetcher(QObject *parent)
    : QObject{parent}
    , mGamsAboutProc(new GamsAboutProcess(this))
{
    mGamsAboutProc->setCurDir(getCurdirForAboutProcess());
    connect(mGamsAboutProc, &GamsAboutProcess::finished, this, &LicenseFetcher::analyzeContent);
}

LicenseFetcher::~LicenseFetcher()
{
    delete mGamsAboutProc;
}

FetcherState LicenseFetcher::state()
{
    return mFetcherState;
}

void LicenseFetcher::fetchGamsLicense()
{
    stopFetching();

    mFetcherState = fsFetching;
    mLicenseState = lsChecking;
    mFormattedContent = "Fetching GAMS system information. Please wait...";
    mContent.clear();
    mLicense.clear();
    mLicenseValids.clear();
    mDurationChar = QChar('~');
    mDurationMonths = 0;
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
        mLicenseState = lsNone;
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

    QString licensesLine;
    QStringList licenseText;
    int licLineCount = -1;
    for(const auto &line : std::as_const(lines)) {
        if (licenseLines) {
            if (licLineCount >= 0) {
                ++licLineCount;
                // extract base date from line 1
                if (licLineCount == 1) {
                    fetchBaseDate(line);

                // extract licenses and validation from lines 3 and 4
                } else if (licLineCount == 3) {
                    licensesLine = line;
                } else if (licLineCount == 4) {
                    fetchLicenseValues(licensesLine, line);

                // extract access code from line 5
                } else if (licLineCount == 5 && line.startsWith("DC")) {
                    fetchAccessCode(line);
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
    updateState();
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

void LicenseFetcher::fetchBaseDate(const QString &line)
{
    QRegularExpressionMatch match = CRexBaseDate.match(line);
    if (match.hasMatch()) {
        QString sDate = match.captured(1);
        if (sDate.length() != 6) {
            emit error("Invalid license date");
            return;
        }
        QList<int> vals;
        for (int i = 0; i < 3; ++i)
            vals << sDate.mid(2*i, 2).toInt();

        // Avoid century switch bug
        int currentYear = QDate::currentDate().year();
        int century = (currentYear / 100) * 100;
        int baseYear = century +  vals.at(0);
        if (baseYear > currentYear + 10)
            baseYear -= 100;

        mBaseDate.setDate(QDate(baseYear, vals.at(1), vals.at(2)));
    }
}

int convertDuration(const QChar &c)
{
    if (c < '1') return 0;
    if (c <= '9')
        return QString(c).toInt();
    if (c < 'A') return 0;
    if (c <= 'Z')
        return c.unicode() - QChar('A').unicode() + 10;
    if (c < 'a') return 0;
    if (c <= 'z')
        return c.unicode() - QChar('a').unicode() + 36;
    return 0;
}

void LicenseFetcher::fetchLicenseValues(const QString &lineLic, const QString &lineVal)
{
    if (lineVal.length() < lineLic.length()) {
        emit error("Invalid license values");
        return;
    }
    mLicenseValids.clear();
    mDurationChar = QChar('~');
    mDurationMonths = 0;
    for (int i = 0; i < lineLic.length(); ++i) {
        if (lineLic.at(i) == '_') break;
        if (lineVal.at(i) == '_') {
            emit error("Inconsistent license values");
            mDurationChar = QChar('0');
            mLicenseValids.clear();
            break;
        }
        if (i%2) continue;
        if (lineVal.at(i) == 0) continue;   // license hasn't a duration
        mLicenseValids.insert(lineLic.mid(i-1, 2), lineVal.at(i));
        if (lineVal.at(i) < mDurationChar)
            mDurationChar = lineVal.at(i);
    }
    mDurationMonths = convertDuration(mDurationChar);
}

void LicenseFetcher::fetchAccessCode(const QString &line)
{
    mAccessCode = QString();
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

void LicenseFetcher::updateState()
{
    // TODO(JM) Update the license state


    emit changed();
}

QString LicenseFetcher::accessCode() const
{
    return mAccessCode;
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
