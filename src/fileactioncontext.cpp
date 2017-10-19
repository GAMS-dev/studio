#include "fileactioncontext.h"
#include <QtWidgets>
#include "exception.h"

namespace gams {
namespace studio {

FileActionContext::FileActionContext(int id, QAction *action)
    : FileSystemContext(id, action->text(), action->toolTip(), FileSystemContext::FileAction), mAction(action)
{
    mFlags = cfVirtual;
}

void FileActionContext::trigger()
{
    emit mAction->trigger();
}

void FileActionContext::setLocation(const QString& location)
{
    Q_UNUSED(location);
    EXCEPT() << "The location of a FileActionContext can't be changed.";
}

QIcon FileActionContext::icon()
{
    return mAction->icon();
}



} // namespace studio
} // namespace gams
