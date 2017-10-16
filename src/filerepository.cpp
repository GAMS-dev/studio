/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "filerepository.h"
#include "exception.h"

namespace gams {
namespace studio {

FileRepository::FileRepository(QObject* parent)
    : QAbstractItemModel(parent), mNextId(0)
{
    mRoot = new FileGroupContext(nullptr, mNextId++, "Root", "", "");
    mTreeRoot = new FileGroupContext(mRoot, mNextId++, "TreeRoot", "", "");
}

FileRepository::~FileRepository()
{
    delete mRoot;
}

void FileRepository::setDefaultActions(QList<QAction*> directActions)
{
    while (!mFileActions.isEmpty()) {
        FileActionContext* fac = mFileActions.takeFirst();
        fac->deleteLater();
    }
    for (QAction *act: directActions) {
        mFileActions.append(new FileActionContext(nullptr, mNextId++, act));
    }
    updateActions();
}

FileSystemContext* FileRepository::context(int fileId, FileSystemContext* startNode)
{
    if (!startNode)
        FATAL() << "missing startNode";
    if (startNode->id() == fileId)
        return startNode;
    for (int i = 0; i < startNode->childCount(); ++i) {
        FileSystemContext* iChild = startNode->childEntry(i);
        if (!iChild)
            FATAL() << "child must not be null";
        FileSystemContext* entry = context(fileId, iChild);
        if (entry) return entry;
    }
    return nullptr;
}

FileContext* FileRepository::fileContext(int fileId, FileSystemContext* startNode)
{
    auto c = context(fileId, (startNode ? startNode : mRoot));
    if (c->type() == FileSystemContext::File)
        return static_cast<FileContext*>(c);
    return nullptr;
}

FileGroupContext* FileRepository::groupContext(int fileId, FileSystemContext* startNode)
{
    auto c = context(fileId, startNode ? startNode : mRoot);
    if (c->type() == FileSystemContext::FileGroup) {
        return static_cast<FileGroupContext*>(c);
    }
    return c->parentEntry();
}

QModelIndex FileRepository::index(FileSystemContext *entry)
{
     if (!entry)
         return QModelIndex();
     if (!entry->parent())
         return createIndex(0, 0, entry);
     for (int i = 0; i < entry->parentEntry()->childCount(); ++i) {
         if (entry->parentEntry()->childEntry(i) == entry) {
             return createIndex(i, 0, entry);
         }
     }
     return QModelIndex();
}

QModelIndex FileRepository::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    FileSystemContext *fc = context(parent)->childEntry(row);
    if (!fc)
        FATAL() << "invalid child for row " << row;
    return createIndex(row, column, fc);
}

QModelIndex FileRepository::parent(const QModelIndex& child) const
{
    FileSystemContext* eChild = context(child);
    if (eChild == mTreeRoot || eChild == mRoot)
        return QModelIndex();
    FileGroupContext* eParent = eChild->parentEntry();
    if (!eParent)
        FATAL() << "invalid parent for child " << eChild->name();
    if (eParent == mTreeRoot || eParent == mRoot)
        return createIndex(0, child.column(), eParent);
    int row = eParent->indexOf(eChild);
    if (row < 0)
        FATAL() << "could not find child in parent";
    return createIndex(row, child.column(), eParent);
}

int FileRepository::rowCount(const QModelIndex& parent) const
{
    FileSystemContext* entry = context(parent);
    if (!entry) return 0;
    return entry->childCount();
}

int FileRepository::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant FileRepository::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    switch (role) {

    case Qt::BackgroundColorRole:
        if (mCurrent && fileContext(index) == mCurrent) {
            return QColor("#4466BBFF");
        }
    case Qt::DisplayRole:
        return context(index)->caption();

    case Qt::FontRole:
        if (context(index)->flags().testFlag(FileSystemContext::cfActive)) {
            QFont f;
            f.setBold(true);
            return f;
        }
        if (context(index)->type() == FileSystemContext::FileAction) {
            QFont f;
            f.setItalic(true);
            return f;
        }
        break;

    case Qt::ForegroundRole: {
        FileSystemContext::ContextFlags flags = context(index)->flags();
        if (flags.testFlag(FileSystemContext::cfMissing))
            return QColor(Qt::red);
        if (flags.testFlag(FileSystemContext::cfActive)) {
            return (mCurrent && fileContext(index) == mCurrent) ? QColor(Qt::blue)
                                                                : QColor(Qt::black);
        }
        break;
    }

    case Qt::DecorationRole:
        return context(index)->icon();

    case Qt::ToolTipRole:
        return context(index)->location();

    default:
        break;
    }
    return QVariant();
}

