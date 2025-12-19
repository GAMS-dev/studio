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
#include "searchcommon.h"

#include <QFileInfo>

namespace gams {
namespace studio {
namespace search {

SearchCommon::SearchCommon()
{

}

QString SearchCommon::toRegularExpression(const QString &wildcard)
{
    QString pattern("^" + wildcard.trimmed() + "$");
    return pattern.replace('.', "\\.").replace('?', '.').replace("*", ".*");
}

void SearchCommon::includeFilters(const QStringList &wildcards,
                                  QList<QRegularExpression> &includeRegEx)
{
    for (const QString &wildcard : std::as_const(wildcards)) {
        auto pattern = toRegularExpression(wildcard);
        auto regex = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        includeRegEx.append(regex);
    }
}

void SearchCommon::excludeFilters(const QStringList &wildcards,
                                  QList<QRegularExpression> &excludeRegEx)
{
    QStringList finalWildcards;
    QStringList additionalWildcards { "*.efi", "*.gdx", "*.guc", "*.opt", "*.pf", "*.ref", "*.yaml", "*.yml", "*.zip" };
    finalWildcards << wildcards << additionalWildcards;
    for (const QString &wildcard : std::as_const(finalWildcards)) {
        auto pattern = toRegularExpression(wildcard);
        auto regex = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        excludeRegEx.append(regex);
    }
}

QString SearchCommon::fileName(const QString &path, QChar separator)
{
    if (path.isEmpty())
        return path;
    auto index = path.lastIndexOf(separator);
    return index < 0 ? path : path.last(path.size()-1-index);
}

}
}
}
