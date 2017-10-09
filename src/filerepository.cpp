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

FileSystemContext* FileRepository::context(int fileId, FileSystemContext* startNode)
{
    if (!startNode)
        throw FATAL() << "missing startNode";
    if (startNode->id() == fileId)
        return startNode;
    for (int i = 0; i < startNode->childCount(); ++i) {
        FileSystemContext* iChild = startNode->childEntry(i);
        if (!iChild)
            throw FATAL() << "child must not be null";
        FileSystemContext* entry = context(fileId, iChild);
        if (entry) return entry;
    }
    return nullptr;
}

FileContext* FileRepository::fileContext(int fileId, FileSystemContext* startNode)
{
    return static_cast<FileContext*>(context(fileId, (startNode ? startNode : mRoot)));
}

FileGroupContext*FileRepository::groupContext(int fileId, FileSystemContext* startNode)
{
    return static_cast<FileGroupContext*>(context(fileId, (startNode ? startNode : mRoot)));
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
    FileSystemContext *fc = node(parent)->childEntry(row);
    if (!fc)
        FATAL() << "invalid child for row " << row;
    return createIndex(row, column, fc);
}

QModelIndex FileRepository::parent(const QModelIndex& child) const
{
    FileSystemContext* eChild = node(child);
    if (eChild == mTreeRoot || eChild == mRoot)
        return QModelIndex();
    FileGroupContext* eParent = eChild->parentEntry();
    if (!eParent)
        FATAL() << "invalid parent for child " << eChild->name();
    if (eParent == mTreeRoot || eParent == mRoot)
        return createIndex(0, child.column(), eParent);
    int row = eParent->indexOf(eChild);
    if (row < 0)
        throw FATAL() << "could not find child in parent";
    return createIndex(row, child.column(), eParent);
}

int FileRepository::rowCount(const QModelIndex& parent) const
{
    FileSystemContext* entry = node(parent);
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

    case Qt::DisplayRole:
        return node(index)->caption();

    case Qt::FontRole:
        if (node(index)->flags().testFlag(FileSystemContext::cfActive)) {
            QFont f;
            f.setBold(true);
            return f;
        }
        break;

    case Qt::ForegroundRole: {
        FileSystemContext::ContextFlags flags = node(index)->flags();
        if (flags.testFlag(FileSystemContext::cfMissing))
            return QColor(Qt::red);
        if (flags.testFlag(FileSystemContext::cfActive)) {
            return flags.testFlag(FileSystemContext::cfGroup) ? QColor(Qt::black)
                                                              : QColor(Qt::blue);
        }
        break;
    }

    case Qt::DecorationRole:
        return node(index)->icon();

    case Qt::ToolTipRole:
        return node(index)->location();

    default:
        break;
    }
    return QVariant();
}

QModelIndex FileRepository::findEntry(QString name, QString location, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    FileGroupContext *par = group(parentIndex);
    if (!par)
        throw FATAL() << "Can't get parent object";

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

QModelIndex FileRepository::addGroup(QString name, QString location, QString runInfo, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootTreeModelIndex();
    FileGroupContext *par = group(parentIndex);
    if (!par)
        throw FATAL() << "Can't get parent object";

    bool hit;
    int offset = par->peekIndex(name, &hit);
    if (hit) {
        FileSystemContext *fc = par->childEntry(offset);
        fc->setLocation(location);
        qDebug() << "found dir " << name << " for " << location << " at pos=" << offset;
        return index(offset, 0, parentIndex);
    }

    beginInsertRows(parentIndex, offset, offset);
    FileGroupContext* fgContext = new FileGroupContext(group(parentIndex), mNextId++, name, location, runInfo);
    endInsertRows();
    connect(fgContext, &FileGroupContext::changed, this, &FileRepository::nodeChanged);
    qDebug() << "added dir " << name << " for " << location << " at pos=" << offset;
//    updatePathNode(fgContext->id(), fgContext->location());
    return index(offset, 0, parentIndex);
}

QModelIndex FileRepository::addFile(QString name, QString location, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    FileGroupContext *par = group(parentIndex);
    if (!par)
        throw FATAL() << "Can't get parent object";

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
    FileContext* fContext = new FileContext(group(parentIndex), mNextId++, name, location);
    endInsertRows();
    connect(fContext, &FileGroupContext::changed, this, &FileRepository::nodeChanged);
    qDebug() << "added file " << name << " for " << location << " at pos=" << offset;
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
        // group exists? -> return index
        if (di == QFileInfo(group->location()))
            return createIndex(i, 0, group);
        if (fi.baseName() == group->name()) {
            extendedCaption = true;
            group->setFlag(FileSystemContext::cfExtendCaption);
        }
    }
    QModelIndex newGroupMi = addGroup(fi.baseName(), fi.path(), fi.fileName(), rootTreeModelIndex());
    if (extendedCaption)
        group(newGroupMi)->setFlag(FileSystemContext::cfExtendCaption);
    updatePathNode(group(newGroupMi)->id(), fi.path());
    return newGroupMi;
}

void FileRepository::close(int fileId)
{
    FileContext *fc = fileContext(fileId);
    fc->setDocument(nullptr);
    QModelIndex fci = index(fc);
    dataChanged(fci, fci);
    emit fileClosed(fileId);
}

void FileRepository::setSuffixFilter(QStringList filter)
{
    for (QString suff: filter) {
        if (!suff.startsWith("."))
            FATAL() << "invalid suffix " << suff << ". A suffix must start with a dot.";
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

void FileRepository::nodeExpanded(const QModelIndex& index)
{
    FileGroupContext *gc = group(index);
    if (!gc->isWatched()) {
        QFileSystemWatcher *watcher = gc->watchIt();
        connect(gc, &FileGroupContext::contentChanged, this, &FileRepository::updatePathNode);
        connect(watcher, &QFileSystemWatcher::directoryChanged, gc, &FileGroupContext::directoryChanged);
    }
}

FileSystemContext*FileRepository::node(const QModelIndex& index) const
{
    return static_cast<FileSystemContext*>(index.internalPointer());
}

FileSystemContext*FileRepository::file(const QModelIndex& index) const
{
    return static_cast<FileContext*>(index.internalPointer());
}

FileGroupContext*FileRepository::group(const QModelIndex& index) const
{
    return static_cast<FileGroupContext*>(index.internalPointer());
}

void FileRepository::changeName(QModelIndex index, QString newName)
{
    node(index)->setName(newName);
    dataChanged(index, index);
}

} // namespace studio
} // namespace gams
