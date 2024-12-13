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
#include <QFile>
#include <QRegularExpression>
#include "search/searchhelpers.h"
#include "searchworker.h"
#include "file/filemeta.h"

namespace gams {
namespace studio {
namespace search {

SearchWorker::SearchWorker(const SearchFile &file, const QRegularExpression &regex, QPoint from, QPoint to,
                           QList<Result> *list, bool showResults)
    : mFiles(QList<SearchFile>() << file), mMatches(list), mRegex(regex), mFrom(from), mTo(to),
      mShowResults(showResults)
{
    // for now, searching with bounds works without extra thread
    mFindInSelection = true;

    // convert 0-based line counting to 1-based
    mFrom += QPoint(0,1);
    mTo += QPoint(0,1);
}

SearchWorker::SearchWorker(const QList<SearchFile> &files, const QRegularExpression &regex,
                           QList<Result> *list, bool showResults)
    : mFiles(files), mMatches(list), mRegex(regex), mFrom(QPoint(0,0)), mTo(QPoint(0,0)),
      mShowResults(showResults)
{
    mFindInSelection = false;
}

void SearchWorker::findInFiles()
{
    bool cacheFull = false;
    NodeId projectGroup;
    int filecounter = 0;
    emit update(0); // initial update to set label to "Searching"

    for (const SearchFile &sf : mFiles) {
        if (cacheFull || thread()->isInterruptionRequested()) break;

        if (sf.fileMeta)
            projectGroup = sf.fileMeta->projectId();

        int lineCounter = 0;
        QFile file(sf.path);
        if (file.open(QFile::ReadOnly)) {

            while (!file.atEnd() && !cacheFull) { // read file

                lineCounter++;
                if (thread()->isInterruptionRequested()) break;

                QByteArray arry = file.readLine();
                // TODO(JM) when switching back to QTextStream this can be removed, as stream doesn't append the \n
                // TODO(JM) encode from utf-8 to file encoding
                if (arry.endsWith('\n')) {
                    if (arry.length() > 1 && arry.at(arry.length()-2) == '\r')
                        arry.chop(2);
                    else
                        arry.chop(1);
                }
                QString line = arry;
                QRegularExpressionMatch match;
                QRegularExpressionMatchIterator i = mRegex.globalMatch(line);
                while (i.hasNext() && !cacheFull) {
                    match = i.next();
                    // abort: too many results
                    if (mMatches->size() > MAX_SEARCH_RESULTS) {
                        cacheFull = true;
                        break;
                    }
                    if (allowInsert(lineCounter, match.capturedStart())) {
                        mMatches->append(Result(lineCounter, match.capturedStart(),
                                                match.capturedLength(), file.fileName(),
                                                projectGroup, line));
                    }
                }

                // update periodically
                if (lineCounter % 250000 == 0)
                    emit update(mMatches->size());
            }
            file.close();
        }

        if (filecounter++ % 1000 == 0)
            emit update(mMatches->size());
    }
    emit showResults(mShowResults, mMatches);
    if (!mFindInSelection) thread()->quit();
}

bool SearchWorker::allowInsert(int line, int col) {
    // no limit set
    if (!mFindInSelection) return true;

    // check lower bound
    if (line < mFrom.y()) return false;
    if (line == mFrom.y() && col < mFrom.x()) return false;

    // check upper bound
    if (line > mTo.y()) return false;
    if (line == mTo.y() && col > mTo.x()) return false;

    return true;
}

}
}
}
