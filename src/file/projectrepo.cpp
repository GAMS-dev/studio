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
#include "projectrepo.h"
#include "exception.h"
#include "syntax.h"
#include "logger.h"
#include "commonpaths.h"

namespace gams {
namespace studio {

ProjectRepo::ProjectRepo(QObject* parent)
    : QObject(parent), mNextId(0), mTreeModel(new ProjectTreeModel(this, new ProjectGroupNode(mNextId++, "Root", nullptr)))
{
    storeNode(mTreeModel->rootNode());
}

ProjectRepo::~ProjectRepo()
{
    delete mTreeModel;
}

inline ProjectAbstractNode *ProjectRepo::node(NodeId id) const
{
    return mNode.value(id, nullptr);
}

inline ProjectAbstractNode*ProjectRepo::node(const QModelIndex& index) const
{
    return node(index.internalId());
}

inline ProjectGroupNode *ProjectRepo::groupNode(NodeId id) const
{
    ProjectAbstractNode* res = mNode.value(id, nullptr);
    return (res && res->type() == NodeType::Group) ? static_cast<ProjectGroupNode*>(res) : nullptr;
}

inline ProjectGroupNode*ProjectRepo::groupNode(const QModelIndex& index) const
{
    return groupNode(index.internalId());
}

inline ProjectFileNode *ProjectRepo::fileNode(NodeId id) const
{
    ProjectAbstractNode* res = mNode.value(id, nullptr);
    return (res && res->type() == NodeType::File) ? static_cast<ProjectFileNode*>(res) : nullptr;
}

inline ProjectFileNode*ProjectRepo::fileNode(const QModelIndex& index) const
{
    return fileNode(index.internalId());
}

inline ProjectLogNode *ProjectRepo::logNode(NodeId id) const
{
    ProjectAbstractNode* res = mNode.value(id, nullptr);
    return (res && res->type() == NodeType::Log) ? static_cast<ProjectLogNode*>(res) : nullptr;
}

inline ProjectLogNode*ProjectRepo::logNode(ProjectAbstractNode* node)
{
    if (!node) return nullptr;
    ProjectGroupNode* group = nullptr;
    if (node->type() != NodeType::Group)
        group = node->parentEntry();
    else
        group = static_cast<ProjectGroupNode*>(node);
    ProjectLogNode* log = group->logNode();
    return log;

    // TODO(JM) no implicit creation
//    if (!log) {
//        log = new ProjectLogNode(mNextId++, "["+group->name()+"]");
//        storeNode(log);
//        connect(log, &ProjectLogNode::openFileNode, this, &ProjectRepo::openFile);
//        connect(log, &ProjectFileNode::findFileNode, this, &ProjectRepo::findFile);
//        connect(log, &ProjectFileNode::findOrCreateFileNode, this, &ProjectRepo::findOrCreateFileNode);
//        log->setParentEntry(group);
//        bool hit;
//        int offset = group->peekIndex(log->name(), &hit);
//        if (hit) offset++;
// //        mTreeModel->insertChild(offset, group, res);
//    }
}


/*

QModelIndex ProjectRepo::findEntry(QString name, QString location, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = mTreeModel->rootModelIndex();
    ProjectGroupNode *par = groupNode(parentIndex);
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

ProjectGroupNode* ProjectRepo::findGroup(const QString &fileName)
{
    ProjectAbstractNode* node = findNode(fileName);
    if (node)
        return node->parentEntry();
    else
        return nullptr;

}

ProjectAbstractNode* ProjectRepo::findNode(QString filePath, ProjectGroupNode* fileGroup)
{
    ProjectGroupNode *group = fileGroup ? fileGroup : mTreeModel->rootNode();
    ProjectAbstractNode* fsc = group->findNode(filePath);
    return fsc;
}

void ProjectRepo::findFile(QString filePath, ProjectFileNode** resultFile, ProjectGroupNode* fileGroup)
{
    ProjectAbstractNode* fsc = findNode(filePath, fileGroup);
    *resultFile = (fsc && fsc->type() == ProjectAbstractNode::File) ? static_cast<ProjectFileNode*>(fsc)  : nullptr;
}

void ProjectRepo::findOrCreateFileNode(QString filePath, ProjectFileNode*& resultFile, ProjectGroupNode* fileGroup)
{
    if (!QFileInfo(filePath).exists()) {
        filePath = QFileInfo(QDir(fileGroup->location()), filePath).absoluteFilePath();
    }
    if (!QFileInfo(filePath).exists()) {
        EXCEPT() << "File not found: " << filePath;
    }
    if (!fileGroup)
        EXCEPT() << "The group must not be null";
    ProjectAbstractNode* fsc = findNode(filePath, fileGroup);
    if (!fsc) {
        QFileInfo fi(filePath);
        resultFile = addFile(fi.fileName(), CommonPaths::absolutFilePath(filePath), fileGroup);
    } else if (fsc->type() == ProjectAbstractNode::File) {
        resultFile = static_cast<ProjectFileNode*>(fsc);
    } else {
        resultFile = nullptr;
    }

}

QList<ProjectFileNode*> ProjectRepo::modifiedFiles(ProjectGroupNode *fileGroup)
{
    if (!fileGroup)
        fileGroup = mTreeModel->rootNode();
    QList<ProjectFileNode*> res;
    for (int i = 0; i < fileGroup->childCount(); ++i) {
        if (fileGroup->childEntry(i)->type() == NodeType::Group) {
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

int ProjectRepo::saveAll()
{
    QList<ProjectFileNode*> files = modifiedFiles();
    for (ProjectFileNode* fc: files) {
        fc->save();
    }
    return files.size();
}

ProjectGroupNode* ProjectRepo::addGroup(QString name, QString location, QString runInfo, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = mTreeModel->rootModelIndex();
    ProjectGroupNode *par = groupNode(parentIndex);
    if (!par)
        FATAL() << "Can't get parent object";

    bool hit;
    int offset = par->peekIndex(name, &hit);
    if (hit) offset++;
    ProjectGroupNode* group = new ProjectGroupNode(mNextId++, name, location, runInfo);
    storeNode(group);
    mTreeModel->insertChild(offset, groupNode(parentIndex), group);
    connect(group, &ProjectGroupNode::changed, this, &ProjectRepo::nodeChanged);
    connect(group, &ProjectGroupNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChanged);
    connect(group, &ProjectGroupNode::removeNode, this, &ProjectRepo::removeNode);
    connect(group, &ProjectGroupNode::requestNode, this, &ProjectRepo::addNode);
    connect(group, &ProjectGroupNode::findOrCreateFileNode, this, &ProjectRepo::findOrCreateFileNode);
    for (QString suff: mSuffixFilter) {
        QFileInfo fi(location, group->name() + suff);
        if (fi.exists()) group->attachFile(fi.filePath());
    }
    return group;
}

ProjectFileNode* ProjectRepo::addFile(QString name, QString location, ProjectGroupNode* parent)
{
    if (!parent)
        parent = mTreeModel->rootNode();
    bool hit;
    int offset = parent->peekIndex(name, &hit);
    if (hit)
        FATAL() << "The group '" << parent->name() << "' already contains '" << name << "'";
    ProjectFileNode* file = new ProjectFileNode(mNextId++, name, location);
    storeNode(file);
    mTreeModel->insertChild(offset, parent, file);
    connect(file, &ProjectGroupNode::changed, this, &ProjectRepo::nodeChanged);
    connect(file, &ProjectFileNode::modifiedExtern, this, &ProjectRepo::onFileChangedExtern);
    connect(file, &ProjectFileNode::deletedExtern, this, &ProjectRepo::onFileDeletedExtern);
    connect(file, &ProjectFileNode::openFileNode, this, &ProjectRepo::openFile);
    connect(file, &ProjectFileNode::findFileNode, this, &ProjectRepo::findFile);
    connect(file, &ProjectFileNode::findOrCreateFileNode, this, &ProjectRepo::findOrCreateFileNode);
    return file;
}

void ProjectRepo::removeNode(ProjectAbstractNode* node)
{
    if (!node) return;
    mTreeModel->removeChild(node);
    deleteNode(node);
}

ProjectGroupNode* ProjectRepo::ensureGroup(const QString &filePath)
{
    bool extendedCaption = false;
    ProjectGroupNode* group = nullptr;

    QFileInfo fi(filePath);
    QFileInfo di(CommonPaths::absolutFilePath(fi.path()));
    for (int i = 0; i < mTreeModel->rootNode()->childCount(); ++i) {
        ProjectAbstractNode* fsc = mTreeModel->rootNode()->childEntry(i);
        if (fsc && fsc->type() == NodeType::Group && fsc->name() == fi.completeBaseName()) {
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

void ProjectRepo::close(NodeId nodeId)
{
    ProjectFileNode *fc = fileNode(nodeId);
    QModelIndex fci = mTreeModel->index(fc);
    mTreeModel->dataChanged(fci, fci);
    emit fileClosed(nodeId, QPrivateSignal());
}

void ProjectRepo::setSuffixFilter(QStringList filter)
{
    for (QString suff: filter) {
        if (!suff.startsWith("."))
            EXCEPT() << "invalid suffix " << suff << ". A suffix must start with a dot.";
    }
    mSuffixFilter = filter;
}

void ProjectRepo::dump(ProjectAbstractNode *fc, int lv)
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

void ProjectRepo::nodeChanged(NodeId nodeId)
{
    ProjectAbstractNode* nd = node(nodeId);
    if (!nd) return;
    QModelIndex ndIndex = mTreeModel->index(nd);
    emit mTreeModel->dataChanged(ndIndex, ndIndex);
}

void ProjectRepo::editorActivated(QWidget* edit)
{
    ProjectFileNode *fc = fileNode(edit);
    QModelIndex mi = mTreeModel->index(fc);
    mTreeModel->setCurrent(mi);
}

void ProjectRepo::setSelected(const QModelIndex& ind)
{
    mTreeModel->setSelected(ind);
}

void ProjectRepo::removeGroup(ProjectGroupNode* fileGroup)
{
    for (int i = 0; i < fileGroup->childCount(); ++i) {
        ProjectAbstractNode *child = fileGroup->childEntry(i);
        mTreeModel->removeChild(child);
        deleteNode(child);
    }
    mTreeModel->removeChild(fileGroup);
    deleteNode(fileGroup);
}

void ProjectRepo::removeFile(ProjectFileNode* file)
{
    removeNode(file);
}

ProjectTreeModel*ProjectRepo::treeModel() const
{
    return mTreeModel;
}

ProjectLogNode*ProjectRepo::logNode(QWidget* edit)
{
    for (int i = 0; i < mTreeModel->rootNode()->childCount(); ++i) {
        ProjectAbstractNode* fsc = mTreeModel->rootNode()->childEntry(i);
        if (fsc->type() == NodeType::Group) {
            ProjectGroupNode* group = static_cast<ProjectGroupNode*>(fsc);

            if (!group->logNode()) return nullptr;
            if (group->logNode()->editors().contains(edit)) {
                return group->logNode();
            }
        }
    }
    return nullptr;
}

void ProjectRepo::removeMarks(ProjectGroupNode* group)
{
    group->removeMarks(QSet<TextMark::Type>() << TextMark::error << TextMark::link << TextMark::none);
}

void ProjectRepo::updateLinkDisplay(AbstractEditor *editUnderCursor)
{
    if (editUnderCursor) {
        ProjectFileNode *fc = fileNode(editUnderCursor);
        bool ctrl = QApplication::queryKeyboardModifiers() & Qt::ControlModifier;
        bool  isLink = fc->mouseOverLink();
        editUnderCursor->viewport()->setCursor(ctrl&&isLink ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
}

void ProjectRepo::read(const QJsonObject &json)
{
    if (json.contains("projects") && json["projects"].isArray()) {
        QJsonArray gprArray = json["projects"].toArray();
        readGroup(mTreeModel->rootNode(), gprArray);
    }
}

void ProjectRepo::readGroup(ProjectGroupNode* group, const QJsonArray& jsonArray)
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
                if (!group->findNode(node["file"].toString()))
                    group->attachFile(node["file"].toString());
//                    addFile(node["name"].toString(), node["file"].toString(), group);
            }
        }
    }
}

void ProjectRepo::write(QJsonObject& json) const
{
    QJsonArray gprArray;
    writeGroup(mTreeModel->rootNode(), gprArray);
    json["projects"] = gprArray;
}

void ProjectRepo::writeGroup(const ProjectGroupNode* group, QJsonArray& jsonArray) const
{
    for (int i = 0; i < group->childCount(); ++i) {
        ProjectAbstractNode *node = group->childEntry(i);
        QJsonObject nodeObject;
        if (node->type() == NodeType::Group) {
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


void ProjectRepo::onFileChangedExtern(NodeId nodeId)
{
    if (!mChangedIds.contains(nodeId)) mChangedIds << nodeId;
    QTimer::singleShot(100, this, &ProjectRepo::processExternFileEvents);
}

void ProjectRepo::onFileDeletedExtern(NodeId nodeId)
{
    if (!mDeletedIds.contains(nodeId)) mDeletedIds << nodeId;
    QTimer::singleShot(100, this, &ProjectRepo::processExternFileEvents);
}

void ProjectRepo::processExternFileEvents()
{
    while (!mDeletedIds.isEmpty()) {
        int nodeId = mDeletedIds.takeFirst();
        if (mChangedIds.contains(nodeId)) mChangedIds.removeAll(nodeId);
        emit fileDeletedExtern(nodeId);
    }
    while (!mChangedIds.isEmpty()) {
        int nodeId = mChangedIds.takeFirst();
        emit fileChangedExtern(nodeId);
    }
}

void ProjectRepo::addNode(QString name, QString location, ProjectGroupNode* parent)
{
    addFile(name, location, parent);
}

ProjectFileNode* ProjectRepo::fileNode(QWidget* edit) const
{
    QWidget *parentEdit = edit ? edit->parentWidget() : nullptr;
    for (ProjectAbstractNode *fsc: mNode) {
        ProjectFileNode *file = fileNode(fsc->id());
        if (file && (file->hasEditor(edit) || file->hasEditor(parentEdit))) return file;
    }
    return nullptr;
}

QWidgetList ProjectRepo::editors(NodeId nodeId)
{
    ProjectFileNode* file = fileNode(nodeId);
    if (file)
        return file->editors();

    ProjectGroupNode* group = groupNode(nodeId);
    if (!group) group = mTreeModel->rootNode();
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

*/

} // namespace studio
} // namespace gams
