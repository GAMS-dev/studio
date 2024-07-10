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
#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <QMutex>
#include <QObject>
#include <QRegularExpression>
#include <QPoint>
#include "result.h"
#include "searchhelpers.h"

namespace gams {
namespace studio {

class FileMeta;

namespace search {

class SearchResultModel;
class SearchWorker : public QObject
{
    Q_OBJECT
public:
    SearchWorker(const SearchFile &file, const QRegularExpression &regex, QPoint from, QPoint to, QList<Result> *list, bool showResults);
    SearchWorker(const QList<SearchFile> &fml, const QRegularExpression &regex, QList<Result> *list, bool showResults);
    void findInFiles();

signals:
    void update(int hits);
    void showResults(bool showResults, QList<gams::studio::search::Result> *results);

private:
    QList<SearchFile> mFiles;
    QList<Result>* mMatches;
    QRegularExpression mRegex;
    QPoint mFrom = QPoint(0,0);
    QPoint mTo = QPoint(0,0);
    bool mFindInSelection = true;
    bool mShowResults = false;

    bool allowInsert(int line, int col);
};

}
}
}

#endif // SEARCHWORKER_H