QModelIndex FileRepository::findEntry(QString name, QString location, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    FileGroupContext *par = groupContext(parentIndex);
    if (!par)
        FATAL() << "Can't get parent object";

    bool hit;
    int offset = par->peekIndex(name, &hit);
    if (hit) {
        FileSystemContext *fc = par->childEntry(offset);
        if (fc->location().compare(location, Qt::CaseInsensitive) == 0) {
            return index(offset, 0, parentIndex);
        }
    }
    return QModelIndex();
}

FileSystemContext* FileRepository::findFile(QString filePath)
{
    FileSystemContext* fsc = mTreeRoot->findFile(filePath);
    return fsc;
}

QList<FileContext*> FileRepository::openFiles(FileGroupContext *fileGroup)
{
    if (!fileGroup)
        fileGroup = mTreeRoot;
    QList<FileContext*> res;
    for (int i = 0; i < fileGroup->childCount(); ++i) {
        if (fileGroup->childEntry(i)->type() == FileSystemContext::FileGroup) {
            FileGroupContext *fgc = static_cast<FileGroupContext*>(fileGroup->childEntry(i));
            QList<FileContext*> sub = openFiles(fgc);
            for (FileContext *fc : sub) {
                if (!res.contains(fc)) {
                    res << fc;
                }
            }
        }
        if (fileGroup->childEntry(i)->type() == FileSystemContext::File) {
            FileContext *fc = static_cast<FileContext*>(fileGroup->childEntry(i));
            if (fc->crudState() == CrudState::eUpdate) {
                res << fc;
            }
        }
    }
    return res;
}

QModelIndex FileRepository::addGroup(QString name, QString location, QString runInfo, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootTreeModelIndex();
    FileGroupContext *par = groupContext(parentIndex);
    if (!par)
        FATAL() << "Can't get parent object";

    bool hit;
    int offset = par->peekIndex(name, &hit);
    if (hit) {
        FileSystemContext *fc = par->childEntry(offset);
        fc->setLocation(location);
        qDebug() << "found dir " << name << " for " << location << " at pos=" << offset;
        return index(offset, 0, parentIndex);
    }

    beginInsertRows(parentIndex, offset, offset);
    FileGroupContext* fgContext = new FileGroupContext(groupContext(parentIndex), mNextId++, name, location, runInfo);
    endInsertRows();
    connect(fgContext, &FileGroupContext::changed, this, &FileRepository::nodeChanged);
    connect(fgContext, &FileGroupContext::contentChanged, this, &FileRepository::updatePathNode);
    qDebug() << "added dir " << name << " for " << location << " at pos=" << offset;
//    updatePathNode(fgContext->id(), fgContext->location());
    updateActions();
    return index(offset, 0, parentIndex);
}

QModelIndex FileRepository::addFile(QString name, QString location, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    FileGroupContext *par = groupContext(parentIndex);
    if (!par)
        FATAL() << "Can't get parent object";

    bool hit;
    int offset = par->peekIndex(name, &hit);
    if (hit) {
        FileSystemContext *fc = par->childEntry(offset);
        fc->setLocation(location);
        fc->setFlag(FileSystemContext::cfMissing, !QFile(location).exists());
        qDebug() << "found file " << name << " for " << location << " at pos=" << offset;
        return index(offset, 0, parentIndex);
    }
    beginInsertRows(parentIndex, offset, offset);
    FileContext* fContext = new FileContext(groupContext(parentIndex), mNextId++, name, location);
    endInsertRows();
    connect(fContext, &FileGroupContext::changed, this, &FileRepository::nodeChanged);
    connect(fContext, &FileContext::modifiedExtern, this, &FileRepository::onFileChangedExtern);
    connect(fContext, &FileContext::deletedExtern, this, &FileRepository::onFileDeletedExtern);
    qDebug() << "added file " << name << " for " << location << " at pos=" << offset;
    updateActions();
    return index(offset, 0, parentIndex);
}

void FileRepository::removeNode(FileSystemContext* node)
{
    QModelIndex mi = index(node);
    int ind = node->parentEntry()->indexOf(node);
    beginRemoveRows(parent(mi), ind, ind);
    node->setParentEntry(nullptr);
    delete node;
    endRemoveRows();
}

QModelIndex FileRepository::rootTreeModelIndex()
{
    return createIndex(0, 0, mTreeRoot);
}

QModelIndex FileRepository::rootModelIndex()
{
    return createIndex(0, 0, mRoot);
}

