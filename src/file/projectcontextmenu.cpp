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
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopServices>
#include "projectcontextmenu.h"
#include "file.h"
#include "commonpaths.h"

namespace gams {
namespace studio {

ProjectContextMenu::ProjectContextMenu()
{
    mActions.insert(0, addAction("&Open location", this, &ProjectContextMenu::onOpenFileLoc));
    mActions.insert(1, addAction("&Open log tab", this, &ProjectContextMenu::onOpenLog));
    mActions.insert(2, addAction("Re&name",  this, &ProjectContextMenu::onRenameGroup));
    mActions.insert(3, addSeparator());

    mActions.insert(4, addAction("&Set as main file", this, &ProjectContextMenu::onSetMainFile));
    mActions.insert(5, addSeparator());

    mActions.insert(6, addAction("Add &existing file", this, &ProjectContextMenu::onAddExisitingFile));
    mActions.insert(7, addAction("Add &new file", this, &ProjectContextMenu::onAddNewFile));

    mActions.insert(8, addSeparator());

    mActions.insert(9, addAction("Close &group", this, &ProjectContextMenu::onCloseGroup));
    mActions.insert(10, addAction("Close &file", this, &ProjectContextMenu::onCloseFile));

//    mActions.insert(2, addSeparator());
//    mActions.insert(2, addAction("Re&name",  this, &ProjectContextMenu::onRenameFile));
}

void ProjectContextMenu::setNode(ProjectAbstractNode* node)
{
    mNode = node;
    bool isGroup = mNode->toGroup();
    ProjectFileNode *fileNode = node->toFile();
    bool isGmsFile = fileNode && fileNode->file()->kind() == FileKind::Gms; // unused
    bool isRunnable = false;

    QString file;
    if (fileNode && fileNode->assignedRunGroup()) {
        file = fileNode->assignedRunGroup()->specialFile(FileKind::Gms);
        isRunnable = fileNode->location() == file;
    }

    mActions[1]->setVisible(isGroup);
    mActions[2]->setVisible(isGroup);
    mActions[3]->setVisible(isGroup);
    mActions[4]->setVisible(isGmsFile && !isRunnable);

    // all files
    mActions[10]->setVisible(fileNode);
}

void ProjectContextMenu::onCloseFile()
{
    ProjectFileNode *file = mNode->toFile();
    if (file) emit closeFile(file);
}

void ProjectContextMenu::onAddExisitingFile()
{
    QString sourcePath = "";
    emit getSourcePath(sourcePath);

    QStringList filePaths = QFileDialog::getOpenFileNames(mParent,
                                                    "Add existing file",
                                                    sourcePath,
                                                    tr("GAMS code (*.gms *.inc *.gdx *.lst *.opt *ref);;"
                                                       "Text files (*.txt);;"
                                                       "All files (*.*)"),
                                                    nullptr,
                                                    DONT_RESOLVE_SYMLINKS_ON_MACOS);
    if (filePaths.isEmpty()) return;

    for (QString filePath: filePaths) {
        ProjectGroupNode *group = mNode->toGroup();
        if (!group) group = mNode->parentNode();
        emit addExistingFile(group, filePath);
    }
}

void ProjectContextMenu::onAddNewFile()
{
    QString sourcePath = "";
    emit getSourcePath(sourcePath);

    QString filePath = QFileDialog::getSaveFileName(mParent,
                                                    "Create new file...",
                                                    sourcePath,
                                                    tr("GAMS code (*.gms *.inc);;"
                                                       "Text files (*.txt);;"
                                                       "All files (*.*)"),
                                                    nullptr,
                                                    DONT_RESOLVE_SYMLINKS_ON_MACOS);

    if (filePath == "") return;

    QFileInfo fi(filePath);
    if (fi.suffix().isEmpty())
        filePath += ".gms";

    QFile file(filePath);
    if (!file.exists()) { // create
        file.open(QIODevice::WriteOnly);
        file.close();
    } else { // replace old
        file.resize(0);
    }
    ProjectGroupNode *group = mNode->toGroup();
    if (!group) group = mNode->parentNode();
    emit addExistingFile(group, filePath);
}

void ProjectContextMenu::setParent(QWidget *parent)
{
    mParent = parent;
}

void ProjectContextMenu::onCloseGroup()
{
    ProjectGroupNode *group = mNode->toGroup();
    if (!group) group = mNode->parentNode();
    if (group) emit closeGroup(group);
}

void ProjectContextMenu::onRunFile()
{
    ProjectFileNode *file = mNode->toFile();
    if (file) emit runFile(file);
}

void ProjectContextMenu::onSetMainFile()
{
    ProjectFileNode *file = mNode->toFile();
    if (file) emit setMainFile(file);
}

void ProjectContextMenu::onRenameGroup()
{
    ProjectGroupNode *group = mNode->toGroup();
    if (group) emit renameGroup(group);
}

void ProjectContextMenu::onOpenFileLoc()
{
    QString openLoc;
    ProjectFileNode *file = mNode->toFile();
    if (file) {
// select file on windows by calling explorer.exe with parameter /select
#ifdef _WIN32
        QString explorerPath = QStandardPaths::findExecutable("explorer.exe");
        if (explorerPath.isEmpty()) {
            ProjectGroupNode *parent = file->parentNode();
            if (parent) openLoc = parent->location();
            QDesktopServices::openUrl(QUrl::fromLocalFile(openLoc));
        } else {
            QProcess proc;
            proc.setProgram(explorerPath);
            QStringList args;
            args << "/select";
            args << ",";
            args << QDir::toNativeSeparators(file->location());
            proc.setArguments(args);
            proc.start();
            proc.waitForFinished();
        }
#else
        ProjectGroupNode *parent = file->parentNode();
        if (parent) openLoc = parent->location();
        QDesktopServices::openUrl(QUrl::fromLocalFile(openLoc));
#endif
    } else if ((mNode->type() == NodeType::group) || (mNode->type() == NodeType::runGroup)){
        ProjectGroupNode *group = mNode->toGroup();
        if (group) openLoc = group->location();
        QDesktopServices::openUrl(QUrl::fromLocalFile(openLoc));
    }
}

void ProjectContextMenu::onOpenLog()
{
    if (mNode) emit openLogFor(mNode, true);
}

} // namespace studio
} // namespace gams
