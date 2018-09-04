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
#include <QApplication>

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
    : QObject(parent), mNextId(0), mTreeModel(new ProjectTreeModel(this, new ProjectRootNode(this)))
{
    addToIndex(mTreeModel->rootNode());
}

ProjectRepo::~ProjectRepo()
{
    FileType::clear(); // TODO(JM) There may be a better place to clear the static type-list.
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

ProjectRunGroupNode *ProjectRepo::findRunGroup(NodeId nodeId) const
{
    ProjectAbstractNode *node = mNodes.value(nodeId);
    if (!node) return nullptr;
    return node->assignedRunGroup();
}

ProjectRunGroupNode *ProjectRepo::findRunGroup(const AbstractProcess *process, ProjectGroupNode *group) const
{
    if (!group) group = mTreeModel->rootNode();
    return group->findRunGroup(process);
}

ProjectFileNode *ProjectRepo::findFile(QString filePath, ProjectGroupNode *fileGroup) const
{
    FileMeta* fm = mFileRepo->fileMeta(filePath);
    return findFile(fm, fileGroup);
}

ProjectFileNode *ProjectRepo::findFile(FileMeta *fileMeta, ProjectGroupNode *fileGroup, bool recurse) const
{
    ProjectGroupNode *group = fileGroup ? fileGroup : mTreeModel->rootNode();
    return group->findFile(fileMeta, recurse);
}

ProjectAbstractNode *ProjectRepo::node(NodeId id) const
{
    return mNodes.value(id, nullptr);
}

ProjectAbstractNode*ProjectRepo::node(const QModelIndex& index) const
{
    return node(NodeId(int(index.internalId())));
}

ProjectGroupNode *ProjectRepo::asGroup(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (!res ? nullptr : res->toGroup());
}

inline ProjectGroupNode*ProjectRepo::asGroup(const QModelIndex& index) const
{
    return asGroup(NodeId(int(index.internalId())));
}

ProjectRunGroupNode *ProjectRepo::asRunGroup(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (!res ? nullptr : res->toRunGroup());
}

ProjectRunGroupNode *ProjectRepo::asRunGroup(const QModelIndex &index) const
{
    return asRunGroup(NodeId(int(index.internalId())));
}

inline ProjectFileNode *ProjectRepo::asFileNode(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (!res ? nullptr : res->toFile());
}

ProjectFileNode*ProjectRepo::asFileNode(const QModelIndex& index) const
{
    return asFileNode(NodeId(int(index.internalId())));
}

ProjectFileNode *ProjectRepo::findFileNode(QWidget *editWidget) const
{
    FileMeta *fileMeta = mFileRepo->fileMeta(editWidget);
    if (!fileMeta) return nullptr;
    AbstractEdit *edit = FileMeta::toAbstractEdit(editWidget);
    gdxviewer::GdxViewer *gdxViewer = FileMeta::toGdxViewer(editWidget);
    NodeId groupId = edit ? edit->groupId() : (gdxViewer ? gdxViewer->groupId() : NodeId());
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
                FileType *ft = &FileType::from(jsonObject["type"].toString(""));
                findOrCreateFileNode(file, group, ft, name);
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
            if (ProjectRunGroupNode *runGroup = node->toRunGroup()) {
                if (runGroup->runnableGms())
                    jsonObject["file"] = node->toRunGroup()->runnableGms()->location();
            }
            const ProjectGroupNode *subGroup = node->toGroup();
            jsonObject["path"] = subGroup->location();
            jsonObject["name"] = node->name();
            if (subGroup->toRunGroup())
                jsonObject["options"] = QJsonArray::fromStringList(subGroup->toRunGroup()->getRunParametersHistory());
            emit isNodeExpanded(mTreeModel->index(subGroup), expand);
            if (!expand) jsonObject["expand"] = false;
            QJsonArray subArray;
            writeGroup(subGroup, subArray);
            jsonObject["nodes"] = subArray;

        } else {
            const ProjectFileNode *file = node->toFile();
            jsonObject["file"] = file->location();
            jsonObject["name"] = file->name();
            if (node->toFile()) {
                ProjectFileNode * fileNode = node->toFile();
                if (!fileNode->file()->suffix().isEmpty())
                    jsonObject["type"] = fileNode->file()->suffix().first();
            }
        }
        jsonArray.append(jsonObject);
    }
}

