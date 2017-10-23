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
    : QObject(parent), mNextId(0), mTreeModel(new FileTreeModel(this, new FileGroupContext(mNextId++, "Root", "", "")))
{}

FileRepository::~FileRepository()
{
    FileType::clear(); // TODO(JM) There may be a better place to clear the static type-list.
    delete mTreeModel;
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
        break;
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

void FileRepository::setDefaultActions(QList<QAction*> directActions)
{
    while (!mFileActions.isEmpty()) {
        FileActionContext* fac = mFileActions.takeFirst();
        fac->deleteLater();
    }
    for (QAction *act: directActions) {
        mFileActions.append(new FileActionContext(mNextId++, act));
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
    auto c = context(fileId, (startNode ? startNode : mTreeModel->rootContext()));
    if (c->type() == FileSystemContext::File)
        return static_cast<FileContext*>(c);
    return nullptr;
}

FileGroupContext* FileRepository::groupContext(int fileId, FileSystemContext* startNode)
{
    auto c = context(fileId, startNode ? startNode : mTreeModel->rootContext());
    if (c->type() == FileSystemContext::FileGroup) {
        return static_cast<FileGroupContext*>(c);
    }
    return c->parentEntry();
}

QModelIndex FileRepository::findEntry(QString name, QString location, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = mTreeModel->rootModelIndex();
    FileGroupContext *par = groupContext(parentIndex);
    if (!par)
        FATAL() << "Can't get parent object";

    bool hit;
    int offset = par->peekIndex(name, &hit);
    if (hit) {
        FileSystemContext *fc = par->childEntry(offset);
        if (fc->location().compare(location, Qt::CaseInsensitive) == 0) {
            return mTreeModel->index(offset, 0, parentIndex);
        }
    }
    return QModelIndex();
}

FileSystemContext* FileRepository::findFile(QString filePath, FileGroupContext* fileGroup)
{
    FileGroupContext *group = fileGroup ? fileGroup : mTreeModel->rootContext();
    FileSystemContext* fsc = group->findFile(filePath);
    return fsc;
}

QList<FileContext*> FileRepository::modifiedFiles(FileGroupContext *fileGroup)
{
    // TODO(JM) rename this to modifiedFiles()
    if (!fileGroup)
        fileGroup = mTreeModel->rootContext();
    QList<FileContext*> res;
    for (int i = 0; i < fileGroup->childCount(); ++i) {
        if (fileGroup->childEntry(i)->type() == FileSystemContext::FileGroup) {
            FileGroupContext *fgc = static_cast<FileGroupContext*>(fileGroup->childEntry(i));
            QList<FileContext*> sub = modifiedFiles(fgc);
            for (FileContext *fc : sub) {
                if (!res.contains(fc)) {
                    res << fc;
                }
            }
        }
        if (fileGroup->childEntry(i)->type() == FileSystemContext::File) {
            FileContext *fc = static_cast<FileContext*>(fileGroup->childEntry(i));
            if (fc->isModified()) {
                res << fc;
            }
        }
    }
    return res;
}

int FileRepository::saveAll()
{
    QList<FileContext*> files = modifiedFiles();
    for (FileContext* fc: files) {
        fc->save();
    }
    return files.size();
}

FileGroupContext* FileRepository::addGroup(QString name, QString location, QString runInfo, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = mTreeModel->rootModelIndex();
    FileGroupContext *par = groupContext(parentIndex);
    if (!par)
        FATAL() << "Can't get parent object";

    bool hit;
    int offset = par->peekIndex(name, &hit);
    if (hit) offset++;
    FileGroupContext* fgContext = new FileGroupContext(mNextId++, name, location, runInfo);
    mTreeModel->insertChild(offset, groupContext(parentIndex), fgContext);
    connect(fgContext, &FileGroupContext::changed, this, &FileRepository::nodeChanged);
    connect(fgContext, &FileGroupContext::contentChanged, this, &FileRepository::updatePathNode);
    fgContext->setWatched();
    qDebug() << "added dir " << name << " for " << location << " at pos=" << offset;
    updateActions();
    return fgContext;
}

FileContext* FileRepository::addFile(QString name, QString location, FileGroupContext* parent)
{
    if (!parent)
        parent = mTreeModel->rootContext();
    bool hit;
    int offset = parent->peekIndex(name, &hit);
    if (hit)
        FATAL() << "The group '" << parent->name() << "' already contains '" << name << "'";
    FileContext* fileContext = new FileContext(mNextId++, name, location);
    mTreeModel->insertChild(offset, parent, fileContext);
    connect(fileContext, &FileGroupContext::changed, this, &FileRepository::nodeChanged);
    connect(fileContext, &FileContext::modifiedExtern, this, &FileRepository::onFileChangedExtern);
    connect(fileContext, &FileContext::deletedExtern, this, &FileRepository::onFileDeletedExtern);
    qDebug() << "added file " << name << " for " << location << " at pos=" << offset;
    updateActions();
    return fileContext;
}

void FileRepository::removeNode(FileSystemContext* node)
{
    mTreeModel->removeChild(node);
    delete node;
}

FileGroupContext* FileRepository::ensureGroup(const QString &filePath)
{
    bool extendedCaption = false;
    FileGroupContext* group = nullptr;
    QFileInfo fi(filePath);
    QFileInfo di(fi.canonicalPath());
    for (int i = 0; i < mTreeModel->rootContext()->childCount(); ++i) {
        FileSystemContext* fsc = mTreeModel->rootContext()->childEntry(i);
        if (fsc && fsc->type() == FileSystemContext::FileGroup && fsc->name() == fi.completeBaseName()) {
            group = static_cast<FileGroupContext*>(fsc);
            if (di == QFileInfo(group->location())) {
                return group;
            } else {
                extendedCaption = true;
                group->setFlag(FileSystemContext::cfExtendCaption);
            }
        }
    }
    group = addGroup(fi.completeBaseName(), fi.path(), fi.fileName(), mTreeModel->rootModelIndex());
    if (extendedCaption)
        group->setFlag(FileSystemContext::cfExtendCaption);
    updatePathNode(group->id(), fi.path());
    return group;
}

void FileRepository::close(int fileId)
{
    FileContext *fc = fileContext(fileId);
    QModelIndex fci = mTreeModel->index(fc);
    mTreeModel->dataChanged(fci, fci);
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
    FileSystemContext* nd = context(fileId, mTreeModel->rootContext());
    if (!nd) return;
    QModelIndex ndIndex = mTreeModel->index(nd);
    emit mTreeModel->dataChanged(ndIndex, ndIndex);
}

typedef QPair<int, FileSystemContext*> IndexedFSContext;

void FileRepository::updatePathNode(int fileId, QDir dir)
{
    qDebug() << "updatePathNode: " << dir;
    FileGroupContext *parGroup = groupContext(fileId, mTreeModel->rootContext());
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
            addFile(fi.fileName(), fi.canonicalFilePath(), parGroup);
        }
    }
}

