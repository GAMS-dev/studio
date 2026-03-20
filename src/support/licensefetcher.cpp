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
#include "logger.h"

#include <QStandardPaths>
#include <QRegularExpression>
#include <QTimeZone>
#include <QFile>
#include <QTcpSocket>

using namespace gams::studio::support::LicenseStateEnum;

namespace gams {
namespace studio {
namespace support {

// using raw string avoids double "\\"
const QRegularExpression CRexBaseDate(R"(G(\d{6})[+\-|])");
const QRegularExpression CRexCheckout(R"(^node:\d+@(\d+)_)");
const QRegularExpression CRexServer(R"(^server:([a-zA-Z0-9.-]+)_port:(\d+))");
const QString CLicenseInvalid1("*** Error: The installed license ");
const QString CLicenseInvalid2(" is invalid.");

LicenseFetcher::LicenseFetcher(QObject *parent)
    : QObject{parent}
    , mGamsAboutProc(new GamsAboutProcess(this))
{
    mFetchTimer.setSingleShot(true);
    mBaseDate.setTimeZone(QTimeZone::utc());
    mGamsAboutProc->setCurDir(getCurdirForAboutProcess());
    connect(mGamsAboutProc, &GamsAboutProcess::finished, this, &LicenseFetcher::analyzeContent);
}

LicenseFetcher::~LicenseFetcher()
{
    delete mGamsAboutProc;
}

LicenseState LicenseFetcher::state()
{
    return mLicenseState;
}

void LicenseFetcher::fetchGamsLicense()
{
    stopFetching();

    // Only show "checking" on previous invalid state
    QList<LicenseState> invalidLicenses = {lsNone, lsLocalInvalid, lsNetInvalid, lsNetCheckoutInvalid};
    if (invalidLicenses.contains(mLicenseState))
        mLicenseState = lsChecking;

    mFormattedContent = "Fetching GAMS system information. Please wait...";
    mContent.clear();
    mLicense.clear();
    mLicenseValids.clear();
    mDurationChar = QChar('~');
    mDurationMonths = 0;
    mAccessCode.clear();
    emit stateChanged(mLicenseState);

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

    QString modulesLine;
    QStringList licenseText;
    int licLineCount = -1;
    for(const auto &line : std::as_const(lines)) {
        if (line.startsWith(CLicenseInvalid1)) {
            QString licFile = line.mid(CLicenseInvalid1.length(), line.indexOf(CLicenseInvalid2) - CLicenseInvalid1.length());
            licenseText = readLicenseFile(licFile);
            if(!licenseText.isEmpty()) {
                mLicenseFile = licFile;
                mLicense = licenseText;
                checkLicense(mLicense);
            }
        }
        if (licenseLines) {
            if (licLineCount >= 0)
                ++licLineCount;
            if (line.startsWith("#L")) {
                // license text start/end marker
                licLineCount = (licLineCount < 0) ? 0 : -1;
                if (licLineCount < 0) {
                    mLicense = licenseText;
                    checkLicense(mLicense);
                }
                continue;
            } else if (line.startsWith("Licensed platform:")) {
                licenseLines = false;
                about << "</pre>" " \r""<br/>" << line << "<br/>";
            } else {
                about << line + "\n";
            }
        } else if (line.startsWith("License ")) {
            mLicenseFile = line.mid(QString("License ").length()).trimmed();
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
    return curdir;
}

QStringList LicenseFetcher::readLicenseFile(const QString &filename)
{
    QStringList res;
    QFile file(filename);
    if (file.exists() && file.open(QFile::ReadOnly)) {
        QString content = file.readAll();
        res = content.split('\n');
        file.close();
    }
    return res;
}

void LicenseFetcher::ensureLicenseCopy()
{
    if (QFile::exists(mLicenseFile) && mLicenseType == lsNet) {
        QString bkFile = mLicenseFile + ".~bk";
        if (QFile::exists(bkFile)) {
            LicenseFetcher fetcher(this);
            fetcher.mLicense = fetcher.readLicenseFile(bkFile);
            if (fetcher.mLicense.isEmpty())
                return;
            fetcher.checkLicense(fetcher.mLicense);
            if (fetcher.mLicenseType == lsNet && fetcher.mAccessCode == mAccessCode)
                return; // backup from same access code already exists
            QFile::remove(bkFile);
        }
        QFile::copy(mLicenseFile, bkFile);
    }
}

bool LicenseFetcher::restoreLicenseCopy()
{
    if (QFile::exists(mLicenseFile) && mLicenseType == LicenseStateEnum::lsNetCheckout) {
        QString bkFile = mLicenseFile + ".~bk";
        if (QFile::exists(bkFile)) {
            LicenseFetcher fetcher(this);
            fetcher.mLicense = fetcher.readLicenseFile(bkFile);
            if (fetcher.mLicense.isEmpty())
                return false;
            fetcher.checkLicense(fetcher.mLicense);
            if (fetcher.mLicenseType != lsNet || fetcher.mAccessCode != mAccessCode)
                return false; // backup is invalid
            QFile::remove(mLicenseFile);
        }
        QFile::copy(bkFile, mLicenseFile);
        emit info("Checkout expired, switched back to network license.");
        return true;
    }
    return false;
}

void LicenseFetcher::checkLicense(const QStringList &lines)
{
    QString modulesLine;
    int lineNr = 0;
    for(const auto &line : std::as_const(lines)) {
        ++lineNr;
        // extract base date from line 1
        if (lineNr == 1) {
            fetchBaseDate(line);

            // extract licenses and validation from lines 3 and 4
        } else if (lineNr == 3) {
            modulesLine = line;
        } else if (lineNr == 4) {
            fetchLicenseValues(modulesLine, line);

            // extract access code from line 5
        } else if (lineNr == 5 && line.startsWith("DC")) {
            fetchAccessCode(line);

            // extract access code from line 6
        } else if (lineNr == 6) {
            fetchLicenseType(line);
        }
    }
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
        if (!(i%2)) continue;
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

void LicenseFetcher::fetchLicenseType(const QString &line)
{
    mLicenseType = lsLocal;
    mCheckoutHours = 0;
    mLicenseServer = QString();
    mLicensePort = 8080;
    if (line.startsWith("server:")) {
        mLicenseType = lsNet;
        QRegularExpressionMatch match = CRexServer.match(line);
        if (match.hasMatch()) {
            mLicenseServer = match.captured(1);
            mLicensePort = match.captured(2).toInt();
        }
    } else if (line.startsWith("node:")) {
        QRegularExpressionMatch match = CRexCheckout.match(line);
        if (match.hasMatch()) {
            mLicenseType = lsNetCheckout;
            mCheckoutHours = match.captured(1).toInt();
        }
    }
}

void LicenseFetcher::updateState()
{
    mFetchTimer.stop();
    mLicenseState = mLicenseType;
    QDateTime now = QDateTime::currentDateTime();
    mExpire = QDateTime();
    if (mLicenseType == lsNetCheckout) {
        mExpire = mBaseDate.addSecs(mCheckoutHours * 3600).toLocalTime();
        qint64 sec = now.secsTo(mExpire);
        if (sec > 3600) {
            mFetchTimer.singleShot((sec -3600 +2) * 1000, this, &LicenseFetcher::fetchGamsLicense);
        } else {
            if (sec > 0) {
                mLicenseState = lsNetCheckoutEnd;
                mFetchTimer.singleShot((sec +2) * 1000, this, &LicenseFetcher::fetchGamsLicense);
            } else {
                mLicenseState = lsNetCheckoutInvalid;
            }
        }
    } else {
        if (mDurationChar != '~') {
            bool ensureServer = mLicenseType == lsNet;
            mExpire = mBaseDate.addDays(mDurationMonths * 30).toLocalTime();
            if (mExpire < now) {
                qint64 sec = now.secsTo(mExpire);
                if (sec < 86400 * 7) { // when change is in less than a week, start a timer
                    mFetchTimer.singleShot((sec +2) * 1000, this, &LicenseFetcher::fetchGamsLicense);
                }
                mLicenseState = LicenseState(mLicenseState + 1);
                mExpire = mExpire.addDays(30);
                if (mExpire < now) {
                    mLicenseState = LicenseState(mLicenseState + 1);
                    ensureServer = false;
                } else if (sec < 86400 * 7) { // when change is in less than a week, start a timer
                    mFetchTimer.singleShot((sec +2) * 1000, this, &LicenseFetcher::fetchGamsLicense);
                }
            } else {
                mExpire = mExpire.addDays(30);
            }
            if (ensureServer) pingServer();
        }
    }
    if (mLicenseState == lsNetCheckoutInvalid) {
        if (restoreLicenseCopy())
            fetchGamsLicense();
        return;
    } else
        ensureLicenseCopy();

    emit stateChanged(mLicenseState, mExpire);
}

void LicenseFetcher::pingServer()
{
    if (mLicenseServer.isEmpty())
        return;
    QTcpSocket *socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, [socket]() {
        socket->disconnectFromHost();
    });
    connect(socket, &QTcpSocket::errorOccurred, [this, socket](QAbstractSocket::SocketError error) {
        qDebug() << "LicenseServer: Error: " << error;
        qDebug() << "   server " << mLicenseServer << "  port "<< mLicensePort;
        mLicenseState = lsNetNoConnection;
        emit stateChanged(mLicenseState, mExpire);
        socket->deleteLater();
    });
    connect(socket, &QTcpSocket::disconnected, [socket]() {
        socket->deleteLater();
    });

    socket->connectToHost(mLicenseServer, mLicensePort);
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
