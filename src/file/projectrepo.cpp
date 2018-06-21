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
#include "filemetarepo.h"
#include "abstractprocess.h"

namespace gams {
namespace studio {

ProjectRepo::ProjectRepo(QObject* parent)
    : QObject(parent), mTreeModel(new ProjectTreeModel(this, new ProjectRootNode(this)))
{
    indexNode(mTreeModel->rootNode());
}

ProjectRepo::~ProjectRepo()
{
    delete mTreeModel;
}

void ProjectRepo::init(FileMetaRepo *fileRepo, TextMarkRepo *textMarkRepo)
{
    if (mFileRepo || mTextMarkRepo) FATAL() << "The ProjectRepo already has been initialized";
    if (!fileRepo) FATAL() << "The FileMetaRepo must not be null";
    if (!textMarkRepo) FATAL() << "The TextMarkRepo must not be null";
    mFileRepo = fileRepo;
    mTextMarkRepo = textMarkRepo;
}

const ProjectGroupNode* ProjectRepo::findGroup(const QString &filePath)
{
    QFileInfo fi(filePath);
    QFileInfo di(CommonPaths::absolutFilePath(fi.path()));
    const ProjectAbstractNode* node = mTreeModel->rootNode()->findNode(di.filePath(), false);
    return node ? node->toGroup() : nullptr;
}

ProjectRunGroupNode *ProjectRepo::findRunGroup(FileId runId, ProjectGroupNode *group) const
{
    if (!group) group = mTreeModel->rootNode();
    return group->findRunGroup(runId);
}

ProjectRunGroupNode *ProjectRepo::findRunGroup(const AbstractProcess *process, ProjectGroupNode *group) const
{
    if (!group) group = mTreeModel->rootNode();
    return group->findRunGroup(process);
}

ProjectAbstractNode* ProjectRepo::findNode(QString filePath, ProjectGroupNode* fileGroup) const
{
    ProjectGroupNode *group = fileGroup ? fileGroup : mTreeModel->rootNode();
    ProjectAbstractNode* node = group->findNode(filePath);
    return node;
}

inline ProjectAbstractNode *ProjectRepo::node(NodeId id) const
{
    return mNodes.value(id, nullptr);
}

ProjectAbstractNode*ProjectRepo::node(const QModelIndex& index) const
{
    return node(index.internalId());
}

ProjectGroupNode *ProjectRepo::asGroup(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (res && res->type() == NodeType::group) ? static_cast<ProjectGroupNode*>(res) : nullptr;
}

inline ProjectGroupNode*ProjectRepo::asGroup(const QModelIndex& index) const
{
    return asGroup(index.internalId());
}

ProjectRunGroupNode *ProjectRepo::asRunGroup(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (res && res->type() == NodeType::runGroup) ? static_cast<ProjectRunGroupNode*>(res) : nullptr;
}

ProjectRunGroupNode *ProjectRepo::asRunGroup(const QModelIndex &index) const
{
    return asRunGroup(index.internalId());
}

inline ProjectFileNode *ProjectRepo::asFileNode(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (res && res->type() == NodeType::file) ? static_cast<ProjectFileNode*>(res) : nullptr;
}

ProjectFileNode*ProjectRepo::asFileNode(const QModelIndex& index) const
{
    return asFileNode(index.internalId());
}

ProjectFileNode *ProjectRepo::findFileNode(QWidget *editWidget) const
{
    AbstractEditor *edit = FileMeta::toAbstractEdit(editWidget);
    gdxviewer::GdxViewer *gdxViewer = FileMeta::toGdxViewer(editWidget);
    FileMeta *fileMeta = mFileRepo->fileMeta(editWidget);
    if (!fileMeta) return nullptr;

    NodeId groupId = edit ? edit->groupId() : gdxViewer ? gdxViewer->groupId() : -1;
    ProjectAbstractNode *node = groupId.isValid() ? mNodes.value(groupId) : nullptr;
    ProjectGroupNode *group = node ? node->toGroup() : nullptr;
    if (!group) return nullptr;

    return group->findFile(fileMeta, true);
}

inline ProjectLogNode *ProjectRepo::asLogNode(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (res && res->type() == NodeType::log) ? static_cast<ProjectLogNode*>(res) : nullptr;
}

ProjectLogNode* ProjectRepo::asLogNode(ProjectAbstractNode* node)
{
    if (!node) return nullptr;
    const ProjectGroupNode* group = node->toGroup();
    if (!group) group = node->parentNode();
    while (!group->toRunGroup()) group = group->parentNode();
    if (group->toRunGroup()) return group->toRunGroup()->logNode();
    return nullptr;

    // TODO(JM) no implicit creation
//    ProjectLogNode* res;
//    if (!res) {
//        res = new ProjectLogNode(mNextId++, "["+group->name()+"]");
//        storeNode(res);
//        connect(res, &ProjectLogNode::openFileNode, this, &ProjectRepo::openFile);
//        connect(res, &ProjectFileNode::findFileNode, this, &ProjectRepo::findFile);
//        connect(res, &ProjectFileNode::findOrCreateFileNode, this, &ProjectRepo::findOrCreateFileNode);
//        res->setParentEntry(group);
//        bool hit;
//        int offset = group->peekIndex(res->name(), &hit);
//        if (hit) offset++;
//    }
//    return res;
}

bool ProjectRepo::isActive(const ProjectAbstractNode *node) const
{
    ProjectAbstractNode *par = mActiveStack.isEmpty() ? nullptr : mActiveStack.at(0);
    while (par) {
        if (par == node) return true;
        par = par->parentNode();
    }
    return false;
}

void ProjectRepo::setActive(ProjectAbstractNode *node)
{
    int i = mActiveStack.indexOf(node);
    if (i < 0) {
        mActiveStack.insert(0, node);
        while (mActiveStack.size() > 30)
            mActiveStack.remove(30);
    } else if (i > 0) {
        mActiveStack.move(i, 0);
    }
}

ProjectTreeModel*ProjectRepo::treeModel() const
{
    return mTreeModel;
}

FileMetaRepo *ProjectRepo::fileRepo() const
{
    if (!mFileRepo) FATAL() << "Instance not initialized";
    return mFileRepo;
}

TextMarkRepo *ProjectRepo::textMarkRepo() const
{
    if (!mTextMarkRepo) FATAL() << "Instance not initialized";
    return mTextMarkRepo;
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
        QJsonObject jsonObject = jsonArray[i].toObject();
        QString name = jsonObject["name"].toString("");
        QString file = jsonObject["file"].toString("");
        QString path = jsonObject["path"].toString("");
        if (path.isEmpty()) path = QFileInfo(file).absolutePath();
        if (jsonObject.contains("nodes")) {
            // group
            QJsonArray gprArray = jsonObject["nodes"].toArray();
            if (!gprArray.isEmpty() && (!name.isEmpty() || !path.isEmpty())) {
                ProjectGroupNode* subGroup = createGroup(name, path, file, group);
                if (subGroup) {
                    readGroup(subGroup, gprArray);
                    if (subGroup->childCount()) {
                        bool expand = jsonObject["expand"].toBool(true);
                        emit setNodeExpanded(mTreeModel->index(subGroup), expand);
                    } else {
                        closeGroup(subGroup); // dont open empty groups
                    }
                }
            }
        } else {
            // file
            if (!name.isEmpty() || !file.isEmpty()) {
                FileMeta* fileMeta = fileRepo()->findOrCreateFileMeta(file);
                if (!group->findNode(file, false))
                    indexNode(new ProjectFileNode(fileMeta, group));
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
        ProjectAbstractNode *node = group->childNode(i);
        QJsonObject jsonObject;
        bool expand = true;
        if (node->toGroup()) {
            if (node->toRunGroup())
                jsonObject["file"] = node->toRunGroup()->runnableGms()->location();
            const ProjectGroupNode *subGroup = node->toGroup();
            jsonObject["path"] = subGroup->location();
            jsonObject["name"] = node->name();
            emit isNodeExpanded(mTreeModel->index(subGroup), expand);
            if (!expand) jsonObject["expand"] = false;
            QJsonArray subArray;
            writeGroup(subGroup, subArray);
            jsonObject["nodes"] = subArray;

        } else {
            const ProjectFileNode *file = node->toFile();
            jsonObject["file"] = file->location();
            jsonObject["name"] = file->name();
        }
        jsonArray.append(jsonObject);
    }
}

ProjectGroupNode* ProjectRepo::createGroup(QString name, QString path, QString runFileName, ProjectGroupNode *_parent)
{
    if (!_parent) _parent = mTreeModel->rootNode();
    if (!_parent) FATAL() << "Can't get parent object";

    bool hit;
    int offset = _parent->peekIndex(name, &hit);
    if (hit) offset++;

    ProjectGroupNode* group;
    ProjectRunGroupNode* runGroup = nullptr;
    if (_parent == mTreeModel->rootNode()) {
        FileMeta* runFile = runFileName.isEmpty() ? nullptr : mFileRepo->findOrCreateFileMeta(runFileName);
        runGroup = new ProjectRunGroupNode(name, path, runFile);
        group = runGroup;
        connect(runGroup, &ProjectRunGroupNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChanged);
    } else
        group = new ProjectGroupNode(name, path);
    indexNode(group);
    mTreeModel->insertChild(offset, _parent, group);
    connect(group, &ProjectGroupNode::changed, this, &ProjectRepo::nodeChanged);
    emit changed();

//    connect(group, &ProjectGroupNode::removeNode, this, &ProjectRepo::removeNode);
//    connect(group, &ProjectGroupNode::requestNode, this, &ProjectRepo::addNode);
//    connect(group, &ProjectGroupNode::findOrCreateFileNode, this, &ProjectRepo::findOrCreateFileNode);

    return group;
}

void ProjectRepo::closeGroup(ProjectGroupNode* group)
{
    // remove normal cildren
    for (int i = 0; i < group->childCount(); ++i) {
        ProjectAbstractNode *node = group->childNode(i);
        ProjectGroupNode* subGroup = node->toGroup();
        if (subGroup) closeGroup(subGroup);
        else {
            mTreeModel->removeChild(node);
            if (!node->toFile())
                EXCEPT() << "unhandled node of type " << (int)node->type();
            closeNode(node->toFile());
        }
    }
    // remove log-node if present
    if (group->toRunGroup()) {
        // TODO(JM) Take the log-node into normal child-list to prevent the need for this extra-saussage
        if (group->toRunGroup()->logNode())
            closeNode(group->toRunGroup()->logNode());
    }
    mTreeModel->removeChild(group);
    deleteNode(group);
    emit changed();
}

void ProjectRepo::closeNode(ProjectFileNode *node)
{
    ProjectRunGroupNode *runGroup = node->runParentNode();
    if (!runGroup)
        EXCEPT() << "Integrity error: this node has no ProjectRunGroupNode as parent";

    // if this is a lst file referenced in a log
    if (runGroup->logNode() && runGroup->logNode()->lstNode() == node)
        runGroup->logNode()->setLstNode(nullptr);

    // close actual file and remove repo node
    deleteNode(node);

    // if this file is marked as runnable remove reference
    if (runGroup->runnableGms() == node->file()) {
        runGroup->setRunnableGms();
        for (int i = 0; i < runGroup->childCount(); i++) {
            // choose next as main gms file
            ProjectFileNode *nextRunable = runGroup->childNode(i)->toFile();
            if (nextRunable && nextRunable->location().endsWith(".gms", Qt::CaseInsensitive)) {
                runGroup->setRunnableGms(nextRunable->file());
                break;
            }
        }
    }

    // close group if empty now
    if (runGroup->childCount() == 0)
        closeGroup(runGroup);

    emit changed();
}

ProjectFileNode *ProjectRepo::findOrCreateFileNode(QString filePath, ProjectGroupNode *fileGroup)
{
    if (filePath.startsWith("[LOG]"))
        EXCEPT() << "A ProjectLogNode is created with ProjectRunGroup::getOrCreateLogNode";
    FileMeta* fileMeta = mFileRepo->findOrCreateFileMeta(filePath);
    if (filePath.isEmpty())
        EXCEPT() << "Couldn't create a FileMeta for filename '" << filePath << "'";
    return findOrCreateFileNode(fileMeta, fileGroup);
}

ProjectFileNode* ProjectRepo::findOrCreateFileNode(FileMeta* fileMeta, ProjectGroupNode* fileGroup)
{
    if (!fileGroup)
        EXCEPT() << "The group must not be null";
    ProjectAbstractNode* node = findNode(fileMeta->location(), fileGroup);
    if (!node) {
        if (fileMeta->kind() == FileKind::Log)
            EXCEPT() << "A ProjectLogNode is added with ProjectRunGroup::getOrCreateLogNode";
        node = new ProjectFileNode(fileMeta, fileGroup);
        emit changed();
    }
    return node->toFile();

}

QVector<ProjectFileNode*> ProjectRepo::fileNodes(const FileId &fileId, const FileId &runId) const
{
    QVector<ProjectFileNode*> res;
    QHashIterator<NodeId, ProjectAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        ProjectFileNode* fileNode = i.value()->toFile();
        if (fileNode && fileNode->file()->id() == fileId) {
            if (!runId.isValid() || fileNode->runFileId() == runId) {
                res << fileNode;
            }
        }
    }
    return res;
}

QVector<ProjectRunGroupNode *> ProjectRepo::runGroups(const FileId &fileId) const
{
    QVector<ProjectRunGroupNode *> res;
    QHashIterator<NodeId, ProjectAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        ProjectFileNode* fileNode = i.value()->toFile();
        if (fileNode && fileNode->file()->id() == fileId) {
            ProjectRunGroupNode *runGroup = fileNode->runParentNode();
            if (runGroup && !res.contains(runGroup)) {
                res << runGroup;
            }
        }
    }
    return res;
}

void ProjectRepo::setSelected(const QModelIndex& ind)
{
    mTreeModel->setSelected(ind);
}

void ProjectRepo::editorActivated(QWidget* edit)
{
    ProjectFileNode *node = findFileNode(edit);
    if (!node) return;
    QModelIndex mi = mTreeModel->index(node);
    mTreeModel->setCurrent(mi);
}

void ProjectRepo::nodeChanged(NodeId nodeId)
{
    ProjectAbstractNode* nd = node(nodeId);
    if (!nd) return;
    QModelIndex ndIndex = mTreeModel->index(nd);
    emit mTreeModel->dataChanged(ndIndex, ndIndex);
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

void ProjectRepo::findFile(QString filePath, ProjectFileNode** resultFile, ProjectGroupNode* fileGroup)
{
    ProjectAbstractNode* fsc = findNode(filePath, fileGroup);
    *resultFile = (fsc && fsc->type() == ProjectAbstractNode::File) ? static_cast<ProjectFileNode*>(fsc)  : nullptr;
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

void ProjectRepo::removeFile(ProjectFileNode* file)
{
    removeNode(file);
}


ProjectLogNode*ProjectRepo::logNode(QWidget* edit)
{
    for (int i = 0; i < mTreeModel->rootNode()->childCount(); ++i) {
        ProjectAbstractNode* fsc = mTreeModel->rootNode()->childEntry(i);
        if (fsc->type() == NodeType::Group) {
            ProjectGroupNode* group = static_cast<ProjectGroupNode*>(fsc);

            if (!group->logNode()) continue;
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
