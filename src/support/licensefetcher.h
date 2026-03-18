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
#ifndef LICENSEFETCHER_H
#define LICENSEFETCHER_H

#include <QObject>
#include <QDateTime>

namespace gams {
namespace studio {

class GamsAboutProcess;

namespace support {

inline namespace LicenseStateEnum {
enum LicenseState {
    lsNone,
    lsChecking,
    lsLocal,
    lsLocalEnd,
    lsLocalInvalid,
    lsNet,
    lsNetEnd,
    lsNetInvalid,
    lsNetNoConnection,
    lsNetCheckout,
    lsNetCheckoutEnd,
};
};

class LicenseFetcher : public QObject
{
    Q_OBJECT
public:
    explicit LicenseFetcher(QObject *parent = nullptr);
    ~LicenseFetcher() override;
    LicenseState state();
    void fetchGamsLicense();
    void stopFetching();
    int lastExitCode() const;
    QString lastErrorMessage() const;
    QString formattedContent() const;
    QString accessCode() const;

signals:
    void changed();
    void stateChanged(LicenseState state);
    void error(const QString &message);

private slots:
    void analyzeContent(int exitCode);

private:
    QString getCurdirForAboutProcess();
    void checkLicense(const QStringList &lines);
    void fetchBaseDate(const QString &line);
    void fetchLicenseValues(const QString &lineLic, const QString &lineVal);
    void fetchAccessCode(const QString &line);
    void fetchLicenseType(const QString &line);
    void updateState();

private:
    GamsAboutProcess* mGamsAboutProc;
    LicenseState mLicenseState = lsNone;
    LicenseState mLicenseType = lsNone;
    QString mCurDir;
    int mLastExitCode = -1;
    QString mLastErrorMessage;
    QString mContent;
    QString mFormattedContent;
    QStringList mLicense;
    QDateTime mBaseDate;
    QHash<QString, QString> mLicenseValids;
    QChar mDurationChar = QChar('~');
    int mDurationMonths = 0;
    int mCheckoutHours = 0;
    QString mAccessCode;

};

} // namespace support
} // namespace studio
} // namespace gams

#endif // LICENSEFETCHER_H
