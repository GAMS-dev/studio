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
#include "projectcontextmenu.h"
#include "filegroupcontext.h"
#include "logcontext.h"

namespace gams {
namespace studio {

ProjectContextMenu::ProjectContextMenu()
{
    mActions.insert(0, addAction("&Open file location", this, &ProjectContextMenu::onOpenFileLoc));

    mActions.insert(1, addSeparator());

    mActions.insert(2, addAction("&Run this file", this, &ProjectContextMenu::onRunFile));
    mActions.insert(3, addAction("&Run this file with options", this, &ProjectContextMenu::onRunFile));
    mActions.insert(4, addAction("&Set as main file", this, &ProjectContextMenu::onRunFile));

    mActions.insert(5, addSeparator());

    mActions.insert(6, addAction("Add &existing file", this, &ProjectContextMenu::onAddExisitingFile));
    mActions.insert(7, addAction("Add &new file", this, &ProjectContextMenu::onAddNewFile));

    mActions.insert(8, addSeparator());

    mActions.insert(9, addAction("Close &group", this, &ProjectContextMenu::onCloseGroup));
    mActions.insert(10, addAction("Close &file", this, &ProjectContextMenu::onCloseFile));

//    mActions.insert(1, addAction("Re&name",  this, &ProjectContextMenu::onRenameGroup));
//    mActions.insert(2, addSeparator());
//    mActions.insert(2, addAction("Re&name",  this, &ProjectContextMenu::onRenameFile));
}

void ProjectContextMenu::setNode(FileSystemContext* context)
{
    mNode = context;
//    mActions[0]->setVisible(true);

    bool isGmsFile = false;
    if (mNode->type() == FileSystemContext::File) {
        FileContext *fc = static_cast<FileContext*>(mNode);
        isGmsFile = (fc->metrics().fileType() == FileType::Gms);
    }

    mActions[2]->setVisible(isGmsFile);
    mActions[3]->setVisible(isGmsFile);
    mActions[4]->setVisible(isGmsFile);
    mActions[5]->setVisible(isGmsFile);

    // TODO: remove
    mActions[3]->setEnabled(false);
    mActions[4]->setEnabled(false);

    // all files
    mActions[10]->setVisible(mNode->type() == FileSystemContext::File);
}

void ProjectContextMenu::onCloseFile()
{
    FileContext *file = (mNode->type() == FileSystemContext::File) ? static_cast<FileContext*>(mNode) : nullptr;
    if (file) emit closeFile(file);
}

void ProjectContextMenu::onAddExisitingFile()
{
    QString sourcePath = "";
    emit getSourcePath(sourcePath);

    QString filePath = QFileDialog::getOpenFileName(mParent, "Add existing file", sourcePath,
                                                    tr("GAMS code (*.gms *.inc *.gdx);;"
                                                       "Text files (*.txt);;"
                                                       "All files (*.*)"));
    if (filePath == "") return;
    FileGroupContext *group = (mNode->type() == FileSystemContext::FileGroup) ? static_cast<FileGroupContext*>(mNode)
                                                                              : mNode->parentEntry();
    emit addExistingFile(group, filePath);
}

void ProjectContextMenu::onAddNewFile()
{
    QString sourcePath = "";
    emit getSourcePath(sourcePath);

    QString filePath = QFileDialog::getSaveFileName(mParent, "Create new file...", sourcePath,
                                                    tr("GAMS code (*.gms *.inc );;"
                                                       "Text files (*.txt);;"
                                                       "All files (*.*)"));

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
    FileGroupContext *group = (mNode->type() == FileSystemContext::FileGroup) ? static_cast<FileGroupContext*>(mNode)
                                                                              : mNode->parentEntry();
    emit addExistingFile(group, filePath);
}

void ProjectContextMenu::setParent(QWidget *parent)
{
    mParent = parent;
}

void ProjectContextMenu::onCloseGroup()
{
    FileGroupContext *group = (mNode->type() == FileSystemContext::FileGroup) ? static_cast<FileGroupContext*>(mNode)
                                                                              : mNode->parentEntry();
    if (group) emit closeGroup(group);
}

void ProjectContextMenu::onRunFile()
{
    FileContext *file = static_cast<FileContext*>(mNode);
    emit runFile(file);
}

void ProjectContextMenu::onOpenFileLoc()
{
    QString openLoc;
    if (mNode->type() == FileSystemContext::File) {
        FileContext *file = static_cast<FileContext*>(mNode);

// select file on windows by calling explorer.exe with parameter /select
#ifdef _WIN32
        QString explorerPath = QStandardPaths::findExecutable("explorer.exe");
        if (explorerPath.isEmpty()) {
            FileGroupContext *parent = file->parentEntry();
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
        FileGroupContext *parent = file->parentEntry();
        if (parent) openLoc = parent->location();
        QDesktopServices::openUrl(QUrl::fromLocalFile(openLoc));
#endif
    } else if (mNode->type() == FileSystemContext::FileGroup) {
        FileGroupContext *group = static_cast<FileGroupContext*>(mNode);
        if (group) openLoc = group->location();
        QDesktopServices::openUrl(QUrl::fromLocalFile(openLoc));
    }
}

} // namespace studio
} // namespace gams
