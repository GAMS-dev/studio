/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMSINFO_H
#define GAMSINFO_H

#include <QString>

namespace gams {
namespace studio {

class GAMSPaths
{
public:
    ///
    /// \brief Get GAMS system directory.
    /// \return Returns the GAMS system directory.
    /// \remark If GAMS Studio is part of the GAMS distribution a relateive
    ///         path based on the executable location is returned;
    ///         otherwise the PATH environment variable used to find GAMS.
    ///
    static QString systemDir();

    static QString defaultWorkingDir();

    static QString userDocumentsDir();

    static QString userModelLibraryDir();

    ///
    /// \brief Get the file path even if the file does not exists.
    /// \param path Path to the file.
    /// \return Returns the canonical file path if the file exists;
    ///         otherwise the absolute file path.
    ///
    static QString filePath(const QString &path);

    static QString path(const QString &file);


    // this macro is needed to deactivate resolving of symlinks on macos.
    // if not deactivated a workaround breaks that allows users to circumvent file naming limitations.
    // to be specific, files or pathes containing spaces.
    // however, if deactivated on other platforms, shortcuts will be opened as textfile instead of being followed.
#ifdef __APPLE__
#define DONTRESOLVESYMLINKSONMACOS QFileDialog::DontResolveSymLinks
#else
#define DONTRESOLVESYMLINKSONMACOS 0
#endif

private:
    GAMSPaths();
};

}
}

#endif // GAMSINFO_H
