/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "editors/systemlogedit.h"
#include "editors/sysloglocator.h"
#include "msgbox.h"

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
    actAddNewFile,
    actAddNewOpt,
    actAddNewPf,
    actSep5,
    actCloseProject,
    actCloseDelProject,
    actCloseGroup,
    actCloseFile,
    actCloseDelFile,
    actSep6,
    actSelectAll,
    actExpandAll,
    actCollapseAll,
    actOpenTerminal,
    actGdxDiff,
    actGdxReset,
    actSep7,
    actProjectNew,
    actProjectOpen,
    actProjectMove,
    actProjectCopy,
    actFocusProject,
    actUnfocusProject,
};

ProjectContextMenu::ProjectContextMenu()
{
    mActions.insert(actOpen, addAction("&Open File", this, &ProjectContextMenu::onOpenFile));
    mActions.insert(actOpenAsText, addAction("&Open File as Text", this, &ProjectContextMenu::onOpenFileAsText));
    mActions.insert(actReOpen, addAction("&Reopen File using Editor", this, &ProjectContextMenu::onReOpenFile));
    mActions.insert(actReOpenAsText, addAction("Reopen File as Text", this, &ProjectContextMenu::onReOpenFileAsText));

    mActions.insert(actSep1, addSeparator());

    mActions.insert(actExplorer, addAction("&Open Location", this, &ProjectContextMenu::onOpenFileLoc));
    mActions.insert(actOpenTerminal, addAction("&Open Terminal", this, &ProjectContextMenu::onOpenTerminal));
    mActions.insert(actGdxDiff, addAction("&Open in GDX Diff", this, &ProjectContextMenu::onGdxDiff));
    mActions.insert(actGdxReset, addAction("&Reset GDX State", this, &ProjectContextMenu::onGdxReset));
    mActions.insert(actLogTab, addAction("&Open Log Tab", this, &ProjectContextMenu::onOpenLog));
    mActions.insert(actOpenEfi, addAction("Create &EFI File", this, &ProjectContextMenu::onOpenEfi));

    mActions.insert(actSep2, addSeparator());
    mActions.insert(actSetMain, addAction("&Set as Main File", this, &ProjectContextMenu::onSetMainFile));

    mActions.insert(actSep3, addSeparator());
    mActions.insert(actAddExisting, addAction("Add &Existing File", this, &ProjectContextMenu::onAddExisitingFile));
    mActions.insert(actAddNewFile, addAction("Add &New File", this, &ProjectContextMenu::onAddNewFile));

    QMenu* newSolverOptionMenu = addMenu( "Add New Solver Option File" );
    mActions.insert(actAddNewOpt, newSolverOptionMenu->menuAction());
    int addNewSolverOptActionBaseIndex = actAddNewOpt*1000;

    mActions.insert(actAddNewPf, addAction("Add &New GAMS Parameter File", this, &ProjectContextMenu::onAddNewPfFile));

    QDir sysdir(CommonPaths::systemDir());
    QStringList optFiles = sysdir.entryList(QStringList() << "opt*.def" , QDir::Files);

    try {
        support::SolverConfigInfo solverInfo;
        QMap<QString, QString> solverDefFileNames = solverInfo.solverOptDefFileNames();

        if (solverDefFileNames.size()>0) { // when solver definition file information is available
            for (auto it = solverDefFileNames.constBegin() ; it != solverDefFileNames.constEnd() ; ++it) {
                if (optFiles.contains(solverDefFileNames.value(it.key()))) { //there exists such a file
                    QString solver = it.key().toLower();
                    QAction* createSolverOption = newSolverOptionMenu->addAction(solver);
                    connect(createSolverOption, &QAction::triggered, this, [=] { onAddNewSolverOptionFile(solver); });

                    mAvailableSolvers << it.key();
                    mSolverOptionActions.insert(++addNewSolverOptActionBaseIndex, createSolverOption);
                }
            }
        } else { // when no information on solver option definition file names
           for (const QString &filename : std::as_const(optFiles)) {
               QString solvername = filename.mid(QString("opt").length());
               solvername.replace(".def", "");
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
    mActions.insert(actSelectAll, addAction("Select &All", this, &ProjectContextMenu::onSelectAll));
    mActions.insert(actCollapseAll, addAction("Collapse All Projects", this, &ProjectContextMenu::onCollapseAll));
    mActions.insert(actExpandAll, addAction("Expand All", this, &ProjectContextMenu::onExpandAll));

    mActions.insert(actSep6, addSeparator());
    mActions.insert(actProjectNew, addAction("&New Project...",  this, &ProjectContextMenu::newProject));
    mActions.insert(actProjectOpen, addAction("&Open Project...",  this, &ProjectContextMenu::openProject));
    mActions.insert(actProjectMove, addAction("&Move Project File...",  this, &ProjectContextMenu::onMoveProject));
    mActions.insert(actProjectCopy, addAction("&Copy Project...",  this, &ProjectContextMenu::onCopyProject));
    mActions.insert(actSep6, addSeparator());
    mActions.insert(actFocusProject, addAction("&Focus Project",  this, &ProjectContextMenu::onFocusProject));
    mActions.insert(actUnfocusProject, addAction("&Unfocus Project",  this, [this]() {
                        emit focusProject(nullptr);
                    }));
    mActions.insert(actSep7, addSeparator());
    mActions.insert(actCloseProject, addAction(mTxtCloseProject, this, &ProjectContextMenu::onCloseProject));
    mActions.insert(actCloseDelProject, addAction(mTxtCloseDelProject, this, &ProjectContextMenu::onCloseDelProject));
    mActions.insert(actCloseGroup, addAction(mTxtCloseProject, this, &ProjectContextMenu::onCloseGroup));
    mActions.insert(actCloseFile, addAction(mTxtCloseFile, this, &ProjectContextMenu::onCloseFile));
    mActions.insert(actCloseDelFile, addAction(mTxtCloseDelFile, this, &ProjectContextMenu::onCloseDelFile));

}

void ProjectContextMenu::initialize(const QVector<PExAbstractNode *> &selected, PExProjectNode *focussedProject)
{
    // synchronize current and selected
    mNodes.clear();
    mNodes = selected;
    if (mNodes.isEmpty() && focussedProject)
        mNodes << focussedProject;
    bool protectNodes = !allowChange(mNodes);
    bool single = mNodes.count() == 1;
    bool isProject = mNodes.size() ? bool(mNodes.first()->toProject()) : false;
    bool isGroup = mNodes.size() ? bool(mNodes.first()->toGroup()) && !isProject : false;
    PExProjectNode *project = mNodes.size() ? mNodes.first()->assignedProject() : nullptr;
    bool canMoveProject = project && !protectNodes && project->childCount() && project->type() <= PExProjectNode::tCommon;
    bool isSmallProject = project && project->type() == PExProjectNode::tSmall;
    bool isGamsSys = false;
    bool isProjectEfi = false;
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        if (PExProjectNode *project = node->toProject())
            if (project->type() > PExProjectNode::tCommon)
                isGamsSys = true;
        if (node->assignedProject() != project)
            canMoveProject = false;
    }
    if (isProject && single) {
        QString efi = getEfiName(project);
        isProjectEfi = !efi.isEmpty();
        if (isProjectEfi && QFileInfo::exists(efi))
            mActions[actOpenEfi]->setText("Open &EFI File");
    }

    PExFileNode *fileNode = mNodes.size() ? mNodes.first()->toFile() : nullptr;
    bool isFreeSpace = !fileNode && !isProject;
    bool isGmsFile = fileNode && fileNode->file()->kind() == FileKind::Gms;
    bool isRunnable = false;
    bool isOpen = fileNode && fileNode->file()->isOpen();
    bool isOpenable = fileNode && !fileNode->file()->isOpen();
    bool isOptFile = fileNode && fileNode->file()->kind() == FileKind::Opt;
    bool isPfFile = fileNode && fileNode->file()->kind() == FileKind::Pf;
    bool isGucFile = fileNode && fileNode->file()->kind() == FileKind::Guc;
    bool isEfiFile = fileNode && fileNode->file()->kind() == FileKind::Efi;
    bool isGConnectFile = fileNode && fileNode->file()->kind() == FileKind::GCon;
    bool isOpenableAsText = isOpenable && (isOptFile || isPfFile || isGucFile || isGConnectFile);
    bool isOpenWithSolverOptionEditor = false;
    bool isOpenWithGamsUserConfigEditor = false;
    bool isOpenWithEfiEditor = false;
    bool isOpenWithGamsConnectEditor = false;
    if (fileNode) {
        for (QWidget *e : fileNode->file()->editors()) {
            if (ViewHelper::toSolverOptionEdit(e))
                isOpenWithSolverOptionEditor = true;
            else if (ViewHelper::toGamsConfigEditor(e))
                isOpenWithGamsUserConfigEditor = true;
            else if (ViewHelper::toEfiEditor(e))
                isOpenWithEfiEditor = true;
            else if (ViewHelper::toGamsConnectEditor(e))
                isOpenWithGamsConnectEditor= true;
        }
    }
    bool isReOpenableAsText = isOpen && (isOpenWithSolverOptionEditor || isOpenWithGamsUserConfigEditor || isOpenWithEfiEditor || isOpenWithGamsConnectEditor);

    bool isReOpenableWithSolverOptionEditor = isOpen && (isOptFile || isPfFile) && !isOpenWithSolverOptionEditor;
    bool isReOpenableWithGamsUserConfigEditor = isOpen && isGucFile && !isOpenWithGamsUserConfigEditor;
    bool isReOpenableWithEfiEditor = isOpen && isEfiFile && !isOpenWithEfiEditor;
    bool isReopenableWithGamsConnectEditor = isOpen && isGConnectFile && !isOpenWithGamsConnectEditor;
    bool isReOpenable = isReOpenableWithSolverOptionEditor || isReOpenableWithGamsUserConfigEditor || isReOpenableWithEfiEditor || isReopenableWithGamsConnectEditor;

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
    bool hasGdx = false;
    bool hasOpenGdx = false;
    for (PExAbstractNode *node : std::as_const(mNodes)) {
        PExFileNode *fileNode = node->toFile();
        if (fileNode && fileNode->file()->kind() == FileKind::Gdx) {
            hasGdx = true;
            if (fileNode->file()->isOpen())
                hasOpenGdx = true;
        }
    }

    QString file;
    if (fileNode && fileNode->assignedProject()) {
        file = fileNode->assignedProject()->parameter("gms");
        isRunnable = fileNode->location() == file;
    }

    mActions[actExplorer]->setEnabled(single);
    mActions[actExplorer]->setVisible(!isGamsSys);
    mActions[actOpenTerminal]->setEnabled(single);
    mActions[actOpenTerminal]->setVisible(!isFreeSpace && !isGamsSys);

    mActions[actGdxDiff]->setEnabled(isOpenableWithGdxDiff);
    mActions[actGdxDiff]->setVisible(isOpenableWithGdxDiff);

    mActions[actGdxReset]->setVisible(hasGdx);
    mActions[actGdxReset]->setText(QString(hasOpenGdx ? "Close and " : "") + "&Reset GDX State");

    mActions[actOpen]->setEnabled(isOpenable);
    mActions[actOpen]->setVisible(isOpenable && !isGamsSys);
    mActions[actOpenAsText]->setEnabled(isOpenableAsText);
    mActions[actOpenAsText]->setVisible(isOpenableAsText);

    if (isReOpenableWithGamsUserConfigEditor)
        mActions[actReOpen]->setText( "&Reopen File using GAMS User Configuration Editor" );
    else if (isReOpenableWithSolverOptionEditor && isOptFile)
        mActions[actReOpen]->setText( "&Reopen File using Solver Option Editor" );
    else if (isReOpenableWithSolverOptionEditor && isPfFile)
        mActions[actReOpen]->setText( "&Reopen File using GAMS Parameter Editor" );
    else if (isReopenableWithGamsConnectEditor)
        mActions[actReOpen]->setText( "&Reopen File using GAMS Connect Editor" );
    else
        mActions[actReOpen]->setText( "&Reopen File using EFI Editor" );
    mActions[actReOpen]->setEnabled(isReOpenable);
    mActions[actReOpen]->setVisible(isReOpenable);
    mActions[actReOpenAsText]->setEnabled(isReOpenableAsText);
    mActions[actReOpenAsText]->setVisible(isReOpenableAsText);

    mActions[actLogTab]->setVisible(isProject && !isGamsSys);
    mActions[actLogTab]->setEnabled(single);

    mActions[actOpenEfi]->setVisible(isProject && !isGamsSys);
    mActions[actOpenEfi]->setEnabled(isProjectEfi);

    mActions[actProjectMove]->setVisible(!isFreeSpace);
    mActions[actProjectMove]->setEnabled(canMoveProject && !isSmallProject);

    mActions[actProjectCopy]->setVisible(!isFreeSpace);
    mActions[actProjectCopy]->setEnabled(canMoveProject);
    mActions[actProjectCopy]->setText(isSmallProject ? "&Export Project..." :"&Copy Project...");

    mActions[actFocusProject]->setVisible(!isGamsSys && !focussedProject && !isFreeSpace);
    mActions[actUnfocusProject]->setVisible(focussedProject);

    mActions[actSep1]->setVisible(isProject);
    mActions[actSetMain]->setVisible(isGmsFile && !isRunnable && canMoveProject && single);
//    mActions[actSetMain]->setEnabled(single);

    mActions[actAddNewFile]->setVisible((isProject || isGroup) && !isGamsSys);
    mActions[actAddExisting]->setVisible((isProject || isGroup) && !isGamsSys);

    mActions[actCloseProject]->setVisible(isProject);
    mActions[actCloseProject]->setEnabled(!protectNodes);
    mActions[actCloseDelProject]->setVisible(isProject);
    mActions[actCloseDelProject]->setEnabled(!protectNodes);
    mActions[actCloseGroup]->setVisible(isGroup);
    mActions[actCloseGroup]->setEnabled(!protectNodes);
    mActions[actCloseFile]->setVisible(fileNode);
    mActions[actCloseFile]->setEnabled(!protectNodes);
    mActions[actCloseDelFile]->setVisible(fileNode);
    mActions[actCloseDelFile]->setEnabled(!protectNodes);

    mActions[actCloseProject]->setText(mTxtCloseProject + (single ? "" : "s"));
    mActions[actCloseDelProject]->setText(mTxtCloseDelProject + (single ? "" : "s"));
    mActions[actCloseGroup]->setText(mTxtCloseGroup);
    mActions[actCloseFile]->setText(mTxtCloseFile + (single ? "" : "s"));
    mActions[actCloseDelFile]->setText(mTxtCloseDelFile + (single ? "" : "s"));

    // create solver option files
    mActions[actSep3]->setVisible(isProject);
    mActions[actAddNewOpt]->setVisible(isProject && !isGamsSys);
    for (QAction* action : std::as_const(mSolverOptionActions))
        action->setVisible(isProject);
    mActions[actAddNewPf]->setVisible(isProject && !isGamsSys);
}

void ProjectContextMenu::onCloseFile()
{
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        PExFileNode *file = node->toFile();
        if (file) emit closeFile(file);
    }
}

void ProjectContextMenu::onCloseDelFile()
{
    QList<PExFileNode *> delFiles;
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        PExFileNode *file = node->toFile();
        if (file) delFiles << file;
    }
    if (!delFiles.isEmpty()) emit closeDelFiles(delFiles);
}

