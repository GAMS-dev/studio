#include "filecontext.h"
#include "filegroupcontext.h"

namespace gams {
namespace ide {

FileContext::FileContext(FileGroupContext *parent, int id, QString name, QString location, bool isGist)
    : FileSystemContext(parent, id, name, location, isGist)
{
    mActive = true;
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
    qDebug() << "Text changed";
    if (mCrudState != CrudState::eUpdate) {
        mCrudState = CrudState::eUpdate;
        emit nameChanged(mId, name());
        qDebug() << "FIRST Text changed";
    }
}

} // namespace ide
} // namespace gams
