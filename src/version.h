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
#ifndef VERSION_H
#define VERSION_H

#define FI_FILEVERSION             STUDIO_MAJOR_VERSION,STUDIO_MINOR_VERSION,STUDIO_PATCH_LEVEL,0
#define FI_FILEVERSION_STR         "0.1.0.0\0"
#define FI_ORIGINALFILENAME_STR    "studio.exe"
#define FI_INTERNALNAME_STR        "GAMS Studio"
#define FI_PRODUCTNAME_STR         "GAMS Studio"
#define FI_FILEDESCRIPTION_STR     "GAMS Studio - optimization modeling development"

#define FI_PRODUCTVERSION          STUDIO_MAJOR_VERSION,STUDIO_MINOR_VERSION,STUDIO_PATCH_LEVEL,00
#define FI_PRODUCTVERSION_STR      "25.00.00.00"

#define FI_COMPANYNAME_STR         "GAMS Software GmbH"
#define FI_COMPANYDOMAIN_STR       "gams.com"
#define FI_LEGALCOPYRIGHT_STR      "Copyright GAMS Software GmbH and GAMS Development Corp."
#define FI_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define FI_LEGALTRADEMARKS2_STR    FI_LEGALTRADEMARKS1_STR

namespace gams {
namespace studio {

///
/// \brief Converts the STUDIO_VERSION into an number.
/// \return The STUDIO_VERSION as number.
/// \remark Used to check for updates.
///
int versionToNumber();

///
/// \brief Get current GAMS Distribution version number.
/// \param version Version string buffer.
/// \param length Length of the version string buffer.
/// \return The GAMS Distribution version number as string. The
///         same as the <c>version</c> argument.
///
char* currentGAMSDistribVersion(char* version, int length=16);

}
}

#endif // VERSION_H
