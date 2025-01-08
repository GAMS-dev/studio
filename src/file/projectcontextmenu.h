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
#ifndef PROJECTCONTEXTMENU_H
#define PROJECTCONTEXTMENU_H

#include <QMenu>
#include <QSignalMapper>
#include "common.h"

namespace gams {
namespace studio {

class PExAbstractNode;
class PExGroupNode;
class PExProjectNode;
class PExFileNode;

class ProjectContextMenu : public QMenu
{
    Q_OBJECT

public:
    ProjectContextMenu();
    void initialize(const QVector<PExAbstractNode*> &selected, PExProjectNode *focussedProject);
    void setParent(QWidget *parent);
    static bool allowChange(const QList<PExAbstractNode *> nodes);

signals:
    void closeProject(gams::studio::PExProjectNode* project);
    void project(gams::studio::PExProjectNode* group);
    void runFile(gams::studio::PExFileNode *fc);
    void setMainFile(gams::studio::PExFileNode *fc);
    void closeFile(gams::studio::PExFileNode* fc);
    void closeDelFiles(QList<gams::studio::PExFileNode*> fileNodes);
    void addExistingFile(gams::studio::PExGroupNode* group, const QString& file);
    void getSourcePath(QString& source);
    void openLogFor(gams::studio::PExAbstractNode* node, bool openOutput, bool createMissing);
    void openFilePath(QString fileName, gams::studio::PExProjectNode* knownProject, gams::studio::OpenGroupOption opt, bool focus);
    void openFile(gams::studio::PExFileNode* node, bool focus = true, int codecMib = -1, bool forcedAstextEditor = false,
                  gams::studio::NewTabStrategy tabStrategy = tabAfterCurrent);
    void reOpenFile(gams::studio::PExFileNode* node, bool focus = true, int codecMib = -1, bool forcedAstextEditor = false);
    void moveProject(gams::studio::PExProjectNode* project, bool fullCopy);
    void focusProject(gams::studio::PExProjectNode* project);
    void newProject();
    void openProject();
    void selectAll();
    void expandAll();
    void collapseAll();
    void newFileDialog(QVector<gams::studio::PExProjectNode *> projects, const QString &path = "", const QString& solverName="",
                       gams::studio::FileKind projectOnly = gams::studio::FileKind::None);
    void openTerminal(const QString& workingDir);
    void openGdxDiffDialog(QString workingDirectory, QString input1, QString input2="");
    void resetGdxStates(const QStringList &files);

private slots:
    void onCloseGroup();
    void onCloseProject();
    void onCloseDelProject();
    void onCloseFile();
    void onCloseDelFile();
    void onAddExisitingFile();
    void onAddNewFile();
    void onAddNewPfFile();
    void onSetMainFile();
    void onMoveProject();
    void onCopyProject();
    void onFocusProject();
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
    QString mTxtCloseGroup = "Close Files in This &Folder";
    QString mTxtCloseProject = "Close &Project";
    QString mTxtCloseDelProject = "&Delete Project File";
    QString mTxtCloseFile = "Close &File";
    QString mTxtCloseDelFile = "&Delete File";
};

} // namespace studio
} // namespace gams

#endif // PROJECTCONTEXTMENU_H

