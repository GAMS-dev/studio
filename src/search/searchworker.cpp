/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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

#include "file/filemeta.h"

namespace gams {
namespace studio {
namespace search {

SearchWorker::SearchWorker(QMutex& mutex, QList<FileMeta*> fml, SearchResultList* list)
    : mMutex(mutex), mFiles(fml), mMatches(list)
{
}

SearchWorker::~SearchWorker()
{
}

void SearchWorker::findInFiles()
{
    QMutexLocker m(&mMutex);
    QList<Result> res;
    for (FileMeta* fm : mFiles) {
        int lineCounter = 0;
        QFile file(fm->location());
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            in.setCodec(fm->codec());

            while (!in.atEnd()) { // read file

                lineCounter++;
                if (lineCounter % 500 == 0 && thread()->isInterruptionRequested()) break;

                QString line = in.readLine();

                QRegularExpressionMatch match;
                QRegularExpressionMatchIterator i = mMatches->searchRegex().globalMatch(line);
                while (i.hasNext()) {
                    match = i.next();
                    // abort: too many results
                    if (mMatches->size() > MAX_SEARCH_RESULTS-1) break;
                    mMatches->addResult(lineCounter, match.capturedStart(), match.capturedLength(),
                                       file.fileName(), line.trimmed());
                }

                // update periodically
                if (lineCounter % 250000 == 0)
                    emit update();
            }
            file.close();
        }
        emit update();
    }
    emit resultReady();
    thread()->quit();
}

}
}
}
