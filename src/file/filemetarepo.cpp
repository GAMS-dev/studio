/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "viewhelper.h"
#include <QFileInfo>

namespace gams {
namespace studio {

FileMetaRepo::FileMetaRepo(QObject *parent) : QObject(parent)
{
    connect(&mWatcher, &QFileSystemWatcher::fileChanged, this, &FileMetaRepo::fileChanged);
    mMissCheckTimer.setInterval(5000);
    mMissCheckTimer.setSingleShot(true);
    connect(&mMissCheckTimer, &QTimer::timeout, this, &FileMetaRepo::checkMissing);
    mSettings = Settings::settings();
}

FileMeta *FileMetaRepo::fileMeta(const FileId &fileId) const
{
    return mFiles.value(fileId, nullptr);
}

FileMeta *FileMetaRepo::fileMeta(const QString &location) const
{
    if (FileType::fsCaseSense())
        return mFileNames.value(location);
    return mFileNames.value(location.toLower());
}

FileMeta *FileMetaRepo::fileMeta(QWidget* const &editor) const
{
    if (!editor) return nullptr;
    if (editor->property("location").isValid()) {
        FileMeta *meta = fileMeta(editor->property("location").toString());
        if (!meta)
            DEB() << "FileMeta request failed for \'" << editor->property("location").toString() << "'";
        return meta;
    } else if (!editor->property("SYS").toString().isEmpty()) {
        return nullptr;
    } else {
        DEB() << "No location information for requested editor, type " << editor->objectName();
        DEB() << " - Parent: " << (editor->parentWidget() ? editor->parentWidget()->objectName() : "-none-");
        return nullptr;
    }
}

const QList<FileMeta *> FileMetaRepo::fileMetas() const
{
    QList<FileMeta*> res;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        if (i.value()->kind() != FileKind::Gsp)
            res << i.value();
    }
    return res;
}

void FileMetaRepo::addFileMeta(FileMeta *fileMeta)
{
    mFiles.insert(fileMeta->id(), fileMeta);
    if (FileType::fsCaseSense())
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
        if (FileType::fsCaseSense()) {
            // checking if the entry isn't an updated one (destructor is called with deleteLater and thus delayed)
            if (mFileNames.value(fileMeta->location()) == fileMeta)
                mFileNames.remove(fileMeta->location());
        } else {
            // checking if the entry isn't an updated one (destructor is called with deleteLater and thus delayed)
            if (mFileNames.value(fileMeta->location().toLower()) == fileMeta)
                mFileNames.remove(fileMeta->location().toLower());
        }
        unwatch(fileMeta);
        if (fileMeta->isAutoReload())
            mAutoReloadLater << fileMeta->location();
    }
}

void FileMetaRepo::toggleBookmark(const FileId &fileId, int lineNr, int posInLine)
{
    if (mTextMarkRepo->marks(fileId, lineNr, -1, TextMark::bookmark, 1).isEmpty()) {
        // add bookmark
        mTextMarkRepo->createMark(fileId, TextMark::bookmark, lineNr, posInLine);
    } else {
        // remove bookmark
        mTextMarkRepo->removeMarks(fileId, QSet<TextMark::Type>() << TextMark::bookmark, lineNr);
    }
}

void FileMetaRepo::jumpToNextBookmark(bool back, const FileId &refFileId, int refLineNr)
{
    TextMark *bookmark = nullptr;
    if (mTextMarkRepo->hasBookmarks(refFileId)) {
        bookmark = mTextMarkRepo->findBookmark(refFileId, refLineNr, back);
    }
    FileMeta *fm = fileMeta(refFileId);
    PExAbstractNode * startNode = fm ? mProjectRepo->findFile(fm) : nullptr;
    PExAbstractNode * node = startNode;
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

QList<FileMeta*> FileMetaRepo::openFiles() const
{
    QList<FileMeta*> res;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        if (i.value()->isOpen() && res.indexOf(i.value()) == -1) res << i.value();
    }
    return res;
}