void FileRepository::nodeClicked(QModelIndex index)
{
    FileActionContext* act = actionContext(index);
    if (act) {
        emit act->trigger();
        return;
    }
}

void FileRepository::editorActivated(QPlainTextEdit* edit)
{
    FileContext *fc = fileContext(edit);
    QModelIndex mi = mTreeModel->index(fc);
    mTreeModel->setCurrent(mi);
}

FileTreeModel*FileRepository::treeModel() const
{
    return mTreeModel;
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
    for (int i = 0; i < mTreeModel->rootContext()->childCount(); i++) {
        if (mTreeModel->rootContext()->childEntry(i)->type() != FileSystemContext::FileAction) {
            actionsVisibleNew = false;
        }
    }
    if (actionsVisibleNew != actionsVisibleOld) {
        if (actionsVisibleNew) {
            int i = 0;
            for (FileActionContext *fac: mFileActions) {
                mTreeModel->insertChild(i++, mTreeModel->rootContext(), fac);
            }
        } else {
            for (FileActionContext *fac: mFileActions) {
                mTreeModel->removeChild(fac);
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
    for (int i = 0; i < mTreeModel->rootContext()->childCount(); ++i) {
        FileSystemContext *group = mTreeModel->rootContext()->childEntry(i);
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
    FileSystemContext* fsc = (fileId < 0 ? mTreeModel->rootContext() : context(fileId, mTreeModel->rootContext()));
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
