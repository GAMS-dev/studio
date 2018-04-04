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
    mActions.insert(0, addAction("Close &group",  this, &ProjectContextMenu::onCloseGroup));
    mActions.insert(1, addAction("Close &file",  this, &ProjectContextMenu::onCloseFile));
//    mActions.insert(0, addAction("&Run group",  this, &ProjectContextMenu::onRunGroup));

    mActions.insert(2, addSeparator());
    mActions.insert(3, addAction("&Open file location",  this, &ProjectContextMenu::onOpenFileLoc));
//    mActions.insert(1, addSeparator());
//    mActions.insert(1, addAction("Re&name",  this, &ProjectContextMenu::onRenameGroup));

//    mActions.insert(2, addSeparator());
//    mActions.insert(2, addAction("Re&name",  this, &ProjectContextMenu::onRenameFile));
}

void ProjectContextMenu::setNode(FileSystemContext* context)
{
    mNode = context;
    mActions[0]->setVisible(true);
    mActions[1]->setVisible(mNode->type() == FileSystemContext::File);
}

void ProjectContextMenu::onCloseFile()
{
    FileContext *file = (mNode->type() == FileSystemContext::File) ? static_cast<FileContext*>(mNode) : nullptr;
    if (file) emit closeFile(file);
}

void ProjectContextMenu::onCloseGroup()
{
    FileGroupContext *group = (mNode->type() == FileSystemContext::FileGroup) ? static_cast<FileGroupContext*>(mNode)
                                                                              : mNode->parentEntry();
    if (group) emit closeGroup(group);
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

void ProjectContextMenu::onRunGroup()
{
    FileGroupContext *group = (mNode->type() == FileSystemContext::FileGroup) ? static_cast<FileGroupContext*>(mNode)
                                                                              : mNode->parentEntry();
    if (group) {
        emit runGroup(group);
    }
}

} // namespace studio
} // namespace gams
