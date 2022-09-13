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
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopServices>
#include "projectcontextmenu.h"
#include "file.h"
#include "commonpaths.h"
#include "viewhelper.h"
#include "support/solverconfiginfo.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

namespace gams {
namespace studio {

enum ContextAction {
    actOpen,
    actOpenAsText,
    actReOpen,
    actReOpenAsText,
    actSep1,
    actExplorer,
    actLogTab,
    actOpenEfi,
    actSep2,
    actSetMain,
    actSep3,
    actAddExisting,
    actSep4,
    actAddNewGms,
    actAddNewOpt,
    actSep5,
    actCloseProject,
    actCloseGroup,
    actCloseFile,
    actSep6,
    actSelectAll,
    actExpandAll,
    actCollapseAll,
    actOpenTerminal,
    actGdxDiff,
    actSep7,
    actProjExport,
    actProjImport,
    actNewProject,
};

ProjectContextMenu::ProjectContextMenu()
{
    mActions.insert(actOpen, addAction("&Open File", this, &ProjectContextMenu::onOpenFile));
    mActions.insert(actOpenAsText, addAction("&Open File As Text", this, &ProjectContextMenu::onOpenFileAsText));
    mActions.insert(actReOpen, addAction("&Reopen File using Editor", this, &ProjectContextMenu::onReOpenFile));
    mActions.insert(actReOpenAsText, addAction("Reopen File as Text", this, &ProjectContextMenu::onReOpenFileAsText));

    mActions.insert(actSep1, addSeparator());

    mActions.insert(actExplorer, addAction("&Open location", this, &ProjectContextMenu::onOpenFileLoc));
    mActions.insert(actOpenTerminal, addAction("&Open terminal", this, &ProjectContextMenu::onOpenTerminal));
    mActions.insert(actGdxDiff, addAction("&Open in GDX Diff", this, &ProjectContextMenu::onGdxDiff));
    mActions.insert(actLogTab, addAction("&Open log tab", this, &ProjectContextMenu::onOpenLog));
    mActions.insert(actOpenEfi, addAction("Create &EFI file", this, &ProjectContextMenu::onOpenEfi));

    mActions.insert(actSep2, addSeparator());
    mActions.insert(actSetMain, addAction("&Set as main file", this, &ProjectContextMenu::onSetMainFile));

    mActions.insert(actSep3, addSeparator());

    mActions.insert(actAddExisting, addAction("Add &existing file", this, &ProjectContextMenu::onAddExisitingFile));

    mActions.insert(actAddNewGms, addAction("Add &new file", this, &ProjectContextMenu::onAddNewFile));

    QMenu* newSolverOptionMenu = addMenu( "Add new solver option file" );
    mActions.insert(actAddNewOpt, newSolverOptionMenu->menuAction());
    int addNewSolverOptActionBaseIndex = actAddNewOpt*1000;

    QDir sysdir(CommonPaths::systemDir());
    QStringList optFiles = sysdir.entryList(QStringList() << "opt*.def" , QDir::Files);

    try {
        support::SolverConfigInfo solverInfo;
        QMap<QString, QString> solverDefFileNames = solverInfo.solverOptDefFileNames();

        if (solverDefFileNames.size()>0) { // when solver definition file information is available
            for (QString solvername : solverDefFileNames.keys()) {
                if (optFiles.contains(solverDefFileNames.value(solvername))) { //there exists such a file
                    QAction* createSolverOption = newSolverOptionMenu->addAction(solvername.toLower());
                    connect(createSolverOption, &QAction::triggered, this, [=] { onAddNewSolverOptionFile(solvername.toLower()); });

                    mAvailableSolvers << solvername;
                    mSolverOptionActions.insert(++addNewSolverOptActionBaseIndex, createSolverOption);
                }
            }
        } else { // when no information on solver option definition file names
           for (QString &filename : optFiles) {
               QString solvername = filename.mid(QString("opt").length());
               solvername.replace(QRegExp(".def"), "");
               if (QString::compare("gams", solvername ,Qt::CaseInsensitive)==0)
                   continue;
               QAction* createSolverOption = newSolverOptionMenu->addAction(solvername);
               connect(createSolverOption, &QAction::triggered, this, [=] { onAddNewSolverOptionFile(solvername); });

               mAvailableSolvers << solvername;
               mSolverOptionActions.insert(++addNewSolverOptActionBaseIndex, createSolverOption);
           }
        }
    } catch (...) {
        // The DistributionValidator as well as the Help View already print an error message,
        // so we just make sure that there is no issue if GAMS is not found.
    }

    mActions.insert(actSep5, addSeparator());
    mActions.insert(actSelectAll, addAction("Select &all", this, &ProjectContextMenu::onSelectAll));
    mActions.insert(actCollapseAll, addAction("Collapse all projects", this, &ProjectContextMenu::onCollapseAll));
    mActions.insert(actExpandAll, addAction("Expand all", this, &ProjectContextMenu::onExpandAll));

    mActions.insert(actSep6, addSeparator());
    mActions.insert(actProjExport, addAction("&Export project",  this, &ProjectContextMenu::onExportProject));
    mActions.insert(actProjImport, addAction("&Import project",  this, &ProjectContextMenu::importProject));
    mActions.insert(actNewProject, addAction("&New project",  this, &ProjectContextMenu::newProject));

    mActions.insert(actSep7, addSeparator());
    mActions.insert(actCloseProject, addAction(mTxtCloseProject, this, &ProjectContextMenu::onCloseProject));
    mActions.insert(actCloseGroup, addAction(mTxtCloseProject, this, &ProjectContextMenu::onCloseGroup));
    mActions.insert(actCloseFile, addAction(mTxtCloseFile, this, &ProjectContextMenu::onCloseFile));

}

void ProjectContextMenu::setNodes(QVector<PExAbstractNode *> selected)
{
    // synchronize current and selected
    mNodes.clear();
    mNodes = selected;
    bool single = mNodes.count() == 1;
    bool isProject = mNodes.size() ? bool(mNodes.first()->toProject()) : false;
    bool isGroup = mNodes.size() ? bool(mNodes.first()->toGroup()) && !isProject : false;
    PExProjectNode *project = mNodes.size() ? mNodes.first()->assignedProject() : nullptr;
    bool canExportProject = project && project->childCount();
    bool isProjectEfi = false;
    for (PExAbstractNode *node: qAsConst(mNodes)) {
        if (!canExportProject) break;
        if (node->assignedProject() != project)
            canExportProject = false;
    }
    if (isProject && single) {
        PExProjectNode *project = mNodes.first()->toProject();
        QString efi = getEfiName(project);
        isProjectEfi = !efi.isEmpty();
        if (isProjectEfi && QFileInfo::exists(efi))
            mActions[actOpenEfi]->setText("Open &EFI file");
    }

    PExFileNode *fileNode = mNodes.size() ? mNodes.first()->toFile() : nullptr;
    bool isFreeSpace = !fileNode && !isProject;
    bool isGmsFile = fileNode && fileNode->file()->kind() == FileKind::Gms;
    bool isRunnable = false;
    bool isOpen = fileNode && fileNode->file()->isOpen();
    bool isOpenable = fileNode && !fileNode->file()->isOpen();
    bool isOptFile = fileNode && fileNode->file()->kind() == FileKind::Opt;
    bool isGucFile = fileNode && fileNode->file()->kind() == FileKind::Guc;
    bool isEfiFile = fileNode && fileNode->file()->kind() == FileKind::Efi;
    bool isOpenableAsText = isOpenable && (isOptFile || isGucFile);
    bool isOpenWithSolverOptionEditor = false;
    bool isOpenWithGamsUserConfigEditor = false;
    bool isOpenWithEfiEditor = false;
    if (fileNode) {
        for (QWidget *e : fileNode->file()->editors()) {
            if (ViewHelper::toSolverOptionEdit(e))
                isOpenWithSolverOptionEditor = true;
            else if (ViewHelper::toGamsConfigEditor(e))
                isOpenWithGamsUserConfigEditor = true;
            else if (ViewHelper::toEfiEditor(e))
                isOpenWithEfiEditor = true;
        }
    }
    bool isReOpenableAsText = isOpen && (isOpenWithSolverOptionEditor || isOpenWithGamsUserConfigEditor || isOpenWithEfiEditor);

    bool isReOpenableWithSolverOptionEditor = isOpen && isOptFile && !isOpenWithSolverOptionEditor;
    bool isReOpenableWithGamsUserConfigEditor = isOpen && isGucFile && !isOpenWithGamsUserConfigEditor;
    bool isReOpenableWithEfiEditor = isOpen && isEfiFile && !isOpenWithEfiEditor;
    bool isReOpenable = isReOpenableWithSolverOptionEditor || isReOpenableWithGamsUserConfigEditor || isReOpenableWithEfiEditor;

    // opening GDX diff is only possible for one or two selected GDX files
    bool isOpenableWithGdxDiff = false;
    if (mNodes.count() == 1 || mNodes.count() == 2) {
        PExFileNode *fn = mNodes.first()->toFile();
        isOpenableWithGdxDiff = fn && fn->file()->kind() == FileKind::Gdx;
        if (mNodes.count() == 2) {
            fn = mNodes.last()->toFile();
            isOpenableWithGdxDiff = isOpenableWithGdxDiff && fn && fn->file()->kind() == FileKind::Gdx;
        }
    }

    QString file;
    if (fileNode && fileNode->assignedProject()) {
        file = fileNode->assignedProject()->parameter("gms");
        isRunnable = fileNode->location() == file;
    }

    mActions[actExplorer]->setEnabled(single);
    mActions[actExplorer]->setVisible(!isFreeSpace);
    mActions[actOpenTerminal]->setVisible(!isFreeSpace);

    mActions[actGdxDiff]->setEnabled(isOpenableWithGdxDiff);
    mActions[actGdxDiff]->setVisible(isOpenableWithGdxDiff);

    mActions[actOpen]->setEnabled(isOpenable);
    mActions[actOpen]->setVisible(isOpenable);
    mActions[actOpenAsText]->setEnabled(isOpenableAsText);
    mActions[actOpenAsText]->setVisible(isOpenableAsText);

    if (isReOpenableWithGamsUserConfigEditor)
        mActions[actReOpen]->setText( "&Reopen File using Gams User Configuration Editor" );
    else if (isReOpenableWithSolverOptionEditor)
        mActions[actReOpen]->setText( "&Reopen File using Solver Option Editor" );
    else
        mActions[actReOpen]->setText( "&Reopen File using EFI Editor" );
    mActions[actReOpen]->setEnabled(isReOpenable);
    mActions[actReOpen]->setVisible(isReOpenable);
    mActions[actReOpenAsText]->setEnabled(isReOpenableAsText);
    mActions[actReOpenAsText]->setVisible(isReOpenableAsText);

    mActions[actLogTab]->setVisible(isProject);
    mActions[actLogTab]->setEnabled(single);

    mActions[actOpenEfi]->setVisible(isProject);
    mActions[actOpenEfi]->setEnabled(isProjectEfi);

    mActions[actProjExport]->setVisible(!isFreeSpace);
    mActions[actProjExport]->setEnabled(canExportProject);

    mActions[actSep1]->setVisible(isProject);
    mActions[actSetMain]->setVisible(isGmsFile && !isRunnable && single);
//    mActions[actSetMain]->setEnabled(single);

    mActions[actAddNewGms]->setVisible(isProject);
    mActions[actAddExisting]->setVisible(isProject);

    mActions[actCloseProject]->setVisible(isProject);
    mActions[actCloseGroup]->setVisible(isGroup);
    mActions[actCloseFile]->setVisible(fileNode);

    if (!single) {
        mActions[actCloseProject]->setText(mTxtCloseProject + "s");
        mActions[actCloseGroup]->setText(mTxtCloseGroup + "s");
        mActions[actCloseFile]->setText(mTxtCloseFile + "s");
    } else {
        mActions[actCloseProject]->setText(mTxtCloseProject);
        mActions[actCloseGroup]->setText(mTxtCloseGroup);
        mActions[actCloseFile]->setText(mTxtCloseFile);
    }

    // create solver option files
    mActions[actSep3]->setVisible(isProject);
    mActions[actAddNewOpt]->setVisible(isProject);
    for (QAction* action : qAsConst(mSolverOptionActions))
        action->setVisible(isProject);
}

void ProjectContextMenu::onCloseFile()
{
    for (PExAbstractNode *node: qAsConst(mNodes)) {
        PExFileNode *file = node->toFile();
        if (file) emit closeFile(file);
    }
}

void ProjectContextMenu::onAddExisitingFile()
{
    QVector<PExProjectNode*> projects;
    for (PExAbstractNode *node: qAsConst(mNodes)) {
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (!projects.contains(project))
            projects << project;
    }

    QString sourcePath = "";
    if (!projects.isEmpty()) sourcePath = projects.first()->location();
    else emit getSourcePath(sourcePath);
    QStringList filePaths = QFileDialog::getOpenFileNames(mParent, "Add existing files", sourcePath,
                                                    ViewHelper::dialogFileFilterAll().join(";;"),
                                                    nullptr,
                                                    DONT_RESOLVE_SYMLINKS_ON_MACOS);
    if (filePaths.isEmpty()) return;

    for (PExProjectNode *project: qAsConst(projects)) {
        for (const QString &filePath: qAsConst(filePaths)) {
            emit addExistingFile(project, filePath);
        }
    }
}

void ProjectContextMenu::onAddNewFile()
{
    QVector<PExProjectNode*> projects;
    for (PExAbstractNode *node: qAsConst(mNodes)) {
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (!projects.contains(project))
            projects << project;
    }
    emit newFileDialog(projects);
}

void ProjectContextMenu::setParent(QWidget *parent)
{
    mParent = parent;
}

void ProjectContextMenu::onCloseGroup()
{
    for (PExAbstractNode *node: qAsConst(mNodes)) {
        PExGroupNode *group = node->toGroup();
        if (!group) continue;
        QVector<PExFileNode*> files = group->listFiles();
        for (PExFileNode* file : qAsConst(files)) {
            emit closeFile(file);
        }
    }
}

void ProjectContextMenu::onCloseProject()
{
    for (PExAbstractNode *node: qAsConst(mNodes)) {
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (project) emit closeProject(project);
    }
}

void ProjectContextMenu::onSetMainFile()
{
    PExFileNode *file = mNodes.first()->toFile();
    if (file) emit setMainFile(file);
}

void ProjectContextMenu::onExportProject()
{
    PExProjectNode *project = mNodes.first()->assignedProject();
    emit exportProject(project);
}

void ProjectContextMenu::onAddNewSolverOptionFile(const QString &solverName)
{
    QVector<PExProjectNode*> groups;
    for (PExAbstractNode *node: qAsConst(mNodes)) {
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (!groups.contains(project))
            groups << project;
    }

    emit newFileDialog(groups, solverName);
}

void ProjectContextMenu::onOpenTerminal()
{
    QString baseDir;
    PExFileNode *file = mNodes.first()->toFile();
    if (file) {
        baseDir = QFileInfo(file->location()).path();
    } else if ((mNodes.first()->type() == NodeType::group) || (mNodes.first()->type() == NodeType::project)) {
        PExGroupNode *group = mNodes.first()->toGroup();
        if (group) baseDir = group->location();
    }
    emit openTerminal(baseDir);
}

void ProjectContextMenu::onGdxDiff()
{
    PExFileNode *file = mNodes.first()->toFile();
    QString workingDir = QFileInfo(file->location()).path();

    if (mNodes.size() == 1)
        emit openGdxDiffDialog(workingDir, mNodes.first()->toFile()->location());
    else if (mNodes.size() == 2)
        emit openGdxDiffDialog(workingDir, mNodes.first()->toFile()->location(), mNodes.last()->toFile()->location());
}

void ProjectContextMenu::onOpenFileLoc()
{
    QString openLoc;
    PExFileNode *file = mNodes.first()->toFile();
    if (file) {
// select file on windows by calling explorer.exe with parameter /select
#ifdef _WIN32
        QString explorerPath = QStandardPaths::findExecutable("explorer.exe");
        if (explorerPath.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(file->location()).path()));
        } else {
            QProcess proc;
            proc.setProgram(explorerPath);
            QStringList args;
            args << "/select";
            args << ",";
            args << QDir::toNativeSeparators(QFileInfo(file->location()).filePath());
            proc.setArguments(args);
            proc.start();
            proc.waitForFinished();
        }
#else
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(file->location()).path()));
#endif
    } else if ((mNodes.first()->type() == NodeType::group) || (mNodes.first()->type() == NodeType::project)){
        PExGroupNode *group = mNodes.first()->toGroup();
        if (group) openLoc = group->location();
        QDesktopServices::openUrl(QUrl::fromLocalFile(openLoc));
    }
}