QModelIndex FileRepository::ensureGroup(const QString &filePath)
{
    bool extendedCaption = false;
    QFileInfo fi(filePath);
    QFileInfo di(fi.path());
    for (int i = 0; i < mTreeRoot->childCount(); ++i) {
        FileSystemContext* group = mTreeRoot->childEntry(i);
        if (!group)
            FATAL() << "invalid element at index " << i << " in TreeRoot";
        if (fi.baseName() == group->name()) {
            // (name,location)-group exists? -> return index
            if (di == QFileInfo(group->location()))
                return createIndex(i, 0, group);
            else {
                extendedCaption = true;
                group->setFlag(FileSystemContext::cfExtendCaption);
            }
        }
    }
    QModelIndex newGroupMi = addGroup(fi.baseName(), fi.path(), fi.fileName(), rootTreeModelIndex());
    if (extendedCaption)
        groupContext(newGroupMi)->setFlag(FileSystemContext::cfExtendCaption);
    updatePathNode(groupContext(newGroupMi)->id(), fi.path());
    return newGroupMi;
}

void FileRepository::close(int fileId)
{
    FileContext *fc = fileContext(fileId);
    QModelIndex fci = index(fc);
    dataChanged(fci, fci);
    emit fileClosed(fileId, QPrivateSignal());
}

void FileRepository::setSuffixFilter(QStringList filter)
{
    for (QString suff: filter) {
        if (!suff.startsWith("."))
            EXCEPT() << "invalid suffix " << suff << ". A suffix must start with a dot.";
    }
    mSuffixFilter = filter;
}

void FileRepository::dump(FileSystemContext *fc, int lv)
{
    if (!fc) return;

    qDebug() << QString("  ").repeated(lv) + "+ " + fc->location() + "  (" + fc->name() + ")";
    FileGroupContext *gc = qobject_cast<FileGroupContext*>(fc);
    if (!gc) return;
    for (int i=0 ; i < gc->childCount() ; i++) {
        FileSystemContext *child = gc->childEntry(i);
        dump(child, lv+1);
    }
}

void FileRepository::nodeChanged(int fileId)
{
    FileSystemContext* nd = context(fileId, mRoot);
    if (!nd) return;
    QModelIndex ndIndex = index(nd);
    dataChanged(ndIndex, ndIndex);
}

typedef QPair<int, FileSystemContext*> IndexedFSContext;

void FileRepository::updatePathNode(int fileId, QDir dir)
{
    qDebug() << "updatePathNode: " << dir;
    FileGroupContext *parGroup = groupContext(fileId, mRoot);
    if (!parGroup)
        throw QException();
    if (dir.exists()) {
        QStringList fileFilter;
        for (QString suff: mSuffixFilter)
            fileFilter << parGroup->name() + suff;
        QFileInfoList fiList = dir.entryInfoList(fileFilter, QDir::Files, QDir::Name);

        // remove known entries from fileList and remember vanished entries
        QList<IndexedFSContext> vanishedEntries;
        for (int i = 0; i < parGroup->childCount(); ++i) {
            FileSystemContext *entry = parGroup->childEntry(i);
            QFileInfo fi(entry->location());
            int pos = fiList.indexOf(fi);
            if (pos >= 0) {
                fiList.removeAt(pos);
                entry->unsetFlag(FileSystemContext::cfMissing);
            } else {
                // prepare indicees in reverse order (highest index first)
                vanishedEntries.insert(0, IndexedFSContext(i, entry));
            }
        }
        // check for vanished files and directories
        for (IndexedFSContext childIndex: vanishedEntries) {
            FileSystemContext* entry = childIndex.second;
            if (entry->testFlag(FileSystemContext::cfActive)) {
                // mark active files as missing (directories recursively)
                entry->setFlag(FileSystemContext::cfMissing);
                qDebug() << "Missing node: " << entry->name();
            } else {
                // inactive files can be removed (directories recursively)
                removeNode(entry);
            }
        }
        // add newly appeared files and directories
        for (QFileInfo fi: fiList) {
            addFile(fi.fileName(), fi.canonicalFilePath(), index(parGroup));
        }
    }
}

void FileRepository::nodeClicked(QModelIndex index)
{
    FileActionContext* act = actionContext(index);
    if (act) {
        emit act->trigger();
    }
}

void FileRepository::editorActivated(QPlainTextEdit* edit)
{
    FileContext *fc = fileContext(edit);
    if (fc && fc != mCurrent) {
        QModelIndex mi = index(mCurrent);
        mCurrent = fc;
        if (mi.isValid()) dataChanged(mi, mi);    // invalidate old
        mi = index(mCurrent);
        if (mi.isValid()) dataChanged(mi, mi);    // invalidate new
    }
}

