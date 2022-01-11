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
 */
#ifndef TESTFILEHANDLER_H
#define TESTFILEHANDLER_H

#include "search/abstractsearchfilehandler.h"

namespace gams {
namespace studio {
namespace search {

class TestFileHandler : public AbstractSearchFileHandler
{
public:
    TestFileHandler();

    FileMeta* fileMeta(QWidget* widget);
    FileMeta* fileMeta(FileId fileId);
    QList<FileMeta*> fileMetas();
    QList<FileMeta*> openFiles();
    PExFileNode* fileNode(QWidget* widget);
    PExFileNode* findFile(QString filepath);

private:
    FileMetaRepo* mFileRepo;

    FileMeta* mGms;
    FileMeta* mLst;

    QList<FileMeta*> mGmsList;
    QList<FileMeta*> mLstList;
    QList<FileMeta*> mMixedList;

    PExFileNode* mFileNode;
};

}
}
}
#endif // TESTFILEHANDLER_H
