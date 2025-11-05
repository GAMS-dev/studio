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
#ifndef SEARCHFILEHANDLER_H
#define SEARCHFILEHANDLER_H

#include "search/abstractsearchfilehandler.h"
#include "mainwindow.h"

namespace gams {
namespace studio {
namespace search {

class SearchFileHandler : public AbstractSearchFileHandler
{
public:
    SearchFileHandler(MainWindow* main);

    FileMeta* fileMeta(QWidget* widget) override;
    FileMeta* fileMeta(FileId fileId) override;
    QSet<FileMeta*> fileMetas() override;
    QSet<FileMeta*> openFiles() override;
    PExFileNode* fileNode(QWidget* widget) override;
    PExFileNode* findFileNode(QString filepath) override;
    PExProjectNode* createProject(QString name, QString path) override;
    PExProjectNode* findProject(QWidget* widget) override;


    FileMeta* findFile(QString filepath) override;
    PExFileNode *openFile(QString fileName, PExProjectNode *knownProject) override;
    QVector<PExProjectNode*> projects() override;

private:
    MainWindow* mMain;
};

}
}
}

#endif // SEARCHFILEHANDLER_H
