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
#include "fileworker.h"

namespace gams {
namespace studio {
namespace search {

FileWorker::FileWorker(AbstractSearchFileHandler* fh) : mFileHandler(fh)
{ }

FileWorker::FileWorker(const SearchParameters &params, AbstractSearchFileHandler* fh)
    : mParameters(params), mFileHandler(fh)
{ }

void FileWorker::setParameters(const SearchParameters &parameters)
{
    mParameters = parameters;
}

QList<SearchFile> FileWorker::collectFilesInFolder()
{
    QList<SearchFile> files;

    if (mParameters.path.isEmpty()) {
        qWarning() << "SearchParameter path is empty but needed for file collection in folder";
        return files;
    }

    QDir dir(mParameters.path);

    QDirIterator::IteratorFlag options = mParameters.includeSubdirs
                                             ? QDirIterator::Subdirectories
                                             : QDirIterator::NoIteratorFlags;
    QDirIterator it(dir.path(), QDir::Files, options);
    while (it.hasNext()) {
        if (thread()->isInterruptionRequested()) break;

        QString path = it.next();
        if (path.isEmpty()) break;

        files << SearchFile(path);
    }

    QList<SearchFile> filteredList = filterFiles(files, mParameters);
    emit filesCollected(filteredList);
    return filteredList;
}

QList<SearchFile> FileWorker::filterFiles(QList<SearchFile> files, SearchParameters params)
{
    bool ignoreWildcard = params.scope == Scope::ThisFile || params.scope == Scope::Selection;

    // create list of include filter regexes
    QList<QRegularExpression> includeFilterList;
    for (const QString &s : std::as_const(params.includeFilter)) {
        QString pattern = QString(QDir::separator() + s.trimmed() + "$").replace('.', "\\.").replace('?', '.').replace("*", ".*");
        includeFilterList.append(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
    }

    // create list of exclude filters
    QStringList excludeFilter = params.excludeFilter;
    excludeFilter << "*.gdx" << "*.zip";

    QList<QRegularExpression> excludeFilterList;
    for (const QString &s : std::as_const(excludeFilter)) {
        QString pattern = QString(QDir::separator() + s.trimmed() + "$").replace('.', "\\.").replace('?', '.').replace("*", ".*");
        excludeFilterList.append(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
    }

    // filter files
    QList<SearchFile> res;
    for (const SearchFile &sf : std::as_const(files)) {
        bool include = includeFilterList.count() == 0;

        // check if file fits inclusion filter
        for (const QRegularExpression &wildcard : std::as_const(includeFilterList)) {
            include = wildcard.match(sf.path).hasMatch();
            if (include) break; // one match is enough, dont overwrite result
        }

        // check if file is included but shall be exlcuded
        if (include)
            for (const QRegularExpression &wildcard : std::as_const(excludeFilterList)) {
                include = !wildcard.match(sf.path).hasMatch();

                if (!include) break;
            }

        // if we can get an fm check if that file is read only
        FileMeta* fm = mFileHandler->findFile(sf.path);
        if ((include || ignoreWildcard) && (!params.ignoreReadOnly || (fm && !fm->isReadOnly()))) {
            res << ((fm && fm->isModified()) ? sf : sf.path);
        }
    }
    return res;
}

}
}
}
