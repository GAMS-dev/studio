#include "filetype.h"
#include "exception.h"

namespace gams {
namespace studio {

QList<FileType*> FileType::mList;
FileType *FileType::mNone = nullptr;

FileType::FileType(Kind kind, QString suffix, QString description, bool autoReload, const Kind dependant)
    : mKind(kind), mSuffix(suffix.split(",", QString::SkipEmptyParts)), mDescription(description)
    , mAutoReload(autoReload), mDependant(dependant)
{}


const FileType::Kind FileType::dependant() const
{
    return mDependant;
}

bool FileType::operator ==(const FileType& fileType) const
{
    return (this == &fileType);
}

bool FileType::operator !=(const FileType& fileType) const
{
    return (this != &fileType);
}

bool FileType::operator ==(const FileType::Kind& kind) const
{
    return (mKind == kind);
}

bool FileType::operator !=(const FileType::Kind& kind) const
{
    return (mKind != kind);
}

const bool FileType::autoReload() const
{
    return mAutoReload;
}

QString FileType::description() const
{
    return mDescription;
}

QStringList FileType::suffix() const
{
    return mSuffix;
}

const QList<FileType*> FileType::list()
{
    if (mList.isEmpty()) {
        mList << new FileType(Gsp, "gsp,pro", "GAMS Studio Project", false);
        mList << new FileType(Gms, "gms,inc,txt", "GAMS Source Code", false);
        mList << new FileType(Lst, "lst", "GAMS List File", true);
        mList << new FileType(Lxi, "lxi", "GAMS List File Index", true, Lst);
        mList << new FileType(Gdx, "gdx", "GAMS Data", true);
        mNone = new FileType(None, "", "Unknown File", false);
    }
    return mList;
}

FileType &FileType::from(QString suffix)
{
    for (FileType *ft: list()) {
        if (ft->mSuffix.contains(suffix, Qt::CaseInsensitive))
            return *ft;
    }
    return *mNone;
}

FileType& FileType::from(FileType::Kind kind)
{
    for (FileType *ft: list()) {
        if (ft->mKind == kind)
            return *ft;
    }
    return *mNone;
}

} // namespace studio
} // namespace gams
