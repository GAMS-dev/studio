#include "filegroupcontext.h"
#include "exception.h"

namespace gams {
namespace ide {

FileGroupContext::~FileGroupContext()
{
    // TODO(JM)  delete the entry in the parents childList
    mChildList.clear();
}

void FileGroupContext::setFlag(ContextFlag flag)
{
    if (flag == FileSystemContext::fcEditMod || flag == FileSystemContext::fcFileMod)
        throw QException();
    FileSystemContext::setFlag(flag);
}

void FileGroupContext::unsetFlag(ContextFlag flag)
{
    if (flag == FileSystemContext::fcEditMod || flag == FileSystemContext::fcFileMod)
        throw QException();
    FileSystemContext::unsetFlag(flag);
}

int FileGroupContext::peekIndex(const QString& name, bool *exactMatch)
{
    if (exactMatch)
        *exactMatch = false;
    for (int i = 0; i < childCount(); ++i) {
        FileSystemContext *child = childEntry(i);
        int comp = name.compare(child->name(), Qt::CaseInsensitive);
        if (comp >= 0) {
            if (comp == 0 && exactMatch)
                *exactMatch = true;
            return i;
        }
    }
    return childCount();
}

void FileGroupContext::insertChild(FileSystemContext* child)
{
    if (!child) return;
    int pos = 0;
    QString name = child->name();
    for (int i = 0; i < childCount(); ++i) {
        int comp = name.compare(childEntry(i)->name(), Qt::CaseInsensitive);
        if (comp > 0) {
            pos = i;
            break;
        }
    }
    insertChild(pos, child);
}

void FileGroupContext::insertChild(int pos, FileSystemContext* child)
{
    if (child == this)
        throw FATAL() << "can't add a element to itself";
    mChildList.insert(pos, child);
}

void FileGroupContext::removeChild(FileSystemContext* child)
{
    mChildList.removeOne(child);
}

int FileGroupContext::childCount()
{
    return mChildList.count();
}

int FileGroupContext::indexOf(FileSystemContext* child)
{
    return mChildList.indexOf(child);
}

FileSystemContext*FileGroupContext::childEntry(int index)
{
    return mChildList.at(index);
}

void FileGroupContext::directoryChanged(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) {

    } else {
        emit contentChanged(mId, dir);
    }
}

FileGroupContext::FileGroupContext(FileGroupContext* parent, int id, QString name, QString location, bool isGist)
    : FileSystemContext(parent, id, name, location, isGist)
{
    mFlags = FileSystemContext::fcGroup;
}

} // namespace ide
} // namespace gams
