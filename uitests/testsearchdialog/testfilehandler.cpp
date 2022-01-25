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
#include "testfilehandler.h"
#include "file/filemetarepo.h"

namespace gams {
namespace studio {
namespace search {

TestFileHandler::TestFileHandler() {
    QFileInfo file("trnsport.gms");

    qDebug() << QTime::currentTime() << file.filePath(); // rogo: delete

//    mFileRepo = new FileMetaRepo(nullptr);
//    mGms = mFileRepo->findOrCreateFileMeta()
//    mLst;

//    mGmsList;
//    mLstList;
//    mMixedList;

//    mFileNode;
}

FileMeta* TestFileHandler::fileMeta(QWidget* widget)
{
    return mGms;
}

FileMeta* TestFileHandler::fileMeta(FileId fileId)
{
    return mGms;
}

QList<FileMeta*> TestFileHandler::fileMetas()
{
    return mMixedList;
}

QList<FileMeta*> TestFileHandler::openFiles()
{
    return mMixedList;
}

PExFileNode* TestFileHandler::fileNode(QWidget* widget)
{
    return mFileNode;
}

PExFileNode* TestFileHandler::findFile(QString filepath)
{
    return mFileNode;
}

}
}
}
