/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "filemetarepo.h"
#include "filemeta.h"
#include "projectrepo.h"
#include "syntax/textmarkrepo.h"
#include "studiosettings.h"
#include "exception.h"
#include "logger.h"
#include <QFileInfo>

namespace gams {
namespace studio {

FileMetaRepo::FileMetaRepo(QObject *parent, StudioSettings *settings) : QObject(parent), mSettings(settings)
{
//    connect(&mWatcher, &QFileSystemWatcher::directoryChanged, this, &FileMetaRepo::dirChanged);
    connect(&mWatcher, &QFileSystemWatcher::fileChanged, this, &FileMetaRepo::fileChanged);
    mMissCheckTimer.setInterval(5000);
    mMissCheckTimer.setSingleShot(true);
    connect(&mMissCheckTimer, &QTimer::timeout, this, &FileMetaRepo::checkMissing);
}

FileMeta *FileMetaRepo::fileMeta(const FileId &fileId) const
{
    return mFiles.value(fileId, nullptr);
}

FileMeta *FileMetaRepo::fileMeta(const QString &location) const
{
    QFileInfo fi(location);
    for (FileMeta* fm: mFiles.values()) {
        if (fi.exists() && (QFileInfo(fm->location()) == fi || location == fm->location()))
            return fm;
    }
//    if (location.startsWith('[')) { // special instances (e.g. "[LOG]123" )
//        for (FileMeta* fm: mFiles.values()) {
//            if (fm->location() == location) return fm;
//        }
//    } else {
//    }
    return nullptr;
}

FileMeta *FileMetaRepo::fileMeta(QWidget* const &editor) const
{
    if (!editor) return nullptr;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        if (i.value()->hasEditor(editor)) return i.value();
    }
    return nullptr;
}

QList<FileMeta*> FileMetaRepo::fileMetas() const
{
    QList<FileMeta*> res;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        res << i.value();
    }
    return res;
}

void FileMetaRepo::addFileMeta(FileMeta *fileMeta)
{
    mFiles.insert(fileMeta->id(), fileMeta);
    if (!fileMeta->location().startsWith('[')) {
        watch(fileMeta);
    }
}

void FileMetaRepo::removedFile(FileMeta *fileMeta)
{
    if (fileMeta && !fileMeta->location().startsWith('[')) {
        mFiles.remove(fileMeta->id());
        unwatch(fileMeta);
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

QVector<FileMeta*> FileMetaRepo::modifiedFiles() const
{
    QVector<FileMeta*> res;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        if (i.value()->isModified()) res << i.value();
    }
    return res;
}

QWidgetList FileMetaRepo::editors() const
{
    QWidgetList res;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        res << i.value()->editors();
    }
    return res;
}

void FileMetaRepo::unwatch(const FileMeta *fileMeta)
{
    mWatcher.removePath(fileMeta->location());
    mMissList.removeAll(fileMeta->location());
    if (mMissList.isEmpty()) mMissCheckTimer.stop();
}

bool FileMetaRepo::watch(const FileMeta *fileMeta)
{
    if (fileMeta->exists(true)) {
        mWatcher.addPath(fileMeta->location());
        return true;
    }
    mMissList << fileMeta->location();
    if (!mMissCheckTimer.isActive()) mMissCheckTimer.start();
    return false;
}

void FileMetaRepo::openFile(FileMeta *fm, NodeId groupId, bool focus, int codecMib)
{
    if (!mProjectRepo) EXCEPT() << "Missing initialization. Method init() need to be called.";
    ProjectRunGroupNode* runGroup = mProjectRepo->findRunGroup(groupId);
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
        mRemoved << path;
        QTimer::singleShot(100, this, &FileMetaRepo::reviewRemoved);
    } else {
        // changedExternally
        if (file->compare(path)) {
            FileEvent e(file->id(), FileEventKind::changedExtern);
            emit fileEvent(e);
        }
    }
}

void FileMetaRepo::reviewRemoved()
{
    while (!mRemoved.isEmpty()) {
        FileMeta *file = fileMeta(mRemoved.takeFirst());
        if (!file) continue;
        if (watch(file)) {
            FileDifferences diff = file->compare();
            if (diff.testFlag(FdMissing)) {
                FileEvent e(file->id(), FileEventKind::removedExtern);
                emit fileEvent(e);
            } else if (diff) {
                FileEvent e(file->id(), FileEventKind::changedExtern);
                emit fileEvent(e);
            }
        } else {
            // (JM) About RENAME: To evaluate if a file has been renamed the directory content before the
            // change must have been stored so it can be ensured that the possible file is no recent copy
            // of the file that was removed.
            FileEvent e(file->id(), FileEventKind::removedExtern);
            emit fileEvent(e);
        }
    }
}

void FileMetaRepo::checkMissing()
{
    QStringList remainMissList;
    while (!mMissList.isEmpty()) {
        QString fileName = mMissList.takeFirst();
        if (QFileInfo(fileName).exists()) {
            FileMeta *file = fileMeta(fileName);
            if (file) {
                FileEvent e(file->id(), FileEventKind::changed);
                emit fileEvent(e);
            }
        } else {
            remainMissList << fileName;
        }
    }
    if (!remainMissList.isEmpty()) {
        mMissList = remainMissList;
        mMissCheckTimer.start();
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

FileMeta* FileMetaRepo::findOrCreateFileMeta(QString location, FileType *knownType)
{
    if (location.isEmpty()) return nullptr;
    FileMeta* res = fileMeta(location);
    if (!res) {
        res = new FileMeta(this, mNextFileId++, location, knownType);
        addFileMeta(res);
    }
    return res;
}

} // namespace studio
} // namespace gams
