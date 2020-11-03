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
#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <QMutex>
#include <QObject>
#include <QRegularExpression>

namespace gams {
namespace studio {

class FileMeta;

namespace search {

class SearchResultList;
class SearchWorker : public QObject
{
    Q_OBJECT
public:
    SearchWorker(QList<FileMeta*> fml, SearchResultList* list);
    ~SearchWorker();
    void findInFiles();

signals:
    void update();
    void resultReady();

private:
    QList<FileMeta*> mFiles;
    SearchResultList* mMatches;
};

}
}
}

#endif // SEARCHWORKER_H
