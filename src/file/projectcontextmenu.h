/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
    void closeProject(gams::studio::PExProjectNode* project);
    void project(gams::studio::PExProjectNode* group);
    void runFile(gams::studio::PExFileNode *fc);
    void setMainFile(gams::studio::PExFileNode *fc);
    void closeFile(gams::studio::PExFileNode* fc);
    void addExistingFile(gams::studio::PExProjectNode* group, const QString& file);
    void getSourcePath(QString& source);
    void openLogFor(gams::studio::PExAbstractNode* node, bool openOutput, bool createMissing);
    void openFilePath(QString fileName, gams::studio::PExProjectNode* knownProject, gams::studio::OpenGroupOption opt, bool focus);
    void openFile(gams::studio::PExFileNode* node, bool focus = true, int codecMib = -1, bool forcedAstextEditor = false,
                  gams::studio::NewTabStrategy tabStrategy = tabAfterCurrent);
    void reOpenFile(gams::studio::PExFileNode* node, bool focus = true, int codecMib = -1, bool forcedAstextEditor = false);
    void moveProject(gams::studio::PExProjectNode* project, bool fullCopy);
    void newProject();
    void openProject();
    void selectAll();
    void expandAll();
    void collapseAll();
    void newFileDialog(QVector<gams::studio::PExProjectNode *> groups, const QString& solverName="", bool projectOnly = false);
    void openTerminal(const QString& workingDir);
    void openGdxDiffDialog(QString workingDirectory, QString input1, QString input2="");
    void resetGdxStates(const QStringList &files);

private slots:
    void onCloseGroup();
    void onCloseProject();
    void onCloseFile();
    void onAddExisitingFile();
    void onAddNewFile();
    void onSetMainFile();
    void onMoveProject();
    void onCopyProject();
    void onSelectAll();
    void onExpandAll();
    void onCollapseAll();
    void onAddNewSolverOptionFile(const QString &solverName);
    void onOpenTerminal();
    void onGdxDiff();
    void onGdxReset();
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