QList<FileMeta *> FileMetaRepo::modifiedFiles() const
{
    QList<FileMeta*> res;
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
    if (fileMeta->kind() != FileKind::Gsp)
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
    if (fileMeta->kind() == FileKind::Log || fileMeta->kind() == FileKind::Gsp)
        return true;
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
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        for (QWidget *wid : i.value()->editors()) {
            AbstractEdit *edit = ViewHelper::toAbstractEdit(wid);
            if (edit) edits.insert(int(i.key()), edit);
        }
    }

//    for (auto it = edits.constBegin() ; it != edits.constEnd() ; ++it) {
//        FileMeta* fm = fileMeta(FileId(it.key()));
//        QString nam = (fm ? fm->name() : "???");
//        DEB() << key << ": " << edits.value(key)->size() << "    " << nam;
//    }
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

void FileMetaRepo::updateRenamed(FileMeta *file, const QString &oldLocation)
{
    if (FileType::fsCaseSense()) {
        mFileNames.remove(oldLocation);
        mFileNames.insert(file->location(), file);
    } else {
        mFileNames.remove(oldLocation.toLower());
        mFileNames.insert(file->location().toLower(), file);
    }
}

void FileMetaRepo::setUserGamsTypes(const QStringList &suffix)
{
    QStringList changed;
    for (const QString &suf : suffix) {
        if (!FileType::userGamsTypes().contains(suf, Qt::CaseInsensitive)) changed << suf;
    }
    for (const QString &suf : FileType::userGamsTypes()) {
        if (!suffix.contains(suf, Qt::CaseInsensitive)) changed << suf;
    }
    FileType::setUserGamsTypes(suffix);
    QVector<PExProjectNode*> projects;
    QHashIterator<FileId, FileMeta*> i(mFiles);
    while (i.hasNext()) {
        i.next();
        QFileInfo fi(i.value()->location());
        if (changed.contains(fi.suffix(), Qt::CaseInsensitive)) {
            i.value()->refreshType();
            const auto nodes = mProjectRepo->fileNodes(i.value()->id());
            for (PExAbstractNode *node : nodes)
                if (!projects.contains(node->assignedProject())) projects << node->assignedProject();
        }
    }
    for (PExProjectNode * project : std::as_const(projects)) {
        if (!project->runnableGms() || project->runnableGms()->kind() != FileKind::Gms) {
            project->setRunnableGms();
        }
    }
}

void FileMetaRepo::openFile(FileMeta *fm, const NodeId &groupId, bool focus, const QString &encoding)
{
    if (!mProjectRepo) EXCEPT() << "Missing initialization. Method init() need to be called.";
    PExProjectNode* project = mProjectRepo->findProject(groupId);
    emit mProjectRepo->openFile(fm, focus, project, encoding);
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
        if (QFileInfo::exists(fileName)) {
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

void FileMetaRepo::fontChangeRequest(FileMeta *fileMeta, const QFont &f)
{
    emit setGroupFontSize(fileMeta->fontGroup(), f.pointSizeF());
}

void FileMetaRepo::getIcon(QIcon &icon, FileMeta *file, const NodeId &projectId)
{
    PExProjectNode *project = mProjectRepo->asProject(projectId);
    PExAbstractNode *node = mProjectRepo->findFile(file, project);
    if (node) {
        icon = node->icon(QIcon::Selected, 40);
    } else if (project) {
        icon = project->icon(QIcon::Selected, 40);
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

FileMeta* FileMetaRepo::findOrCreateFileMeta(const QString &location, FileType *knownType)
{
    if (location.isEmpty()) return nullptr;
    FileMeta* res = fileMeta(location);
    if (!res) {
        res = new FileMeta(this, mNextFileId++, location, knownType);
        connect(res, &FileMeta::editableFileSizeCheck, this, &FileMetaRepo::editableFileSizeCheck);
        connect(res, &FileMeta::fontChangeRequest, this, &FileMetaRepo::fontChangeRequest);
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
