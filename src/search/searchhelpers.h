/*
 * This file is part of the GAMS Studio project.
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
#ifndef SEARCHHELPERS_H
#define SEARCHHELPERS_H

#include "file/filemeta.h"
#include <QRegularExpression>

namespace gams {
namespace studio {
namespace search {

    enum Scope {
        Selection,
        ThisFile,
        ThisProject,
        OpenTabs,
        AllFiles,
        Folder
    };

    struct SearchFile
    {
        QString path;
        FileMeta* fileMeta;

        SearchFile(FileMeta* fm) : path(fm ? fm->location() : ""), fileMeta(fm) { }
        SearchFile(const QString &path, FileMeta* fm = nullptr) : path(path), fileMeta(fm) { }

        bool operator==(const SearchFile &rhs) const
        {
            return fileMeta && fileMeta == rhs.fileMeta &&
                   path == rhs.path;
        }
    };

    struct SearchParameters
    {
        QRegularExpression regex;
        QString searchTerm;
        QString replaceTerm;

        FileMeta* currentFile = nullptr;

        bool useRegex;
        bool caseSensitive;
        bool searchBackwards;
        bool showResults;
        bool ignoreReadOnly;
        bool includeSubdirs;

        Scope scope;
        QString path;
        QStringList excludeFilter;
        QStringList includeFilter;
    };

}
}
}
#endif // SEARCHHELPERS_H
