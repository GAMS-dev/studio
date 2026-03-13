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

namespace gams {
namespace studio {

class GamsAboutProcess;

namespace support {

enum FetcherState {
    fsInitial,
    fsFetching,
    fsInvalid,
    fsValid,
};

class LicenseFetcher : public QObject
{
    Q_OBJECT
public:
    explicit LicenseFetcher(QObject *parent = nullptr);
    ~LicenseFetcher() override;
    FetcherState state();
    void fetchGamsLicense();
    void stopFetching();
    int lastExitCode() const;
    QString lastErrorMessage() const;
    QString formattedContent() const;

signals:
    void changed();

private slots:
    void analyzeContent(int exitCode);

private:
    QString getCurdirForAboutProcess();


private:
    FetcherState mFetcherState = fsInitial;
    GamsAboutProcess* mGamsAboutProc;
    QString mCurDir;
    int mLastExitCode = -1;
    QString mLastErrorMessage;
    QString mContent;
    QString mFormattedContent;
    QStringList mLicense;
    QString mAccessCode;

};

} // namespace support
} // namespace studio
} // namespace gams

#endif // LICENSEFETCHER_H