void ProjectContextMenu::onAddExisitingFile()
{
    QVector<PExProjectNode*> projects;
    QString sourcePath = "";
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        if (sourcePath.isEmpty()) {
            PExGroupNode *group = node->toGroup();
            if (group) sourcePath = group->location();
        }
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (!project) continue;
        if (!projects.contains(project))
            projects << project;
    }
    if (projects.isEmpty()) return;
    if (sourcePath.isEmpty()) sourcePath = projects.first()->location();
    QStringList filePaths = QFileDialog::getOpenFileNames(mParent, "Add existing files", sourcePath,
                                                    ViewHelper::dialogFileFilterAll().join(";;"),
                                                    nullptr,
                                                    DONT_RESOLVE_SYMLINKS_ON_MACOS);
    if (filePaths.isEmpty()) return;

    for (PExProjectNode *project: std::as_const(projects)) {
        for (const QString &filePath: std::as_const(filePaths)) {
            if (filePath.endsWith(".gsp", Qt::CaseInsensitive))
                SysLogLocator::systemLog()->append("Can't open project in project: " + filePath + " ignored.");
            else
                emit addExistingFile(project, filePath);
        }
    }
}

void ProjectContextMenu::onAddNewFile()
{
    QVector<PExProjectNode*> projects;
    QString path = "";
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        if (path.isEmpty()) {
            PExGroupNode *group = node->toGroup();
            if (group) path = group->location();
        }
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (!project) continue;
        if (!projects.contains(project))
            projects << project;
    }
    if (!projects.isEmpty())
        emit newFileDialog(projects, path);
}

