#include "filemetrics.h"

namespace gams {
namespace studio {

FileMetrics::FileMetrics()
    : mExists(false), mSize(0), mType(&FileType::from(FileType::None))
{ }

FileMetrics::FileMetrics(const FileMetrics& other)
    : mExists(other.mExists), mSize(other.mSize), mCreated(other.mCreated)
    , mModified(other.mModified), mType(other.mType)
{ }

FileMetrics&FileMetrics::operator=(const FileMetrics& other)
{
    mExists = other.mExists;
    mSize = other.mSize;
    mCreated = other.mCreated;
    mModified = other.mModified;
    mType = other.mType;
    return *this;
}

const FileType&FileMetrics::fileType() const
{
    return *mType;
}

FileMetrics::FileMetrics(QFileInfo fileInfo)
    : mType(&FileType::from(fileInfo.suffix()))
{
    mExists = fileInfo.exists();
    mSize = mExists ? fileInfo.size() : 0;
    mCreated = mExists ? fileInfo.created() : QDateTime();
    mModified = mExists ? fileInfo.lastModified() : QDateTime();
}

FileMetrics::ChangeKind FileMetrics::check(QFileInfo fileInfo)
{
    if (mModified.isNull()) return ckSkip;
    if (!fileInfo.exists()) {
        // TODO(JM) #106: find a file in the path fitting created, modified and size values
        return ckNotFound;
    }
    if (fileInfo.lastModified() != mModified) return ckModified;
    return ckUnchanged;
}

} // namespace studio
} // namespace gams
