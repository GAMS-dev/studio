#include "filemetarepo.h"
#include "filemeta.h"
#include <QFileInfo>

namespace gams {
namespace studio {

FileMetaRepo::FileMetaRepo(QObject *parent) : QObject(parent)
{}

FileMeta *FileMetaRepo::fileMeta(const FileId &fileId) const
{
    return mFiles.value(fileId, nullptr);
}

FileMeta *FileMetaRepo::fileMeta(const QString &location) const
{
    if (location.startsWith('[')) { // special instances (e.g. "[LOG]123" )
        foreach (FileMeta* fm, mFiles.values()) {
            if (fm->location() == location) return fm;
        }
    } else {
        QFileInfo fi(location);
        foreach (FileMeta* fm, mFiles.values()) {
            if (QFileInfo(fm->location()) == fi) return fm;
        }
    }
    return nullptr;
}

FileId FileMetaRepo::addFileMeta(FileMeta *fileMeta)
{
    FileId res = mNextFileId++;
    mFiles.insert(res, fileMeta);
    return res;
}

FileMeta* FileMetaRepo::findOrCreateFileMeta(QString location)
{
    FileMeta* res = fileMeta(location);
    if (!res) {
        res = new FileMeta(mNextFileId++, location);
        addFileMeta(res);
    }
    return res;
}

} // namespace studio
} // namespace gams
