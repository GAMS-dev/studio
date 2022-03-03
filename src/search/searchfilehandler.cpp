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
#include "searchfilehandler.h"

namespace gams {
namespace studio {
namespace search {

SearchFileHandler::SearchFileHandler(MainWindow *main) : mMain(main)
{ }

FileMeta* SearchFileHandler::fileMeta(QWidget *widget)
{
    return mMain->fileRepo()->fileMeta(widget);
}

FileMeta* SearchFileHandler::fileMeta(FileId fileId)
{
    return mMain->fileRepo()->fileMeta(fileId);
}

QSet<FileMeta *> SearchFileHandler::fileMetas()
{
    QList<FileMeta*> fileMetas = mMain->fileRepo()->fileMetas();
    return QSet<FileMeta*>(fileMetas.begin(), fileMetas.end());
}

QSet<FileMeta*> SearchFileHandler::openFiles()
{
    QList<FileMeta*> openFiles = mMain->fileRepo()->fileMetas();
    return QSet<FileMeta*>(openFiles.begin(), openFiles.end());
}

PExFileNode* SearchFileHandler::fileNode(NodeId nodeId)
{
    return mMain->projectRepo()->asFileNode(nodeId);
}

PExFileNode* SearchFileHandler::fileNode(QWidget *widget)
{
    return mMain->projectRepo()->findFileNode(widget);
}

PExFileNode* SearchFileHandler::findFileNode(QString filepath)
{
    return mMain->projectRepo()->findOrCreateFileNode(filepath);
}

PExProjectNode* SearchFileHandler::createProject(QString name, QString path)
{
    return mMain->projectRepo()->createProject(name, path, "");
}

FileMeta* SearchFileHandler::findOrCreateFile(QString filepath)
{
    return mMain->fileRepo()->findOrCreateFileMeta(filepath);
}

PExFileNode* SearchFileHandler::openFile(QString fileName) {
    return mMain->openFileWithOption(fileName);
}

}
}
}
