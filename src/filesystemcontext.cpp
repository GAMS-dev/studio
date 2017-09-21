#include "filesystemcontext.h"
#include "filegroupcontext.h"

namespace gams {
namespace ide {

FileSystemContext::FileSystemContext(FileGroupContext* parent, int id, QString name, QString location, bool isGist)
    : QObject(parent), mId(id), mParent(nullptr), mName(name), mLocation(location), mIsGist(isGist)
{
    setParentEntry(parent);
    mFlags = 0;

}

void FileSystemContext::checkFlags()
{
}

FileSystemContext::~FileSystemContext()
{
    if (parentEntry()) {
        parentEntry()->removeChild(this);
    }
}

int FileSystemContext::id() const
{
    return mId;
}

bool FileSystemContext::matches(const QString &name, bool isGist) const
{
    return isGist == mIsGist && mName.compare(name, Qt::CaseInsensitive) == 0;
}

FileGroupContext* FileSystemContext::parentEntry() const
{
    return mParent;
}

void FileSystemContext::setParentEntry(FileGroupContext* parent)
{
    if (parent != mParent) {
        if (mParent) mParent->removeChild(this);
        mParent = parent;
        if (mParent) {
            mParent->insertChild(this);
        }
    }
}

FileSystemContext* FileSystemContext::childEntry(int index)
{
    Q_UNUSED(index);
    return nullptr;
}

int FileSystemContext::childCount()
{
    return 0;
}

bool FileSystemContext::isGist() const
{
    return mIsGist;
}

const QString FileSystemContext::name()
{
    return mName;
}

void FileSystemContext::setName(const QString& name)
{
    if (mName != name) {
        mName = name;
        emit nameChanged(mId, mName);
    }
}

const QString& FileSystemContext::location() const
{
    return mLocation;
}

void FileSystemContext::setLocation(const QString& location)
{
    if (!location.isEmpty()) {
        mLocation = location;
        QFileInfo fi(location);
        setName(fi.fileName());
    }
}

const FileSystemContext::ContextFlags& FileSystemContext::flags() const
{
    return mFlags;
}

void FileSystemContext::setFlag(ContextFlag flag, bool value)
{
    if (!value) {
        unsetFlag(flag);
    } else if (!mFlags.testFlag(flag)) {
        mFlags.setFlag(flag);
        if (parentEntry()) parentEntry()->checkFlags();
    }
}

void FileSystemContext::unsetFlag(ContextFlag flag)
{
    if (mFlags.testFlag(flag)) {
        mFlags &= ~flag;
        if (parentEntry()) parentEntry()->checkFlags();
    }
}

bool FileSystemContext::testFlag(FileSystemContext::ContextFlag flag)
{
    return mFlags.testFlag(flag);
}


} // namespace ide
} // namespace gams
