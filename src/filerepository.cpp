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
#include "syntax.h"
#include "logger.h"

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

FileSystemContext* FileRepository::context(FileId fileId, FileSystemContext* startNode)
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

FileContext* FileRepository::fileContext(FileId fileId, FileSystemContext* startNode)
{
    auto c = context(fileId, (startNode ? startNode : mTreeModel->rootContext()));
    if (c->type() == FileSystemContext::File)
        return static_cast<FileContext*>(c);
    return nullptr;
}

FileGroupContext* FileRepository::groupContext(FileId fileId, FileSystemContext* startNode)
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

FileSystemContext* FileRepository::findContext(QString filePath, FileGroupContext* fileGroup)
{
    FileGroupContext *group = fileGroup ? fileGroup : mTreeModel->rootContext();
    FileSystemContext* fsc = group->findFile(filePath);
    return fsc;
}

void FileRepository::findFile(QString filePath, FileContext** resultFile, FileGroupContext* fileGroup)
{
    FileSystemContext* fsc = findContext(filePath, fileGroup);
    *resultFile = (fsc && fsc->type() == FileSystemContext::File) ? static_cast<FileContext*>(fsc)  : nullptr;
}

void FileRepository::findOrCreateFileContext(QString filePath, FileContext** resultFile, FileGroupContext* fileGroup)
{
    if (!fileGroup)
        EXCEPT() << "The group must not be null";
    FileSystemContext* fsc = findContext(filePath, fileGroup);
    if (!fsc) {
        QFileInfo fi(filePath);
        *resultFile = addFile(fi.fileName(), fi.canonicalFilePath(), fileGroup);
    } else if (fsc->type() == FileSystemContext::File) {
        *resultFile = static_cast<FileContext*>(fsc);
    } else {
        *resultFile = nullptr;
    }

}

QList<FileContext*> FileRepository::modifiedFiles(FileGroupContext *fileGroup)
{
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
    connect(fgContext, &FileGroupContext::gamsProcessStateChanged, this, &FileRepository::gamsProcessStateChanged);
    connect(fgContext, &FileGroupContext::removeNode, this, &FileRepository::removeNode);
    connect(fgContext, &FileGroupContext::requestNode, this, &FileRepository::addNode);
    updateActions();
    for (QString suff: mSuffixFilter) {
        QFileInfo fi(location, fgContext->name() + suff);
        fgContext->attachFile(fi.filePath());
    }
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
    connect(fileContext, &FileContext::openFileContext, this, &FileRepository::openFileContext);
    connect(fileContext, &FileContext::findOrCreateFileContext, this, &FileRepository::findOrCreateFileContext);
    updateActions();
    return fileContext;
}

void FileRepository::removeNode(FileSystemContext* node)
{
    mTreeModel->removeChild(node);
    delete node;
}

FileGroupContext* FileRepository::ensureGroup(const QString &filePath, const QString &additionalFile)
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
                group->attachFile(fi.filePath());
                group->updateChildNodes();
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
    group->attachFile(fi.filePath());
    group->updateChildNodes();
    return group;
}

void FileRepository::close(FileId fileId)
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

void FileRepository::nodeChanged(FileId fileId)
{
    FileSystemContext* nd = context(fileId, mTreeModel->rootContext());
    if (!nd) return;
    QModelIndex ndIndex = mTreeModel->index(nd);
    emit mTreeModel->dataChanged(ndIndex, ndIndex);
}

void FileRepository::nodeClicked(QModelIndex index)
{
    FileActionContext* act = actionContext(index);
    if (act) {
        emit act->trigger();
        return;
    }
}

void FileRepository::editorActivated(QWidget* edit)
{
    FileContext *fc = fileContext(edit);
    QModelIndex mi = mTreeModel->index(fc);
    mTreeModel->setCurrent(mi);
}

void FileRepository::setSelected(const QModelIndex& ind)
{
    mTreeModel->setSelected(ind);
}

