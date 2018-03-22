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
#include "filerepository.h"
#include "exception.h"
#include "syntax.h"
#include "logger.h"
#include"tool.h"

namespace gams {
namespace studio {

FileRepository::FileRepository(QObject* parent)
    : QObject(parent), mNextId(0), mTreeModel(new FileTreeModel(this, new FileGroupContext(mNextId++, "Root", "", "")))
{
    storeContext(mTreeModel->rootContext());
}

FileRepository::~FileRepository()
{
    FileType::clear(); // TODO(JM) There may be a better place to clear the static type-list.
    delete mTreeModel;
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
    FileSystemContext* fsc = group->findContext(filePath);
    return fsc;
}

void FileRepository::findFile(QString filePath, FileContext** resultFile, FileGroupContext* fileGroup)
{
    FileSystemContext* fsc = findContext(filePath, fileGroup);
    *resultFile = (fsc && fsc->type() == FileSystemContext::File) ? static_cast<FileContext*>(fsc)  : nullptr;
}

void FileRepository::findOrCreateFileContext(QString filePath, FileContext*& resultFile, FileGroupContext* fileGroup)
{
    if (!QFileInfo(filePath).exists()) {
        filePath = QFileInfo(QDir(fileGroup->location()), filePath).absoluteFilePath();
    }
    if (!QFileInfo(filePath).exists()) {
        EXCEPT() << "File not found: " << filePath;
    }
    if (!fileGroup)
        EXCEPT() << "The group must not be null";
    FileSystemContext* fsc = findContext(filePath, fileGroup);
    if (!fsc) {
        QFileInfo fi(filePath);
        resultFile = addFile(fi.fileName(), Tool::absolutePath(filePath), fileGroup);
    } else if (fsc->type() == FileSystemContext::File) {
        resultFile = static_cast<FileContext*>(fsc);
    } else {
        resultFile = nullptr;
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
    FileGroupContext* group = new FileGroupContext(mNextId++, name, location, runInfo);
    storeContext(group);
    mTreeModel->insertChild(offset, groupContext(parentIndex), group);
    connect(group, &FileGroupContext::changed, this, &FileRepository::nodeChanged);
    connect(group, &FileGroupContext::gamsProcessStateChanged, this, &FileRepository::gamsProcessStateChanged);
    connect(group, &FileGroupContext::removeNode, this, &FileRepository::removeNode);
    connect(group, &FileGroupContext::requestNode, this, &FileRepository::addNode);
    connect(group, &FileGroupContext::findOrCreateFileContext, this, &FileRepository::findOrCreateFileContext);
    for (QString suff: mSuffixFilter) {
        QFileInfo fi(location, group->name() + suff);
        group->attachFile(fi.filePath());
    }
    return group;
}

FileContext* FileRepository::addFile(QString name, QString location, FileGroupContext* parent)
{
    if (!parent)
        parent = mTreeModel->rootContext();
    bool hit;
    int offset = parent->peekIndex(name, &hit);
    if (hit)
        FATAL() << "The group '" << parent->name() << "' already contains '" << name << "'";
    FileContext* file = new FileContext(mNextId++, name, location);
    storeContext(file);
    mTreeModel->insertChild(offset, parent, file);
    connect(file, &FileGroupContext::changed, this, &FileRepository::nodeChanged);
    connect(file, &FileContext::modifiedExtern, this, &FileRepository::onFileChangedExtern);
    connect(file, &FileContext::deletedExtern, this, &FileRepository::onFileDeletedExtern);
    connect(file, &FileContext::openFileContext, this, &FileRepository::openFileContext);
    connect(file, &FileContext::findFileContext, this, &FileRepository::findFile);
    connect(file, &FileContext::findOrCreateFileContext, this, &FileRepository::findOrCreateFileContext);
    return file;
}

void FileRepository::removeNode(FileSystemContext* node)
{
    if (!node) return;
    mTreeModel->removeChild(node);
    deleteContext(node);
}

FileGroupContext* FileRepository::ensureGroup(const QString &filePath)
{
    bool extendedCaption = false;
    FileGroupContext* group = nullptr;

    QFileInfo fi(filePath);
    QFileInfo di(Tool::absolutePath(fi.path()));
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

    if (!fi.isDir())
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
    FileSystemContext* nd = context(fileId);
    if (!nd) return;
    QModelIndex ndIndex = mTreeModel->index(nd);
    emit mTreeModel->dataChanged(ndIndex, ndIndex);
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
        deleteContext(child);
    }
    mTreeModel->removeChild(fileGroup);
    deleteContext(fileGroup);
}

void FileRepository::removeFile(FileContext* file)
{
    removeNode(file);
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

            if (!group->logContext()) return nullptr;
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
    LogContext* log = group->logContext();
    if (!log) {
        log = new LogContext(mNextId++, "["+group->name()+"]");
        storeContext(log);
        connect(log, &LogContext::openFileContext, this, &FileRepository::openFileContext);
        connect(log, &FileContext::findFileContext, this, &FileRepository::findFile);
        connect(log, &FileContext::findOrCreateFileContext, this, &FileRepository::findOrCreateFileContext);
        log->setParentEntry(group);
        bool hit;
        int offset = group->peekIndex(log->name(), &hit);
        if (hit) offset++;
//        mTreeModel->insertChild(offset, group, res);
    }
    return log;
}

void FileRepository::removeMarks(FileGroupContext* group)
{
    group->removeMarks(QSet<TextMark::Type>() << TextMark::error << TextMark::link << TextMark::none);
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

void FileRepository::read(const QJsonObject &json)
{
    if (json.contains("projects") && json["projects"].isArray()) {
        QJsonArray gprArray = json["projects"].toArray();
        readGroup(mTreeModel->rootContext(), gprArray);
    }
}

void FileRepository::readGroup(FileGroupContext* group, const QJsonArray& jsonArray)
{
    for (int i = 0; i < jsonArray.size(); ++i) {
        QJsonObject node = jsonArray[i].toObject();
        if (node.contains("nodes")) {
            if (node.contains("file") && node["file"].isString()) {
                // TODO(JM) later, groups of deeper level need to be created, too
                FileGroupContext* subGroup = ensureGroup(node["file"].toString());
                if (subGroup) {
                    QJsonArray gprArray = node["nodes"].toArray();
                    readGroup(subGroup, gprArray);
                    // TODO(JM) restore expanded-state
                    emit setNodeExpanded(mTreeModel->index(subGroup));
                }
            }
        } else {
            if (node.contains("name") && node["name"].isString() && node.contains("file") && node["file"].isString()) {
                if (!group->findContext(node["file"].toString()))
                    group->attachFile(node["file"].toString());
//                    addFile(node["name"].toString(), node["file"].toString(), group);
            }
        }
    }
}

void FileRepository::write(QJsonObject& json) const
{
    QJsonArray gprArray;
    writeGroup(mTreeModel->rootContext(), gprArray);
    json["projects"] = gprArray;
}

void FileRepository::writeGroup(const FileGroupContext* group, QJsonArray& jsonArray) const
{
    for (int i = 0; i < group->childCount(); ++i) {
        FileSystemContext *node = group->childEntry(i);
        QJsonObject nodeObject;
        if (node->type() == FileSystemContext::FileGroup) {
            FileGroupContext *subGroup = static_cast<FileGroupContext*>(node);
            nodeObject["file"] = subGroup->runableGms();
            nodeObject["name"] = node->name();
            QJsonArray subArray;
            writeGroup(subGroup, subArray);
            nodeObject["nodes"] = subArray;
        } else {
            nodeObject["file"] = node->location();
            nodeObject["name"] = node->name();
        }
        jsonArray.append(nodeObject);
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

FileSystemContext*FileRepository::context(const QModelIndex& index) const
{
    return context(index.internalId());
}

FileContext*FileRepository::fileContext(const QModelIndex& index) const
{
    return fileContext(index.internalId());
}

FileContext* FileRepository::fileContext(QWidget* edit) const
{
    QWidget *parentEdit = edit ? edit->parentWidget() : nullptr;
    for (FileSystemContext *fsc: mContext) {
        FileContext *file = fileContext(fsc->id());
        if (file && (file->hasEditor(edit) || file->hasEditor(parentEdit))) return file;
    }
    return nullptr;
}

FileGroupContext*FileRepository::groupContext(const QModelIndex& index) const
{
    return groupContext(index.internalId());
}

QWidgetList FileRepository::editors(FileId fileId)
{
    FileContext* file = fileContext(fileId);
    if (file)
        return file->editors();

    FileGroupContext* group = groupContext(fileId);
    if (!group) group = mTreeModel->rootContext();
    if (!group) return QWidgetList();
    QWidgetList allEdits;
    for (int i = 0; i < group->childCount(); ++i) {
        QWidgetList groupEdits = editors(group->childEntry(i)->id());
        for (QWidget* ed: groupEdits) {
            if (!allEdits.contains(ed))
                allEdits << ed;
        }
    }
    return allEdits;
}

} // namespace studio
} // namespace gams