void ProjectContextMenu::onAddNewPfFile()
{
    QVector<PExProjectNode*> projects;
    QString path = "";
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (!project) continue;
        if (path.isEmpty()) path = project->location();
        if (!projects.contains(project))
            projects << project;
    }
    if (!projects.isEmpty())
        emit newFileDialog(projects, path, "", FileKind::Pf);
}

void ProjectContextMenu::setParent(QWidget *parent)
{
    mParent = parent;
}

void ProjectContextMenu::onCloseGroup()
{
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        PExGroupNode *group = node->toGroup();
        if (!group) continue;
        QVector<PExFileNode*> files = group->listFiles();
        for (PExFileNode* file : std::as_const(files)) {
            emit closeFile(file);
        }
    }
}

void ProjectContextMenu::onCloseProject()
{
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (project) emit closeProject(project);
    }
}

void ProjectContextMenu::onCloseDelProject()
{
    QList<PExProjectNode*> pro2close;
    QList<QString> pro2del;
    QString proFirst;
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (project) {
            pro2close << project;
            QString gsp = project->fileName();
            if (project->type() == PExProjectNode::tCommon && QFile::exists(gsp)) {
                pro2del << gsp;
                if (proFirst.isEmpty()) proFirst = project->name();
            }
        }
    }
    if (pro2del.size()) {
        bool one = pro2del.size() == 1;
        QString text = one ? "Project file for " + proFirst : QString::number(pro2del.size()) + " project files";

        int choice = MsgBox::question("Deleting Project" + QString(one ? "" : "s") , text + " will be deleted", mParent
                                      , "OK", "Cancel", "", 0, 1);
        if (choice == 1) return;
    }

    for (PExProjectNode *pro : pro2close)
        emit closeProject(pro);
    for (QString gsp : pro2del) {
        bool ok = QFile::remove(gsp);
        if (!ok) SysLogLocator::systemLog()->append("Couldn't remove " + gsp);
    }
}

