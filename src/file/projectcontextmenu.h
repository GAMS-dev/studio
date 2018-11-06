/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef PROJECTCONTEXTMENU_H
#define PROJECTCONTEXTMENU_H

#include <QMenu>

namespace gams {
namespace studio {

class ProjectAbstractNode;
class ProjectGroupNode;
class ProjectFileNode;

class ProjectContextMenu : public QMenu
{
    Q_OBJECT

public:
    ProjectContextMenu();
    bool setNodes(ProjectAbstractNode* current, QVector<ProjectAbstractNode*> selected);

    void setParent(QWidget *parent);

signals:
    void closeGroup(ProjectGroupNode* group);
    void runGroup(ProjectGroupNode* group);
    void runFile(ProjectFileNode *fc);
    void setMainFile(ProjectFileNode *fc);
    void closeFile(ProjectFileNode* fc);
    void addExistingFile(ProjectGroupNode* group, const QString& file);
    void getSourcePath(QString& source);
    void openLogFor(ProjectAbstractNode* node, bool createMissing);
    void renameGroup(ProjectGroupNode* group);

private slots:
    void onCloseGroup();
    void onCloseFile();
    void onAddExisitingFile();
    void onAddNewFile();
    void onSetMainFile();
    void onRenameGroup();

private:
    void onOpenFileLoc();
    void onOpenLog();

private:
    QVector<ProjectAbstractNode*> mNodes;
    QHash<int, QAction*> mActions;
    QWidget *mParent = nullptr;
};

} // namespace studio
} // namespace gams

#endif // PROJECTCONTEXTMENU_H