void FileRepository::removeGroup(FileGroupContext* fileGroup)
{
    for (int i = 0; i < fileGroup->childCount(); ++i) {
        FileSystemContext *child = fileGroup->childEntry(i);
        mTreeModel->removeChild(child);
        delete child;
    }
    mTreeModel->removeChild(fileGroup);
    delete fileGroup;
}

FileTreeModel*FileRepository::treeModel() const
{
    return mTreeModel;
}

LogContext*FileRepository::logContext(QWidget* edit)
{
    for (int i = 0; i < mTreeModel->rootContext()->childCount(); ++i) {
        FileSystemContext* fsc = mTreeModel->rootContext()->childEntry(i);
        if (fsc->type() == FileSystemContext::FileGroup) {
            FileGroupContext* group = static_cast<FileGroupContext*>(fsc);
            if (group->logContext()->editors().contains(edit)) {
                return group->logContext();
            }
        }
    }
    return nullptr;
}

LogContext*FileRepository::logContext(FileSystemContext* node)
{
    if (!node) return nullptr;
    FileGroupContext* group = nullptr;
    if (node->type() != FileSystemContext::FileGroup)
        group = node->parentEntry();
    else
        group = static_cast<FileGroupContext*>(node);
    LogContext* res = group->logContext();
    if (!res) {
        res = new LogContext(mNextId++, "["+group->name()+"]");
        connect(res, &LogContext::openFileContext, this, &FileRepository::openFileContext);
        connect(res, &FileContext::findOrCreateFileContext, this, &FileRepository::findOrCreateFileContext);
        bool hit;
        int offset = group->peekIndex(res->name(), &hit);
        if (hit) offset++;
        mTreeModel->insertChild(offset, group, res);
    }
    return res;
}

void FileRepository::removeMarks(FileGroupContext* group)
{
    for (int i = 0; i < group->childCount(); ++i) {
        FileSystemContext* fsc = group->childEntry(i);
        if (fsc->type() == FileSystemContext::File) {
            FileContext* fc = static_cast<FileContext*>(fsc);
            fc->removeTextMarks(QSet<TextMark::Type>() << TextMark::error << TextMark::link);
        }
    }
}

void FileRepository::updateLinkDisplay(QPlainTextEdit* editUnderCursor)
{
    if (editUnderCursor) {
        FileContext *fc = fileContext(editUnderCursor);
        bool ctrl = QApplication::queryKeyboardModifiers() & Qt::ControlModifier;
        bool  isLink = fc->mouseOverLink();
        editUnderCursor->viewport()->setCursor(ctrl&&isLink ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
}

void FileRepository::onFileChangedExtern(FileId fileId)
{
    if (!mChangedIds.contains(fileId)) mChangedIds << fileId;
    QTimer::singleShot(100, this, &FileRepository::processExternFileEvents);
}

void FileRepository::onFileDeletedExtern(FileId fileId)
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

void FileRepository::addNode(QString name, QString location, FileGroupContext* parent)
{
    addFile(name, location, parent);
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

FileContext* FileRepository::fileContext(QWidget* edit)
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

QWidgetList FileRepository::editors(FileId fileId)
{
    FileSystemContext* fsc = (fileId < 0 ? mTreeModel->rootContext() : context(fileId, mTreeModel->rootContext()));
    if (!fsc) return QWidgetList();

    if (fsc->type() == FileSystemContext::FileGroup) {
        // TODO(JM) gather all editors of the group
        QWidgetList allEdits;
        FileGroupContext* group = static_cast<FileGroupContext*>(fsc);
        for (int i = 0; i < group->childCount(); ++i) {
            QWidgetList groupEdits = editors(group->childEntry(i)->id());
            for (QWidget* ed: groupEdits) {
                if (!allEdits.contains(ed))
                    allEdits << ed;
            }
        }
        return allEdits;
    }
    if (fsc->type() == FileSystemContext::File) {
        return static_cast<FileContext*>(fsc)->editors();
    }
    return QWidgetList();
}

} // namespace studio
} // namespace gams
