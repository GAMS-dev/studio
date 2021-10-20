/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "searchresultmodel.h"
#include "searchworker.h"
#include "file/filemeta.h"

namespace gams {
namespace studio {
namespace search {

SearchWorker::SearchWorker(FileMeta* file, QRegularExpression regex, QPoint from, QPoint to, QList<Result> *list)
    : mFiles(QList<FileMeta*>() << file), mMatches(list), mRegex(regex), mFrom(from), mTo(to)
{
    // for now, searching with bounds works without extra thread
    hasOwnThread = false;

    // convert 0-based line counting to 1-based
    mFrom += QPoint(0,1);
    mTo += QPoint(0,1);
}

SearchWorker::SearchWorker(QList<FileMeta*> fml, QRegularExpression regex, QList<Result> *list)
    : mFiles(fml), mMatches(list), mRegex(regex), mFrom(QPoint(0,0)), mTo(QPoint(0,0))
{
    hasOwnThread = true;
}

SearchWorker::~SearchWorker()
{
}

void SearchWorker::findInFiles()
{
    bool cacheFull = false;
    for (FileMeta* fm : qAsConst(mFiles)) {
        if (cacheFull) break;

        int lineCounter = 0;
        QFile file(fm->location());
        if (file.open(QIODevice::ReadOnly)) {

            QTextStream in(&file);
            in.setCodec(fm->codec());

            while (!in.atEnd() && !cacheFull) { // read file

                lineCounter++;
                if (lineCounter % 500 == 0 && thread()->isInterruptionRequested()) break;

                QString line = in.readLine();

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
                                                line.trimmed()));
                    }
                }

                // update periodically
                if (lineCounter % 250000 == 0)
                    emit update(mMatches->size());
            }
            file.close();
        }
        emit update(mMatches->size());
    }
    emit resultReady();
    if (hasOwnThread) thread()->quit();
}

bool SearchWorker::allowInsert(int line, int col) {
    // no limit set
    if (mTo == QPoint(0,0)) return true;

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