void ProjectContextMenu::onSetMainFile()
{
    PExFileNode *file = mNodes.first()->toFile();
    if (file) emit setMainFile(file);
}

void ProjectContextMenu::onMoveProject()
{
    PExProjectNode *project = mNodes.first()->assignedProject();
    emit moveProject(project, false);
}

void ProjectContextMenu::onCopyProject()
{
    PExProjectNode *project = mNodes.first()->assignedProject();
    emit moveProject(project, true);
}

void ProjectContextMenu::onFocusProject()
{
    PExProjectNode *project = mNodes.isEmpty() ? nullptr : mNodes.first()->assignedProject();
    emit focusProject(project);
}

void ProjectContextMenu::onAddNewSolverOptionFile(const QString &solverName)
{
    QVector<PExProjectNode*> groups;
    QString path = "";
    for (PExAbstractNode *node: std::as_const(mNodes)) {
        PExProjectNode *project = node->toProject();
        if (!project) project = node->assignedProject();
        if (!project) continue;
        if (path.isEmpty()) path = project->location();
        if (!groups.contains(project))
            groups << project;
    }
    if (!groups.isEmpty())
        emit newFileDialog(groups, path, solverName);
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

void ProjectContextMenu::onGdxReset()
{
    QStringList files;
    for (PExAbstractNode *node : std::as_const(mNodes)) {
        PExFileNode *fileNode = node->toFile();
        if (fileNode && fileNode->file()->kind() == FileKind::Gdx)
            files << fileNode->location();
    }
    if (!files.isEmpty())
        emit resetGdxStates(files);
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
        if (!project) return;
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
    if (!project || !project->runnableGms()) return QString();
    QFileInfo info(project->runnableGms()->location());
    return info.path() + '/' + info.completeBaseName() + ".efi";
}

bool ProjectContextMenu::allowChange(const QList<PExAbstractNode*> nodes)
{
    for (PExAbstractNode *node : nodes) {
        if (!node) continue;
        PExProjectNode *project = node->assignedProject();
        if (!project) continue;
        if (!project->process() || project->process()->state() == QProcess::NotRunning)
            continue;
        if (node->toProject()) return false;
        if (PExFileNode *file = node->toFile()) {
            if (!file) continue;
            if (project->runnableGms() == file->file()) return false;
        }
    }
    return true;
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
