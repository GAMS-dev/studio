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
#ifndef SEARCHCOMMON_H
#define SEARCHCOMMON_H

#include <QRegularExpression>
#include <QString>
#include <QStringList>

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

struct Parameters
{
    QRegularExpression regex;
    QString searchTerm;
    QString replaceTerm;

    bool useRegex = false;
    bool caseSensitive = false;
    bool searchBackwards = false;
    bool showResults = false;
    bool ignoreReadOnly = false;
    bool includeSubdirs = false;

    Scope scope = Scope::Selection;
    QString path;
    QStringList excludeFilter;
    QStringList includeFilter;
};

class SearchCommon
{
private:
    SearchCommon();

public:
    static QString toRegularExpression(const QString &wildcard);

    static void includeFilters(const QStringList &wildcards, QList<QRegularExpression> &includeRegEx);

    static void excludeFilters(const QStringList &wildcards, QList<QRegularExpression> &excludeRegEx);

    static QString fileName(const QString &path, QChar separator);
};

}
}
}

#endif // SEARCHCOMMON_H