void ProjectContextMenu::onOpenFile()
{
    PExFileNode *file = mNodes.first()->toFile();
    if (file) emit openFile(file, true, -1, false);
}

void ProjectContextMenu::onOpenFileAsText()
{
    PExFileNode *file = mNodes.first()->toFile();
    if (file) emit openFile(file, true, -1, true);
}

void ProjectContextMenu::onReOpenFile()
{
    PExFileNode *file = mNodes.first()->toFile();
    if (file) emit reOpenFile(file, true, -1, false);
}

void ProjectContextMenu::onReOpenFileAsText()
{
    PExFileNode *file = mNodes.first()->toFile();
    if (file) emit reOpenFile(file, true, -1, true);
}

void ProjectContextMenu::onOpenLog()
{
    if (mNodes.first()) emit openLogFor(mNodes.first(), true, true);
}

void ProjectContextMenu::onOpenEfi()
{
    if (mNodes.first()) {
        PExProjectNode *project = mNodes.first()->toProject();
        QString efi = getEfiName(project);
        if (!efi.isEmpty()) {
            QFile file(efi);
            if (!file.exists()) {
                if (file.open(QFile::WriteOnly | QFile::Text))
                    file.close();
            }
            emit openFilePath(efi, project, ogNone, true);
        }
    }
}

QString ProjectContextMenu::getEfiName(PExProjectNode *project) const
{
    if (!project->runnableGms()) return QString();
    QFileInfo info(project->runnableGms()->location());
    return info.path() + '/' + info.completeBaseName() + ".efi";
}

void ProjectContextMenu::onSelectAll()
{
    emit selectAll();
}

void ProjectContextMenu::onExpandAll()
{
    emit expandAll();
}
void ProjectContextMenu::onCollapseAll()
{
    emit collapseAll();
}
} // namespace studio
} // namespace gams
