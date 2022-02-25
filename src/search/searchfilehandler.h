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
#ifndef SEARCHFILEHANDLER_H
#define SEARCHFILEHANDLER_H

#include "search/abstractsearchfilehandler.h"
#include "mainwindow.h"

class MainWindow;
namespace gams {
namespace studio {
namespace search {

class SearchFileHandler : public AbstractSearchFileHandler
{
public:
    SearchFileHandler(MainWindow* main);

    FileMeta* fileMeta(QWidget* widget) override;
    FileMeta* fileMeta(FileId fileId) override;
    QList<FileMeta*> fileMetas() override;
    QList<FileMeta*> openFiles() override;
    PExFileNode* fileNode(NodeId nodeId) override;
    PExFileNode* fileNode(QWidget* widget) override;
    PExFileNode* findFileNode(QString filepath) override;
    PExProjectNode *createProject(QString name, QString path) override;
    FileMeta* findOrCreateFile(QString filepath) override;

private:
    MainWindow* mMain;

};

}
}
}

#endif // SEARCHFILEHANDLER_H
