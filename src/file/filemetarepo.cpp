/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "settings.h"
#include "exception.h"
#include "logger.h"
#include "viewhelper.h"
#include <QFileInfo>

namespace gams {
namespace studio {

FileMetaRepo::FileMetaRepo(QObject *parent) : QObject(parent)
{
    connect(&mWatcher, &QFileSystemWatcher::fileChanged,
            this, &FileMetaRepo::fileChanged);
    mMissCheckTimer.setInterval(5000);
    mMissCheckTimer.setSingleShot(true);
    connect(&mMissCheckTimer, &QTimer::timeout, this, &FileMetaRepo::checkMissing);
    mSettings = Settings::settings();
#ifdef __unix__
    mFsCaseSensitive = true;
#endif
}

FileMeta *FileMetaRepo::fileMeta(const FileId &fileId) const
{
    return mFiles.value(fileId, nullptr);
}

FileMeta *FileMetaRepo::fileMeta(const QString &location) const
{
    QFileInfo fi(location);
    if (!fi.exists())
        return nullptr;
    if (mFsCaseSensitive)
        return mFileNames.value(location);
    return mFileNames.value(location.toLower());
}

FileMeta *FileMetaRepo::fileMeta(QWidget* const &editor) const
{
    if (!editor) return nullptr;
    return fileMeta(editor->property("location").toString());
}

const QList<FileMeta *> FileMetaRepo::fileMetas() const
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
    if (mFsCaseSensitive)
        mFileNames.insert(fileMeta->location(), fileMeta);
    else
        mFileNames.insert(fileMeta->location().toLower(), fileMeta);
    watch(fileMeta);
}

bool FileMetaRepo::askBigFileEdit() const
{
    return mAskBigFileEdit;
}

void FileMetaRepo::setAskBigFileEdit(bool askBigFileEdit)
{
    mAskBigFileEdit = askBigFileEdit;
}

void FileMetaRepo::removeFile(FileMeta *fileMeta)
{
    if (fileMeta) {
        mFiles.remove(fileMeta->id());
        if (mFsCaseSensitive)
            mFileNames.remove(fileMeta->location());
        else
            mFileNames.remove(fileMeta->location().toLower());
        unwatch(fileMeta);
        if (fileMeta->isAutoReload())
            mAutoReloadLater << fileMeta->location();
    }
}

void FileMetaRepo::toggleBookmark(FileId fileId, int lineNr, int posInLine)
{
    if (mTextMarkRepo->marks(fileId, lineNr, -1, TextMark::bookmark, 1).isEmpty()) {
        // add bookmark
        mTextMarkRepo->createMark(fileId, TextMark::bookmark, lineNr, posInLine);
    } else {
        // remove bookmark
        mTextMarkRepo->removeMarks(fileId, QSet<TextMark::Type>() << TextMark::bookmark, lineNr);
    }
}

void FileMetaRepo::jumpToNextBookmark(bool back, FileId refFileId, int refLineNr)
{
    TextMark *bookmark = nullptr;
    if (mTextMarkRepo->hasBookmarks(refFileId)) {
        bookmark = mTextMarkRepo->findBookmark(refFileId, refLineNr, back);
    }
    FileMeta *fm = fileMeta(refFileId);
    ProjectAbstractNode * startNode = fm ? mProjectRepo->findFile(fm) : nullptr;
    ProjectAbstractNode * node = startNode;
    while (!bookmark && startNode) {
        node = back ? mProjectRepo->previous(node) : mProjectRepo->next(node);
        FileId fileId = node->toFile() ? node->toFile()->file()->id() : FileId();
        if (fileId.isValid() && mTextMarkRepo->hasBookmarks(fileId)) {
            bookmark = mTextMarkRepo->findBookmark(fileId, -1, back);
        }
        if (node == startNode) break;
    }

    if (bookmark) bookmark->jumpToMark();
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
        if (i.value()->isOpen() && res.indexOf(i.value()) == -1) res << i.value();
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
    unwatch(fileMeta->location());
}

void FileMetaRepo::unwatch(const QString &filePath)
{
    if (filePath.isEmpty()) return;
    mWatcher.removePath(filePath);
    mMissList.removeAll(filePath);
    if (mMissList.isEmpty()) mMissCheckTimer.stop();
}

bool FileMetaRepo::watch(const FileMeta *fileMeta)
{
    if (fileMeta->exists(true)) {
        mWatcher.addPath(fileMeta->location());
        return true;
    }
    mMissList << fileMeta->location();
    if (!mMissCheckTimer.isActive())
        mMissCheckTimer.start();
    return false;
}

void FileMetaRepo::setAutoReload(const QString &location)
{
    FileMeta * meta = fileMeta(location);
    if (meta)
        meta->setAutoReload();
    else if (!mAutoReloadLater.contains(location))
        mAutoReloadLater << location;
}

void FileMetaRepo::setDebugMode(bool debug)
{
    mDebug = debug;
    if (!debug) return;
//    DEB() << "\n--------------- FileMetas (Editors) ---------------";
    QMap<int, AbstractEdit*> edits;
    for (QWidget* wid: editors()) {
        AbstractEdit*ed = ViewHelper::toAbstractEdit(wid);
        if (ed) edits.insert(int(ViewHelper::fileId(ed)), ed);
    }

    for (int key: edits.keys()) {
        FileMeta* fm = fileMeta(FileId(key));
        QString nam = (fm ? fm->name() : "???");
//        DEB() << key << ": " << edits.value(key)->size() << "    " << nam;
    }
    mCompleter.setDebugMode(debug);
}

bool FileMetaRepo::debugMode() const
{
    return mDebug;
}

bool FileMetaRepo::equals(const QFileInfo &fi1, const QFileInfo &fi2)
{
    return (fi1.exists() || fi2.exists()) ? fi1 == fi2 : fi1.absoluteFilePath() == fi2.absoluteFilePath();
}

void FileMetaRepo::updateRenamed(FileMeta *file, QString oldLocation)
{
    if (mFsCaseSensitive) {
        mFileNames.remove(oldLocation);
        mFileNames.insert(file->location(), file);
    } else {
        mFileNames.remove(oldLocation.toLower());
        mFileNames.insert(file->location().toLower(), file);
    }
}

void FileMetaRepo::setUserGamsTypes(QStringList suffix)
{
    QStringList changed;
    for (const QString &suf : suffix) {
        if (!FileType::userGamsTypes().contains(suf)) changed << suf;
    }
    for (const QString &suf : FileType::userGamsTypes()) {
        if (!suffix.contains(suf)) changed << suf;
    }
    FileType::setUserGamsTypes(suffix);
    QVector<ProjectRunGroupNode*> runGroups;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        QFileInfo fi(i.value()->location());
        if (changed.contains(fi.suffix())) {
            i.value()->refreshType();
            for (ProjectAbstractNode *node : mProjectRepo->fileNodes(i.value()->id()))
                if (!runGroups.contains(node->assignedRunGroup())) runGroups << node->assignedRunGroup();
        }
    }
    for (ProjectRunGroupNode * group : runGroups) {
        if (!group->runnableGms() || group->runnableGms()->kind() != FileKind::Gms) {
            group->setRunnableGms();
        }
    }
}