void FileRepository::onFileChangedExtern(int fileId)
{
    if (!mChangedIds.contains(fileId)) mChangedIds << fileId;
    QTimer::singleShot(100, this, &FileRepository::processExternFileEvents);
}

void FileRepository::onFileDeletedExtern(int fileId)
{
    if (!mDeletedIds.contains(fileId)) mDeletedIds << fileId;
    QTimer::singleShot(100, this, &FileRepository::processExternFileEvents);
}

void FileRepository::processExternFileEvents()
{
    while (!mDeletedIds.isEmpty()) {
        int fileId = mDeletedIds.takeFirst();
        if (mChangedIds.contains(fileId)) mChangedIds.removeAll(fileId);
        emit fileDeletedExtern(fileId);
    }
    while (!mChangedIds.isEmpty()) {
        int fileId = mChangedIds.takeFirst();
        emit fileChangedExtern(fileId);
    }
}

void FileRepository::updateActions()
{
    if (mFileActions.isEmpty())
        return;
    bool actionsVisibleOld = mFileActions.first()->parentEntry();
    bool actionsVisibleNew = true;
    for (int i = 0; i < mTreeRoot->childCount(); i++) {
        if (mTreeRoot->childEntry(i)->type() != FileSystemContext::FileAction) {
            actionsVisibleNew = false;
        }
    }
    if (actionsVisibleNew != actionsVisibleOld) {
        QModelIndex treeRootMI = index(mTreeRoot);
        if (actionsVisibleNew) {
            beginInsertRows(treeRootMI, 0, 1);
            for (FileActionContext *fac: mFileActions) {
                fac->setParentEntry(mTreeRoot);
            }
            endInsertRows();
        } else {
            for (int i = mTreeRoot->childCount()-1; i >= 0; --i) {
                FileSystemContext *fsc = mTreeRoot->childEntry(i);
                if (fsc->type() == FileSystemContext::FileAction) {
                    beginRemoveRows(treeRootMI, i, i);
                    fsc->setParentEntry(nullptr);
                    endRemoveRows();
                }
            }
        }
    }
}

FileSystemContext*FileRepository::context(const QModelIndex& index) const
{
    return static_cast<FileSystemContext*>(index.internalPointer());
}

FileContext*FileRepository::fileContext(const QModelIndex& index) const
{
    FileSystemContext *fsc = context(index);
    if (fsc->type() != FileSystemContext::File)
        return nullptr;
    return static_cast<FileContext*>(fsc);
}

FileContext* FileRepository::fileContext(QPlainTextEdit* edit)
{
    for (int i = 0; i < mTreeRoot->childCount(); ++i) {
        FileSystemContext *group = mTreeRoot->childEntry(i);
        for (int j = 0; j < group->childCount(); ++j) {
            FileContext *file = qobject_cast<FileContext*>(group->childEntry(j));
            if (file && file->hasEditor(edit)) {
                return file;
            }
        }
    }
    return nullptr;
}

FileGroupContext*FileRepository::groupContext(const QModelIndex& index) const
{
    FileSystemContext *fsc = context(index);
    if (fsc->type() != FileSystemContext::FileGroup)
        return nullptr;
    return static_cast<FileGroupContext*>(fsc);
}

FileActionContext*FileRepository::actionContext(const QModelIndex& index) const
{
    FileSystemContext *fsc = context(index);
    if (fsc->type() != FileSystemContext::FileAction)
        return nullptr;
    return static_cast<FileActionContext*>(fsc);
}

QList<QPlainTextEdit*> FileRepository::editors(int fileId)
{
    FileSystemContext* fsc = (fileId < 0 ? mTreeRoot : context(fileId, mTreeRoot));
    if (!fsc) return QList<QPlainTextEdit*>();

    if (fsc->type() == FileSystemContext::FileGroup) {
        // TODO(JM) gather all editors of the group
        QList<QPlainTextEdit*> allEdits;
        FileGroupContext* group = static_cast<FileGroupContext*>(fsc);
        for (int i = 0; i < group->childCount(); ++i) {
            QList<QPlainTextEdit*> groupEdits = editors(group->childEntry(i)->id());
            for (QPlainTextEdit* ed: groupEdits) {
                if (!allEdits.contains(ed))
                    allEdits << ed;
            }
        }
        return allEdits;
    }
    if (fsc->type() == FileSystemContext::File) {
        return static_cast<FileContext*>(fsc)->editors();
    }
    return QList<QPlainTextEdit*>();
}

} // namespace studio
} // namespace gams
