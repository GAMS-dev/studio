/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#ifndef ABSTRACTSEARCHFILEHANDLER_H
#define ABSTRACTSEARCHFILEHANDLER_H

#include <QList>
#include <QWidget>
#include "file/filemeta.h"
#include "file/pexfilenode.h"

namespace gams {
namespace studio {
namespace search {

class AbstractSearchFileHandler
{
public:
    virtual FileMeta* fileMeta(QWidget* widget) = 0;
    virtual FileMeta* fileMeta(FileId fileId) = 0;
    virtual QList<FileMeta*> fileMetas() = 0;
    virtual QList<FileMeta*> openFiles() = 0;
    virtual PExFileNode* fileNode(NodeId nodeId) = 0;
    virtual PExFileNode* fileNode(QWidget* widget) = 0;
    virtual PExFileNode* findFile(QString filepath) = 0;
    virtual FileMeta* findOrCreateFile(QString filepath) = 0;
};

}
}
}

#endif // ABSTRACTSEARCHFILEHANDLER_H
