/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#ifndef COMMON_PATHS_H
#define COMMON_PATHS_H

#include <QString>

namespace gams {
namespace studio {

/// this macro is needed to deactivate resolving of symlinks on macos.
/// if not deactivated a workaround breaks that allows users to circumvent file naming limitations.
/// to be specific, files or pathes containing spaces.
/// however, if deactivated on other platforms, shortcuts will be opened as textfile instead of being followed.
#ifdef __APPLE__
#define DONT_RESOLVE_SYMLINKS_ON_MACOS QFileDialog::DontResolveSymlinks
#else
#define DONT_RESOLVE_SYMLINKS_ON_MACOS QFileDialog::Option()
#endif

class CommonPaths
{
public:

    static const QString& documentationDir();
    static QString modelLibraryDir(const QString &libname);

    ///
    /// \brief Get GAMS system directory.
    /// \return Returns the GAMS system directory.
    ///
    static const QString& systemDir();

    ///
    /// \brief Set GAMS system directory.
    /// \return Returns the GAMS system directory.
    /// \remark If GAMS Studio is part of the GAMS distribution a relateive
    ///         path based on the executable location is returned;
    ///         otherwise the PATH environment variable used to find GAMS.
    ///
    static void setSystemDir(const QString &sysdir = QString());

    ///
    /// \brief Checks if the current system directory is a valid GAMS directory.
    /// \return <c>true</c> if the system directory contains GAMS; otherwise <c>false</c>.
    ///
    static bool isSystemDirValid();

    static QString defaultWorkingDir();

    static QString gamsDocumentsDir();

    static QString userDocumentsDir();

    static QString userModelLibraryDir();

    ///
    /// \brief Get the absolut file path.
    /// \param filePath File to get the absolute path for.
    /// \return Returns the absolute file path if \p filePath is not empty;
    ///         otherwise an empty string is returned.
    ///
    static QString absolutFilePath(const QString &filePath);

    ///
    /// \brief Get the absolut directory path.
    /// \param dir Directory to get the absolute path for.
    /// \return Returns the absolute directory path if \p dir is not empty;
    ///         otherwise an empty string is returned.
    ///
    static QString absolutPath(const QString &dir);

    ///
    /// \brief Get GAMS config file name (with path).
    /// \return Returns GAMS config file name.
    ///
    static QString configFile();

    ///
    /// \brief Get GAMS license file name (with path).
    /// \return Returns GAMS license file name.
    ///
    static QString licenseFile();

    ///
    /// \brief Get the changelog file including its path.
    /// \return Returns the changelog file name.
    ///
    static QString changelog();

private:
    CommonPaths();

private:
    static QString SystemDir;
    static const QString ConfigFile;
    static const QString DocumentationDir;
    static const QString ModlibsPrefixPath;
    static const QString LicenseFile;
};

}
}

#endif // GAMSINFO_H
