#include "projectcontextmenu.h"
#include "filegroupcontext.h"
#include "logcontext.h"

namespace gams {
namespace studio {

ProjectContextMenu::ProjectContextMenu()
{
    mActions.insert(0, addAction("&Close group",  this, &ProjectContextMenu::onCloseGroup));
//    mActions.insert(0, addAction("&Run group",  this, &ProjectContextMenu::onRunGroup));

//    mActions.insert(1, addSeparator());
//    mActions.insert(1, addAction("Re&name",  this, &ProjectContextMenu::onRenameGroup));

//    mActions.insert(2, addSeparator());
//    mActions.insert(2, addAction("Re&name",  this, &ProjectContextMenu::onRenameFile));
}

void ProjectContextMenu::setNode(FileSystemContext* context)
{
    mNode = context;
}

void ProjectContextMenu::onCloseGroup()
{
    FileGroupContext *group = (mNode->type() == FileSystemContext::FileGroup) ? static_cast<FileGroupContext*>(mNode)
                                                                              : mNode->parentEntry();
    if (group) {
        emit closeGroup(group);
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
