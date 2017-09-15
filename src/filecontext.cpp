#include "filecontext.h"
#include "filegroupcontext.h"

namespace gams {
namespace ide {

FileContext::FileContext(FileGroupContext *parent, int id, QString name, QString location, bool isGist)
    : FileSystemContext(parent, id, name, location, isGist)
{
    mActive = true;
}

CrudState FileContext::crudState() const
{
    return mCrudState;
}

void FileContext::saved()
{
    if (mCrudState != CrudState::eRead) {
        mCrudState = CrudState::eRead;
        emit nameChanged(mId, name());
    }
}

QString FileContext::codec() const
{
    return mCodec;
}

void FileContext::setCodec(const QString& codec)
{
    // TODO(JM) changing the codec must trigger conversion (not necessarily HERE)
    mCodec = codec;
}

const QString FileContext::name()
{
    return mName + (mCrudState==CrudState::eUpdate ? "*" : "");
}

void FileContext::textChanged()
{
    if (mCrudState != CrudState::eUpdate) {
        mCrudState = CrudState::eUpdate;
        emit nameChanged(mId, name());
    }
}

} // namespace ide
} // namespace gams
