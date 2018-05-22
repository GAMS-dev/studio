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
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include "projectfilerepo.h"
#include "exception.h"
#include "syntax.h"
#include "logger.h"
#include "commonpaths.h"

namespace gams {
namespace studio {

ProjectFileRepo::ProjectFileRepo(QObject* parent)
    : QObject(parent), mNextId(0), mTreeModel(new ProjectTreeModel(this, new ProjectGroupNode(mNextId++, "Root", "", "")))
{
    storeContext(mTreeModel->rootContext());
}

ProjectFileRepo::~ProjectFileRepo()
{
    FileType::clear(); // TODO(JM) There may be a better place to clear the static type-list.
    delete mTreeModel;
}

QModelIndex ProjectFileRepo::findEntry(QString name, QString location, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = mTreeModel->rootModelIndex();
    ProjectGroupNode *par = groupContext(parentIndex);
    if (!par)
        FATAL() << "Can't get parent object";

    bool hit;
    int offset = par->peekIndex(name, &hit);
    if (hit) {
        ProjectAbstractNode *fc = par->childEntry(offset);
        if (fc->location().compare(location, Qt::CaseInsensitive) == 0) {
            return mTreeModel->index(offset, 0, parentIndex);
        }
    }
    return QModelIndex();
}

ProjectGroupNode* ProjectFileRepo::findGroup(const QString &fileName)
{
    ProjectAbstractNode* context = findContext(fileName);
    if (context)
        return context->parentEntry();
    else
        return nullptr;

}

ProjectAbstractNode* ProjectFileRepo::findContext(QString filePath, ProjectGroupNode* fileGroup)
{
    ProjectGroupNode *group = fileGroup ? fileGroup : mTreeModel->rootContext();
    ProjectAbstractNode* fsc = group->findContext(filePath);
    return fsc;
}

void ProjectFileRepo::findFile(QString filePath, ProjectFileNode** resultFile, ProjectGroupNode* fileGroup)
{
    ProjectAbstractNode* fsc = findContext(filePath, fileGroup);
    *resultFile = (fsc && fsc->type() == ProjectAbstractNode::File) ? static_cast<ProjectFileNode*>(fsc)  : nullptr;
}

void ProjectFileRepo::findOrCreateFileContext(QString filePath, ProjectFileNode*& resultFile, ProjectGroupNode* fileGroup)
{
    if (!QFileInfo(filePath).exists()) {
        filePath = QFileInfo(QDir(fileGroup->location()), filePath).absoluteFilePath();
    }
    if (!QFileInfo(filePath).exists()) {
        EXCEPT() << "File not found: " << filePath;
    }
    if (!fileGroup)
        EXCEPT() << "The group must not be null";
    ProjectAbstractNode* fsc = findContext(filePath, fileGroup);
    if (!fsc) {
        QFileInfo fi(filePath);
        resultFile = addFile(fi.fileName(), CommonPaths::absolutFilePath(filePath), fileGroup);
    } else if (fsc->type() == ProjectAbstractNode::File) {
        resultFile = static_cast<ProjectFileNode*>(fsc);
    } else {
        resultFile = nullptr;
    }

}

QList<ProjectFileNode*> ProjectFileRepo::modifiedFiles(ProjectGroupNode *fileGroup)
{
    if (!fileGroup)
        fileGroup = mTreeModel->rootContext();
    QList<ProjectFileNode*> res;
    for (int i = 0; i < fileGroup->childCount(); ++i) {
        if (fileGroup->childEntry(i)->type() == ProjectAbstractNode::FileGroup) {
            ProjectGroupNode *fgc = static_cast<ProjectGroupNode*>(fileGroup->childEntry(i));
            QList<ProjectFileNode*> sub = modifiedFiles(fgc);
            for (ProjectFileNode *fc : sub) {
                if (!res.contains(fc)) {
                    res << fc;
                }
            }
        }
        if (fileGroup->childEntry(i)->type() == ProjectAbstractNode::File) {
            ProjectFileNode *fc = static_cast<ProjectFileNode*>(fileGroup->childEntry(i));
            if (fc->isModified()) {
                res << fc;
            }
        }
    }
    return res;
}

int ProjectFileRepo::saveAll()
{
    QList<ProjectFileNode*> files = modifiedFiles();
    for (ProjectFileNode* fc: files) {
        fc->save();
    }
    return files.size();
}

ProjectGroupNode* ProjectFileRepo::addGroup(QString name, QString location, QString runInfo, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = mTreeModel->rootModelIndex();
    ProjectGroupNode *par = groupContext(parentIndex);
    if (!par)
        FATAL() << "Can't get parent object";

    bool hit;
    int offset = par->peekIndex(name, &hit);
    if (hit) offset++;
    ProjectGroupNode* group = new ProjectGroupNode(mNextId++, name, location, runInfo);
    storeContext(group);
    mTreeModel->insertChild(offset, groupContext(parentIndex), group);
    connect(group, &ProjectGroupNode::changed, this, &ProjectFileRepo::nodeChanged);
    connect(group, &ProjectGroupNode::gamsProcessStateChanged, this, &ProjectFileRepo::gamsProcessStateChanged);
    connect(group, &ProjectGroupNode::removeNode, this, &ProjectFileRepo::removeNode);
    connect(group, &ProjectGroupNode::requestNode, this, &ProjectFileRepo::addNode);
    connect(group, &ProjectGroupNode::findOrCreateFileContext, this, &ProjectFileRepo::findOrCreateFileContext);
    for (QString suff: mSuffixFilter) {
        QFileInfo fi(location, group->name() + suff);
        if (fi.exists()) group->attachFile(fi.filePath());
    }
    return group;
}

ProjectFileNode* ProjectFileRepo::addFile(QString name, QString location, ProjectGroupNode* parent)
{
    if (!parent)
        parent = mTreeModel->rootContext();
    bool hit;
    int offset = parent->peekIndex(name, &hit);
    if (hit)
        FATAL() << "The group '" << parent->name() << "' already contains '" << name << "'";
    ProjectFileNode* file = new ProjectFileNode(mNextId++, name, location);
    storeContext(file);
    mTreeModel->insertChild(offset, parent, file);
    connect(file, &ProjectGroupNode::changed, this, &ProjectFileRepo::nodeChanged);
    connect(file, &ProjectFileNode::modifiedExtern, this, &ProjectFileRepo::onFileChangedExtern);
    connect(file, &ProjectFileNode::deletedExtern, this, &ProjectFileRepo::onFileDeletedExtern);
    connect(file, &ProjectFileNode::openFileContext, this, &ProjectFileRepo::openFileContext);
    connect(file, &ProjectFileNode::findFileContext, this, &ProjectFileRepo::findFile);
    connect(file, &ProjectFileNode::findOrCreateFileContext, this, &ProjectFileRepo::findOrCreateFileContext);
    return file;
}

void ProjectFileRepo::removeNode(ProjectAbstractNode* node)
{
    if (!node) return;
    mTreeModel->removeChild(node);
    deleteContext(node);
}

ProjectGroupNode* ProjectFileRepo::ensureGroup(const QString &filePath)
{
    bool extendedCaption = false;
    ProjectGroupNode* group = nullptr;

    QFileInfo fi(filePath);
    QFileInfo di(CommonPaths::absolutFilePath(fi.path()));
    for (int i = 0; i < mTreeModel->rootContext()->childCount(); ++i) {
        ProjectAbstractNode* fsc = mTreeModel->rootContext()->childEntry(i);
        if (fsc && fsc->type() == ProjectAbstractNode::FileGroup && fsc->name() == fi.completeBaseName()) {
            group = static_cast<ProjectGroupNode*>(fsc);
            if (di == QFileInfo(group->location())) {
                group->attachFile(fi.filePath());
                group->updateChildNodes();
                return group;
            } else {
                extendedCaption = true;
                group->setFlag(ProjectAbstractNode::cfExtendCaption);
            }
        }
    }
    group = addGroup(fi.completeBaseName(), fi.path(), fi.fileName(), mTreeModel->rootModelIndex());
    if (extendedCaption)
        group->setFlag(ProjectAbstractNode::cfExtendCaption);

    if (!fi.isDir())
        group->attachFile(fi.filePath());

    group->updateChildNodes();
    return group;
}

void ProjectFileRepo::close(FileId fileId)
{
    ProjectFileNode *fc = fileContext(fileId);
    QModelIndex fci = mTreeModel->index(fc);
    mTreeModel->dataChanged(fci, fci);
    emit fileClosed(fileId, QPrivateSignal());
}

void ProjectFileRepo::setSuffixFilter(QStringList filter)
{
    for (QString suff: filter) {
        if (!suff.startsWith("."))
            EXCEPT() << "invalid suffix " << suff << ". A suffix must start with a dot.";
    }
    mSuffixFilter = filter;
}

void ProjectFileRepo::dump(ProjectAbstractNode *fc, int lv)
{
    if (!fc) return;

    qDebug() << QString("  ").repeated(lv) + "+ " + fc->location() + "  (" + fc->name() + ")";
    ProjectGroupNode *gc = qobject_cast<ProjectGroupNode*>(fc);
    if (!gc) return;
    for (int i=0 ; i < gc->childCount() ; i++) {
        ProjectAbstractNode *child = gc->childEntry(i);
        dump(child, lv+1);
    }
}

void ProjectFileRepo::nodeChanged(FileId fileId)
{
    ProjectAbstractNode* nd = context(fileId);
    if (!nd) return;
    QModelIndex ndIndex = mTreeModel->index(nd);
    emit mTreeModel->dataChanged(ndIndex, ndIndex);
}

void ProjectFileRepo::editorActivated(QWidget* edit)
{
    ProjectFileNode *fc = fileContext(edit);
    QModelIndex mi = mTreeModel->index(fc);
    mTreeModel->setCurrent(mi);
}

void ProjectFileRepo::setSelected(const QModelIndex& ind)
{
    mTreeModel->setSelected(ind);
}

void ProjectFileRepo::removeGroup(ProjectGroupNode* fileGroup)
{
    for (int i = 0; i < fileGroup->childCount(); ++i) {
        ProjectAbstractNode *child = fileGroup->childEntry(i);
        mTreeModel->removeChild(child);
        deleteContext(child);
    }
    mTreeModel->removeChild(fileGroup);
    deleteContext(fileGroup);
}

void ProjectFileRepo::removeFile(ProjectFileNode* file)
{
    removeNode(file);
}

ProjectTreeModel*ProjectFileRepo::treeModel() const
{
    return mTreeModel;
}

ProjectLogNode*ProjectFileRepo::logContext(QWidget* edit)
{
    for (int i = 0; i < mTreeModel->rootContext()->childCount(); ++i) {
        ProjectAbstractNode* fsc = mTreeModel->rootContext()->childEntry(i);
        if (fsc->type() == ProjectAbstractNode::FileGroup) {
            ProjectGroupNode* group = static_cast<ProjectGroupNode*>(fsc);

            if (!group->logContext()) return nullptr;
            if (group->logContext()->editors().contains(edit)) {
                return group->logContext();
            }
        }
    }
    return nullptr;
}

ProjectLogNode*ProjectFileRepo::logContext(ProjectAbstractNode* node)
{
    if (!node) return nullptr;
    ProjectGroupNode* group = nullptr;
    if (node->type() != ProjectAbstractNode::FileGroup)
        group = node->parentEntry();
    else
        group = static_cast<ProjectGroupNode*>(node);
    ProjectLogNode* log = group->logContext();
    if (!log) {
        log = new ProjectLogNode(mNextId++, "["+group->name()+"]");
        storeContext(log);
        connect(log, &ProjectLogNode::openFileContext, this, &ProjectFileRepo::openFileContext);
        connect(log, &ProjectFileNode::findFileContext, this, &ProjectFileRepo::findFile);
        connect(log, &ProjectFileNode::findOrCreateFileContext, this, &ProjectFileRepo::findOrCreateFileContext);
        log->setParentEntry(group);
        bool hit;
        int offset = group->peekIndex(log->name(), &hit);
        if (hit) offset++;
//        mTreeModel->insertChild(offset, group, res);
    }
    return log;
}

void ProjectFileRepo::removeMarks(ProjectGroupNode* group)
{
    group->removeMarks(QSet<TextMark::Type>() << TextMark::error << TextMark::link << TextMark::none);
}

void ProjectFileRepo::updateLinkDisplay(AbstractEditor *editUnderCursor)
{
    if (editUnderCursor) {
        ProjectFileNode *fc = fileContext(editUnderCursor);
        bool ctrl = QApplication::queryKeyboardModifiers() & Qt::ControlModifier;
        bool  isLink = fc->mouseOverLink();
        editUnderCursor->viewport()->setCursor(ctrl&&isLink ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
}

void ProjectFileRepo::read(const QJsonObject &json)
{
    if (json.contains("projects") && json["projects"].isArray()) {
        QJsonArray gprArray = json["projects"].toArray();
        readGroup(mTreeModel->rootContext(), gprArray);
    }
}

void ProjectFileRepo::readGroup(ProjectGroupNode* group, const QJsonArray& jsonArray)
{
    for (int i = 0; i < jsonArray.size(); ++i) {
        QJsonObject node = jsonArray[i].toObject();
        if (node.contains("nodes")) {
            if (node.contains("file") && node["file"].isString()) {
                // TODO(JM) later, groups of deeper level need to be created, too
                ProjectGroupNode* subGroup = ensureGroup(node["file"].toString());
                if (subGroup) {
                    QJsonArray gprArray = node["nodes"].toArray();
                    readGroup(subGroup, gprArray);

                    if (subGroup->childCount() > 0) {
                        // TODO(JM) restore expanded-state
                        emit setNodeExpanded(mTreeModel->index(subGroup));
                    } else {
                        removeGroup(subGroup); // dont open empty groups
                    }
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

void ProjectFileRepo::write(QJsonObject& json) const
{
    QJsonArray gprArray;
    writeGroup(mTreeModel->rootContext(), gprArray);
    json["projects"] = gprArray;
}

void ProjectFileRepo::writeGroup(const ProjectGroupNode* group, QJsonArray& jsonArray) const
{
    for (int i = 0; i < group->childCount(); ++i) {
        ProjectAbstractNode *node = group->childEntry(i);
        QJsonObject nodeObject;
        if (node->type() == ProjectAbstractNode::FileGroup) {
            ProjectGroupNode *subGroup = static_cast<ProjectGroupNode*>(node);
            nodeObject["file"] = (!subGroup->runnableGms().isEmpty() ? subGroup->runnableGms()
                                                                : subGroup->childEntry(0)->location());
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


void ProjectFileRepo::onFileChangedExtern(FileId fileId)
{
    if (!mChangedIds.contains(fileId)) mChangedIds << fileId;
    QTimer::singleShot(100, this, &ProjectFileRepo::processExternFileEvents);
}

void ProjectFileRepo::onFileDeletedExtern(FileId fileId)
{
    if (!mDeletedIds.contains(fileId)) mDeletedIds << fileId;
    QTimer::singleShot(100, this, &ProjectFileRepo::processExternFileEvents);
}

void ProjectFileRepo::processExternFileEvents()
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

void ProjectFileRepo::addNode(QString name, QString location, ProjectGroupNode* parent)
{
    addFile(name, location, parent);
}

ProjectAbstractNode*ProjectFileRepo::context(const QModelIndex& index) const
{
    return context(index.internalId());
}

ProjectFileNode*ProjectFileRepo::fileContext(const QModelIndex& index) const
{
    return fileContext(index.internalId());
}

ProjectFileNode* ProjectFileRepo::fileContext(QWidget* edit) const
{
    QWidget *parentEdit = edit ? edit->parentWidget() : nullptr;
    for (ProjectAbstractNode *fsc: mContext) {
        ProjectFileNode *file = fileContext(fsc->id());
        if (file && (file->hasEditor(edit) || file->hasEditor(parentEdit))) return file;
    }
    return nullptr;
}

ProjectGroupNode*ProjectFileRepo::groupContext(const QModelIndex& index) const
{
    return groupContext(index.internalId());
}

QWidgetList ProjectFileRepo::editors(FileId fileId)
{
    ProjectFileNode* file = fileContext(fileId);
    if (file)
        return file->editors();

    ProjectGroupNode* group = groupContext(fileId);
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
