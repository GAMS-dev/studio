#include "filemetarepo.h"
#include "filemeta.h"
#include "syntax/textmarkrepo.h"
#include "studiosettings.h"
#include "exception.h"
#include <QFileInfo>

namespace gams {
namespace studio {

FileMetaRepo::FileMetaRepo(QObject *parent, StudioSettings *settings) : QObject(parent), mSettings(settings)
{
    connect(&mWatcher, &QFileSystemWatcher::directoryChanged, this, &FileMetaRepo::dirChanged);
    connect(&mWatcher, &QFileSystemWatcher::fileChanged, this, &FileMetaRepo::fileChanged);
}

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
    QFileInfo fi(fileMeta->location());
    if (fi.exists()) {
        mWatcher.addPath(fi.absoluteFilePath());
        mWatcher.addPath(fi.absolutePath());
    } else {
        mMissList << fi.absoluteFilePath();
    }
    return res;
}

TextMarkRepo *FileMetaRepo::textMarkRepo() const
{
    if (!mTextMarkRepo) EXCEPT() << "Missing initialization. Method init() need to be called.";
    return mTextMarkRepo;
}

QVector<FileMeta*> FileMetaRepo::openFiles() const
{
    QVector<FileMeta*> res;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        if (i.value()->isOpen()) res << i.value();
    }
    return res;
}

void FileMetaRepo::openFile(FileMeta *fm, FileId runId, bool focus, int codecMib)
{
    if (!mProjectRepo) EXCEPT() << "Missing initialization. Method init() need to be called.";
    ProjectRunGroupNode* runGroup = mProjectRepo->findRunGroup(runId);
    emit mProjectRepo->openFile(fm, focus, runGroup, codecMib);
}

void FileMetaRepo::dirChanged(const QString &path)
{
    // TODO(JM) stack dir-name to check after timeout if it's deleted or contents has changed
}

void FileMetaRepo::fileChanged(const QString &path)
{
    // TODO(JM) stack file-name to check after timeout if it's deleted or contents has changed
}

StudioSettings *FileMetaRepo::settings() const
{
    return mSettings;
}

void FileMetaRepo::init(TextMarkRepo *textMarkRepo, ProjectRepo *projectRepo)
{
    if (mTextMarkRepo == textMarkRepo && mProjectRepo == projectRepo) return;
    if (mTextMarkRepo || mProjectRepo) EXCEPT() << "The FileMetaRepo already has been initialized.";
    if (!textMarkRepo) EXCEPT() << "The TextMarkRepo must not be null.";
    if (!projectRepo) EXCEPT() << "The ProjectRepo must not be null.";
    mTextMarkRepo = textMarkRepo;
    mProjectRepo = projectRepo;
}

FileMeta* FileMetaRepo::findOrCreateFileMeta(QString location)
{
    if (location.isEmpty()) return nullptr;
    FileMeta* res = fileMeta(location);
    if (!res) {
        res = new FileMeta(this, mNextFileId++, location);
        addFileMeta(res);
    }
    return res;
}

} // namespace studio
} // namespace gams
