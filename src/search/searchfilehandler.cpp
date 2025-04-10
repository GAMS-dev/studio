/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
    auto openFiles = mMain->fileRepo()->openFiles();
    return QSet<FileMeta*>(openFiles.begin(), openFiles.end());
}

PExFileNode* SearchFileHandler::fileNode(QWidget *widget)
{
    return mMain->projectRepo()->findFileNode(widget);
}

PExFileNode* SearchFileHandler::findFileNode(QString filepath)
{
    return mMain->projectRepo()->findFile(filepath);
}

PExProjectNode* SearchFileHandler::createProject(QString name, QString path)
{
    return mMain->projectRepo()->createProject(name, path, "", onExist_AddNr, "", PExProjectNode::tSearch);
}

PExProjectNode *SearchFileHandler::findProject(QWidget *widget)
{
    return mMain->projectRepo()->findProject(widget);
}

FileMeta* SearchFileHandler::findFile(QString filepath)
{
    return mMain->fileRepo()->fileMeta(filepath);
}

PExFileNode* SearchFileHandler::openFile(QString fileName,  PExProjectNode* knownProject) {
    return mMain->openFilePath(fileName, knownProject);
}

QVector<PExProjectNode *> SearchFileHandler::projects()
{
    return mMain->projectRepo()->projects();
}

}
}
}