void FileMetaRepo::openFile(FileMeta *fm, NodeId groupId, bool focus, int codecMib)
{
    if (!mProjectRepo) EXCEPT() << "Missing initialization. Method init() need to be called.";
    ProjectRunGroupNode* runGroup = mProjectRepo->findRunGroup(groupId);
    emit mProjectRepo->openFile(fm, focus, runGroup, codecMib);
}

void FileMetaRepo::fileChanged(const QString &path)
{
    FileMeta *file = fileMeta(path);
    if (!file) return;
    mProjectRepo->fileChanged(file->id());
    QFileInfo fi(path);
    if (!fi.exists()) {
        // deleted: delayed check to ensure it's not just rewritten (or renamed)
        mRemoved << path;
        QTimer::singleShot(100, this, &FileMetaRepo::reviewRemoved);
    } else {
        // changedExternally
        watch(file); // sometimes the watcher looses the entry
        if (file->compare(path)) {
            FileEvent e(file->id(), FileEventKind::changedExtern);
            file->updateView();
            emit fileEvent(e);
        }
    }
}

void FileMetaRepo::reviewRemoved()
{
    while (!mRemoved.isEmpty()) {
        FileMeta *file = fileMeta(mRemoved.takeFirst());
        if (!file) continue;
        mProjectRepo->fileChanged(file->id());
        if (watch(file)) {
            FileMeta::FileDifferences diff = file->compare();
            if (diff.testFlag(FileMeta::FdMissing)) {
                FileEvent e(file->id(), FileEventKind::removedExtern);
                emit fileEvent(e);
            } else if (diff) {
                file->refreshMetaData();
                FileEventKind feKind = FileEventKind::changedExtern;
                FileEvent e(file->id(), feKind);
                file->updateView();
                emit fileEvent(e);
            }
        } else {
            // (JM) About RENAME: To evaluate if a file has been renamed
            // the directory content before the change must have been stored
            // so it can be ensured that the possible file is no recent copy
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
        FileMeta *file = fileMeta(fileName);
        if (!file) continue;
        mProjectRepo->fileChanged(file->id());
        if (QFileInfo(fileName).exists()) {
            watch(file);
            file->refreshMetaData();
            FileEventKind feKind = FileEventKind::changedExtern;
            FileEvent e(file->id(), feKind);
            file->updateView();
            emit fileEvent(e);
        } else {
            remainMissList << fileName;
        }
    }
    if (!remainMissList.isEmpty()) {
        mMissList = remainMissList;
        mMissCheckTimer.start();
    }
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
    QString adaptedLocation;
    FileMeta* res = fileMeta(location);
    if (!res) {
        res = new FileMeta(this, mNextFileId++, location, knownType);
        connect(res, &FileMeta::editableFileSizeCheck, this, &FileMetaRepo::editableFileSizeCheck);
        addFileMeta(res);
        if (mAutoReloadLater.contains(location)) {
            mAutoReloadLater.removeAll(location);
            res->setAutoReload();
        }
    }
    return res;
}

} // namespace studio
} // namespace gams
