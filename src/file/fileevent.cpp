#include "fileevent.h"

namespace gams {
namespace studio {

QEvent::Type FileEvent::mType = QEvent::None;

FileEvent::FileEvent(FileId fileId, Kind kind): QEvent(type()), mFileId(fileId), mKind(kind)
{

}

QEvent::Type FileEvent::type()
{
    if (mType == QEvent::None)
        mType = static_cast<QEvent::Type>(QEvent::registerEventType());
    return mType;

}

FileId FileEvent::fileId() const
{
    return mFileId;
}

FileEvent::Kind FileEvent::kind() const
{
    return mKind;
}

} // namespace studio
} // namespace gams
