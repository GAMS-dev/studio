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
#include "searchresultlist.h"
#include "searchworker.h"

#include <QFile>

#include <file/filemeta.h>

namespace gams {
namespace studio {

SearchWorker::SearchWorker(QRegularExpression regex, FileMeta* fm)
    : mRegex(regex), mFm(fm)
{
    mMatches = new SearchResultList;
}

SearchWorker::~SearchWorker()
{
//    delete mMatches;
}

void SearchWorker::search()
{
    int lineCounter = 0;
    QFile file(mFm->location());
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) { // read file
            lineCounter++;

            if (lineCounter % 1000 == 0 && thread()->isInterruptionRequested()) break;

            QString line = in.readLine();

            QRegularExpressionMatch match;
            QRegularExpressionMatchIterator i = mRegex.globalMatch(line);
            while (i.hasNext()) {
                match = i.next();
                mMatches->addResult(lineCounter, match.capturedStart(), match.capturedLength(),
                                   file.fileName(), line.trimmed());
            }

            // abort: too many results
            if (mMatches->size() > 9999) break;

            // update periodically
            if (lineCounter % 10000 == 0) {
                emit update(mMatches);
            }


        }
        file.close();
    }
    emit resultReady(mMatches);

    thread()->quit();
}

}
}
