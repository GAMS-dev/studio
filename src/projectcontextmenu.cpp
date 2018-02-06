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
        FileGroupContext *parent = file->parentEntry();

        if (parent) openLoc = parent->location();

    } else if (mNode->type() == FileSystemContext::FileGroup) {
        FileGroupContext *group = static_cast<FileGroupContext*>(mNode);
        if (group) openLoc = group->location();
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(openLoc));
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
