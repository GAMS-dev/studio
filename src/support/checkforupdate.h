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
#ifndef CHECKFORUPDATE_H
#define CHECKFORUPDATE_H

#include <QObject>
#include <QRegularExpression>

namespace gams {
namespace studio {
namespace support {

class GamsLicenseInfo;
class VersionInfoLoader;

///
/// \brief Handels all steps required to perform the check for update process.
///
class CheckForUpdate : public QObject
{
    Q_OBJECT

public:
    CheckForUpdate(bool downloadVersions, QObject* parent = nullptr);

    ~CheckForUpdate();

    QString versionInformation() const;

    ///
    /// \brief Short version information string, e.g. used at Studio start up.
    /// \return Short version information string.
    ///
    QString versionInformationShort() const;

    ///
    /// \brief Get current GAMS distribution version as integer.
    /// \return Latest version integer, like <c>2510</c>.
    ///         <c>-1</c> in case of an error.
    ///
    int localDistribVersion() const;

    ///
    /// \brief Get current GAMS distribution version as short string.
    /// \return Short version string, like <c>"25.1"</c> (MAJOR.MINOR).
    ///         <c>""</c> in case of an error.
    ///
    QString localDistribVersionShort() const;

    ///
    /// \brief Get latest GAMS distribution version as integer.
    /// \return Latest version integer, like <c>2510</c>.
    /// \remark This function connects to <c>gams.com</c>.
    ///         <c>-1</c> in case of an error.
    ///
    int remoteDistribVersion() const;

    ///
    /// \brief Check if the used GAMS distribution is the latest one.
    /// \return <c>true</c> if it is the latest GAMS distribution; otherwise <c>false</c>.
    /// \remark This function connects to <c>gams.com</c>.
    ///
    bool isLocalDistribLatest();

    ///
    /// \brief Get local GAMS Studio version.
    /// \return GAMS Studio version as <c>int</c>, like <c>120</c>.
    /// \remark Used to check for updates.
    ///
    int localStudioVersion() const;

public slots:
    void checkForUpdate(bool downloadVersions);

signals:
    void versionInformationAvailable();

private slots:
    void updateVersionInformation();

private:
    void getDistribLicenseInfo();
    void setGenericLicenseText();
    void setDistribVersionDetails();

    QString addLiTag(const QString& text);

private:
    QScopedPointer<GamsLicenseInfo> mLicenseInfo;
    QScopedPointer<VersionInfoLoader> mVersionInfoLoader;

    QRegularExpression mDistribRegEx;
    QRegularExpression mStudioRegEx;

    bool mValidLicense = false;

    int mLocalDistribVersion;
    QString mLocalDistribVersionString;

    int mLocalStudioVersion;

    QString mDistribVersionInfo;
    QString mStudioVersionInfo;

    int mToday;
    int mEvalDate;
    int mMainDate;
    const int GraceDays = 30;
};

}
}
}

#endif // CHECKFORUPDATE_H
