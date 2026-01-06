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
#ifndef UNCPATH_H
#define UNCPATH_H

#include <QObject>
#include <QSet>

namespace gams {
namespace studio {
namespace file {

class UncPath : public QObject
{
    Q_OBJECT
public:

    ///
    /// \brief Singleton constructor for UncPath
    /// \return
    ///
    static UncPath* unc();

    ///
    /// \brief Release UncPath singleton
    ///
    static void releaseUnc();

    ///
    /// \brief Destructor
    ///
    ~UncPath() override;

    ///
    /// \brief Translates a UNC path to a local path with a mapped drive letter (Windows only)
    /// \param uncPath the UNC path
    /// \param forceGenerate if no match is found, generate a mapping for a free drive letter
    /// \return The path translated to a mapped drive
    ///
    QString toMappedPath(const QString &uncPath, bool forceGenerate = false);

    ///
    /// \brief Generate a mapping for a free drive letter (Windows only)
    /// \param localDrive The drive letter to map to
    /// \param uncPath the UNC path. Credentials needs to be already unlocked.
    /// \return TRUE if the mapping suceeded
    ///
    bool mapNetworkDrive(const QString &localDrive, const QString &uncPath);

signals:
    // remark: Signal destroyed is used to set the static member mUnc to nullptr.

private:
    static UncPath *mUnc;
    QStringList mTempMapped;
    explicit UncPath(QObject *parent = nullptr);

private:
    bool unmapDrive(const QString &driveLetter, bool force = false);
};

} // namespace file
} // namespace studio
} // namespace gams

#endif // UNCPATH_H