ProjectGroupNode* ProjectRepo::createGroup(QString name, QString path, QString runFileName, ProjectGroupNode *_parent)
{
    if (!_parent) _parent = mTreeModel->rootNode();
    if (!_parent) FATAL() << "Can't get tree-model root-node";

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
    addToIndex(group);
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
    for (int i = group->childCount()-1; i >= 0; --i) {
        ProjectAbstractNode *node = group->childNode(i);
        ProjectGroupNode* subGroup = node->toGroup();
        if (subGroup) closeGroup(subGroup);
        else {
            if (!node->toFile())
                EXCEPT() << "unhandled node of type " << int(node->type());
            closeNode(node->toFile());
        }
    }

    if (mNodes.contains(group->id())) {
        mTreeModel->removeChild(group);
        removeFromIndex(group);
        emit changed();
    }
}

void ProjectRepo::closeNode(ProjectFileNode *node)
{
    ProjectRunGroupNode *runGroup = node->assignedRunGroup();
    if (!runGroup)
        EXCEPT() << "Integrity error: this node has no ProjectRunGroupNode as parent";

    if (node->file()->isOpen() && fileNodes(node->file()->id()).size() == 1) {
        DEB() << "Close error: Node has open editors";
        return;
    }

    // Remove reference (if this is a lst file referenced in a log)
    if (runGroup->logNode() && runGroup->logNode()->lstNode() == node)
        runGroup->logNode()->setLstNode(nullptr);

    // close actual file and remove repo node

    if (mNodes.contains(node->id())) {
        mTreeModel->removeChild(node);
        removeFromIndex(node);
    }

    // TODO(JM) check if this was the last node for the FileMeta - then also remove the FileMeta

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

ProjectFileNode *ProjectRepo::findOrCreateFileNode(QString location, ProjectGroupNode *fileGroup, FileType *knownType
                                                   , QString explicitName)
{
//    if (location.startsWith("[LOG]")) {
//        EXCEPT() << "A ProjectLogNode is created with ProjectRunGroup::getOrCreateLogNode";
//    }
    if (location.isEmpty()) {
        // TODO(JM) should we allow FileMeta to be created for a non-existant file?
        EXCEPT() << "Couldn't create a FileMeta for filename '" << location << "'";
    }
    if (!knownType || knownType->kind() == FileKind::None)
        knownType = parseGdxHeader(location) ? &FileType::from(FileKind::Gdx) : nullptr;

    FileMeta* fileMeta = mFileRepo->findOrCreateFileMeta(location, knownType);
    return findOrCreateFileNode(fileMeta, fileGroup, explicitName);
}

ProjectFileNode* ProjectRepo::findOrCreateFileNode(FileMeta* fileMeta, ProjectGroupNode* fileGroup, QString explicitName)
{
    if (!fileMeta) {
        DEB() << "The file meta must not be null";
        return nullptr;
    }
    if (!fileGroup) {
        QFileInfo fi(fileMeta->location());
        QString groupName = explicitName.isNull() ? fi.completeBaseName() : explicitName;
        fileGroup = createGroup(groupName, fi.absolutePath(), fi.filePath());
        if (!fileGroup) {
            DEB() << "The group must not be null";
            return nullptr;
        }
    }
    ProjectFileNode* file = findFile(fileMeta, fileGroup, false);
    if (!file) {
        if (fileMeta->kind() == FileKind::Log) {
            ProjectRunGroupNode *runGroup = fileGroup->assignedRunGroup();
            file = runGroup->getOrCreateLogNode(mFileRepo);
        } else {
            file = new ProjectFileNode(fileMeta, fileGroup);
        }
        if (!explicitName.isNull())
            file->setName(explicitName);
        int offset = fileGroup->peekIndex(file->name());
        addToIndex(file);
        mTreeModel->insertChild(offset, fileGroup, file);
        emit changed();
    }
    connect(fileGroup, &ProjectGroupNode::changed, this, &ProjectRepo::nodeChanged);
    return file;
}

QVector<ProjectFileNode*> ProjectRepo::fileNodes(const FileId &fileId, const NodeId &groupId) const
{
    QVector<ProjectFileNode*> res;
    QHashIterator<NodeId, ProjectAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        ProjectFileNode* fileNode = i.value()->toFile();
        if (fileNode && fileNode->file()->id() == fileId) {
            if (!groupId.isValid() || fileNode->runGroupId() == groupId) {
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
            ProjectRunGroupNode *runGroup = fileNode->assignedRunGroup();
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

void ProjectRepo::lstTexts(NodeId groupId, const QList<TextMark *> &marks, QStringList &result)
{
    ProjectRunGroupNode *runGroup = asRunGroup(groupId);
    if (runGroup)
        runGroup->lstTexts(marks, result);
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

bool ProjectRepo::parseGdxHeader(QString location)
{
    QFile file(location);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.read(50);
        file.close();
        return data.contains("\aGAMSGDX\a");
    }
    return false;
}

bool ProjectRepo::debugMode() const
{
    return mDebugMode;
}

ProjectLogNode* ProjectRepo::logNode(QWidget* edit)
{
    ProjectFileNode* node = findFileNode(edit);
    if (!node) return nullptr;
    return logNode(node);
}

ProjectLogNode*ProjectRepo::logNode(ProjectAbstractNode* node)
{
    if (!node) return nullptr;
    // Find the runGroup
    ProjectRunGroupNode* runGroup = node->assignedRunGroup();
    if (!runGroup) return nullptr;

    ProjectLogNode* log = runGroup->getOrCreateLogNode(mFileRepo);
    if (!log) {
        DEB() << "Error while creating LOG node.";
        return nullptr;
    }
    addToIndex(log);
    return log;
}

void ProjectRepo::update(ProjectAbstractNode *node)
{
    QModelIndex mi = mTreeModel->index(node);
    mTreeModel->setCurrent(mi);
}

void ProjectRepo::setDebugMode(bool debug)
{
    mDebugMode = debug;
    mTreeModel->setDebugMode(debug);
    mFileRepo->setDebugMode(debug);
    mTextMarkRepo->setDebugMode(debug);
}








//void ProjectRepo::dump(ProjectAbstractNode *fc, int lv)
//{
//    if (!fc) return;

//    qDebug() << QString("  ").repeated(lv) + "+ " + fc->location() + "  (" + fc->name() + ")";
//    ProjectGroupNode *gc = qobject_cast<ProjectGroupNode*>(fc);
//    if (!gc) return;
//    for (int i=0 ; i < gc->childCount() ; i++) {
//        ProjectAbstractNode *child = gc->childNode(i);
//        dump(child, lv+1);
//    }
//}

// TODO(JM) move implementation to AbstractEdit
//void ProjectRepo::updateLinkDisplay(AbstractEdit *editUnderCursor)
//{
//    if (editUnderCursor) {
//        ProjectFileNode *fc = fileNode(editUnderCursor);
//        bool ctrl = QApplication::queryKeyboardModifiers() & Qt::ControlModifier;
//        bool  isLink = fc->mouseOverLink();
//        editUnderCursor->viewport()->setCursor(ctrl&&isLink ? Qt::PointingHandCursor : Qt::ArrowCursor);
//    }
//}

} // namespace studio
} // namespace gams
