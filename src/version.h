/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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

#define QUOTE_HELPER(m) #m
#define QUOTE(m) QUOTE_HELPER(m)

#define GAMS_VERSION                 STUDIO_MAJOR_VERSION,STUDIO_MINOR_VERSION,STUDIO_PATCH_LEVEL,0
#define GAMS_VERSION_STR             QUOTE(STUDIO_MAJOR_VERSION.STUDIO_MINOR_VERSION.STUDIO_PATCH_LEVEL.0)
#define GAMS_ORIGINALFILENAME_STR    "studio.exe"
#define GAMS_INTERNALNAME_STR        "GAMS Studio"
#define GAMS_PRODUCTNAME_STR         "GAMS Studio"
#define GAMS_FILEDESCRIPTION_STR     "GAMS Studio"

#define GAMS_ORGANIZATION_STR        "GAMS"
#define GAMS_COMPANYNAME_STR         "GAMS Development Corp."
#define GAMS_COMPANYDOMAIN_STR       "gams.com"
#define GAMS_LEGALCOPYRIGHT_STR      "Copyright GAMS Software GmbH, Copyright GAMS Development Corp."
#define GAMS_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define GAMS_LEGALTRADEMARKS2_STR    GAMS_LEGALTRADEMARKS1_STR

#endif // VERSION_H
