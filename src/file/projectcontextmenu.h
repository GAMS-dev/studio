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
#ifndef PROJECTCONTEXTMENU_H
#define PROJECTCONTEXTMENU_H

#include <QMenu>
#include <QSignalMapper>
#include "common.h"

namespace gams {
namespace studio {

class PExAbstractNode;
class PExProjectNode;
class PExFileNode;

class ProjectContextMenu : public QMenu
{
    Q_OBJECT

public:
    ProjectContextMenu();
    void setNodes(QVector<PExAbstractNode*> selected);
    void setParent(QWidget *parent);

signals:
    void closeProject(PExProjectNode* project);
    void project(PExProjectNode* group);
    void runFile(PExFileNode *fc);
    void setMainFile(PExFileNode *fc);
    void closeFile(PExFileNode* fc);
    void addExistingFile(PExProjectNode* group, const QString& file);
    void getSourcePath(QString& source);
    void openLogFor(PExAbstractNode* node, bool openOutput, bool createMissing);
    void openFilePath(QString fileName, PExProjectNode* knownProject, OpenGroupOption opt, bool focus);
    void openFile(PExFileNode* node, bool focus = true, int codecMib = -1, bool forcedAstextEditor = false,
                  NewTabStrategy tabStrategy = tabAfterCurrent);
    void reOpenFile(PExFileNode* node, bool focus = true, int codecMib = -1, bool forcedAsTextEditor = false);
    void exportProject(PExProjectNode* group);
    void importProject();
    void newProject();
    void selectAll();
    void expandAll();
    void collapseAll();
    void newFileDialog(QVector<PExProjectNode *> groups, const QString& solverName="", bool projectOnly = false);
    void openTerminal(const QString& workingDir);
    void openGdxDiffDialog(QString workingDirectory, QString input1, QString input2="");

private slots:
    void onCloseGroup();
    void onCloseProject();
    void onCloseFile();
    void onAddExisitingFile();
    void onAddNewFile();
    void onSetMainFile();
    void onExportProject();
    void onSelectAll();
    void onExpandAll();
    void onCollapseAll();
    void onAddNewSolverOptionFile(const QString &solverName);
    void onOpenTerminal();
    void onGdxDiff();
    void onOpenFileLoc();
    void onOpenFile();
    void onOpenFileAsText();
    void onReOpenFile();
    void onReOpenFileAsText();
    void onOpenLog();
    void onOpenEfi();

private:
    QString getEfiName(PExProjectNode *project) const;

private:
    QVector<PExAbstractNode*> mNodes;
    QHash<int, QAction*> mActions;
    QStringList mAvailableSolvers;
    QHash<int, QAction*> mSolverOptionActions;
    QWidget *mParent = nullptr;

    // Strings
    QString mTxtCloseGroup = "Close files in this &folder";
    QString mTxtCloseProject = "Close &project";
    QString mTxtCloseFile = "Close &file";
};

} // namespace studio
} // namespace gams

#endif // PROJECTCONTEXTMENU_H

