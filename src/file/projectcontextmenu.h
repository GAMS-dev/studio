/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include <QSignalMapper>

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
    void setNodes(QVector<ProjectAbstractNode*> selected);
    void setParent(QWidget *parent);

signals:
    void closeGroup(ProjectGroupNode* group);
    void runGroup(ProjectGroupNode* group);
    void runFile(ProjectFileNode *fc);
    void setMainFile(ProjectFileNode *fc);
    void closeFile(ProjectFileNode* fc);
    void addExistingFile(ProjectGroupNode* group, const QString& file);
    void getSourcePath(QString& source);
    void openLogFor(ProjectAbstractNode* node, bool openOutput, bool createMissing);
    void renameGroup(ProjectGroupNode* group);
    void openFile(ProjectFileNode* node, bool focus = true, int codecMib = -1, bool forcedAstextEditor = false);
    void reOpenFile(ProjectFileNode* node, bool focus = true, int codecMib = -1, bool forcedAstextEditor = false);
    void selectAll();
    void expandAll();
    void collapseAll();
    void newFileDialog(QVector<ProjectGroupNode *> groups, const QString& solverName="");
    void openTerminal(const QString& workingDir);

private slots:
    void onCloseGroup();
    void onCloseFile();
    void onAddExisitingFile();
    void onAddNewFile();
    void onSetMainFile();
    void onRenameGroup();
    void onSelectAll();
    void onExpandAll();
    void onCollapseAll();
    void onAddNewSolverOptionFile(const QString &solverName);
    void onOpenTerminal();

private:
    void onOpenFileLoc();
    void onOpenFile();
    void onOpenFileAsText();
    void onReOpenSolverOptionFile();
    void onReOpenSolverOptionFileAsText();
    void onOpenLog();

private:
    QVector<ProjectAbstractNode*> mNodes;
    QHash<int, QAction*> mActions;
    QStringList mAvailableSolvers;
    QHash<int, QAction*> mSolverOptionActions;
    QWidget *mParent = nullptr;

    // Strings
    QString mTxtCloseGroup = "Close &group";
    QString mTxtCloseFile = "Close &file";
};

} // namespace studio
} // namespace gams

#endif // PROJECTCONTEXTMENU_H

