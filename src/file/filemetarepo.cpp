#include "filemetarepo.h"
#include "filemeta.h"
#include "projectrepo.h"
#include "syntax/textmarkrepo.h"
#include "studiosettings.h"
#include "exception.h"
#include <QFileInfo>

namespace gams {
namespace studio {

FileMetaRepo::FileMetaRepo(QObject *parent, StudioSettings *settings) : QObject(parent), mSettings(settings)
{
//    connect(&mWatcher, &QFileSystemWatcher::directoryChanged, this, &FileMetaRepo::dirChanged);
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

FileMeta *FileMetaRepo::fileMeta(QWidget * const &editor) const
{
    if (!editor) return nullptr;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        if (i.value()->hasEditor(editor)) return i.value();
    }
    return nullptr;
}

FileId FileMetaRepo::addFileMeta(FileMeta *fileMeta)
{
    FileId res = mNextFileId++;
    mFiles.insert(res, fileMeta);
    QFileInfo fi(fileMeta->location());

    if (!fileMeta->location().startsWith('[')) {
        if (fi.exists()) {
            mWatcher.addPath(fi.absoluteFilePath());
//            mWatcher.addPath(fi.absolutePath());
        } else {
            mMissList << fi.absoluteFilePath();
        }
    }
    return res;
}

void FileMetaRepo::removedFile(FileMeta *fileMeta)
{
    if (fileMeta && !fileMeta->location().startsWith('[')) {
        mFiles.remove(fileMeta->id());
        mWatcher.removePath(fileMeta->location());
        mMissList.removeAll(fileMeta->location());
    }
}

TextMarkRepo *FileMetaRepo::textMarkRepo() const
{
    if (!mTextMarkRepo) EXCEPT() << "Missing initialization. Method init() need to be called.";
    return mTextMarkRepo;
}

ProjectRepo *FileMetaRepo::projectRepo() const
{
    if (!mProjectRepo) EXCEPT() << "Missing initialization. Method init() need to be called.";
    return mProjectRepo;
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

void FileMetaRepo::unwatch(const QString &path)
{
    mWatcher.removePath(path);
    if (fileMeta(path)) mMissList << path;
}

void FileMetaRepo::openFile(FileMeta *fm, FileId runId, bool focus, int codecMib)
{
    if (!mProjectRepo) EXCEPT() << "Missing initialization. Method init() need to be called.";
    ProjectRunGroupNode* runGroup = mProjectRepo->findRunGroup(runId);
    emit mProjectRepo->openFile(fm, focus, runGroup, codecMib);
}

//void FileMetaRepo::dirChanged(const QString &path)
//{
//    // TODO(JM) stack dir-name to check after timeout if it's deleted or contents has changed
//}

void FileMetaRepo::fileChanged(const QString &path)
{
    FileMeta *file = fileMeta(path);
    if (!file) return;
    QFileInfo fi(path);
    if (!fi.exists()) {
        // deleted: delayed check to ensure it's not just rewritten (or renamed)
        mCheckExistance << path;
        QTimer::singleShot(100, this, &FileMetaRepo::reviewMissing);
    } else {
        // changedExternally
        FileEvent e(file->id(), FileEvent::Kind::changedExtern);
        emit fileEvent(e);
    }
    // TODO(JM) stack file-name to check after timeout if it's deleted or contents has changed
}

void FileMetaRepo::reviewMissing()
{
    while (!mCheckExistance.isEmpty()) {
        FileMeta *file = fileMeta(mCheckExistance.takeFirst());
        if (!file) continue;
        QFileInfo fi(file->location());
        if (fi.exists()) {
            FileEvent e(file->id(), FileEvent::Kind::changedExtern);
            emit fileEvent(e);
        } else {
            // (JM) About RENAME: To evaluate if a file has been renamed the directory content before the
            // change must have been stored so it can be ensured that the possible file is no recent copy
            // of the file that was removed.

            mMissList << file->location();
            FileEvent e(file->id(), FileEvent::Kind::removedExtern);
            emit fileEvent(e);
        }
    }
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
