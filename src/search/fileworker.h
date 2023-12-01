/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#ifndef FILEWORKER_H
#define FILEWORKER_H

#include <QObject>
#include "searchhelpers.h"

namespace gams {
namespace studio {
namespace search {

class FileWorker : public QObject
{
    Q_OBJECT

public:
    FileWorker(SearchParameters parameters);
    QList<SearchFile> collectFilesInFolder();

signals:
    void filesCollected(QList<SearchFile> files);

private:
    SearchParameters mParameters;


};

}
}
}

#endif // FILEWORKER_H
