/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "projecttreeview.h"
#include "editors/viewhelper.h"

namespace gams {
namespace studio {

ProjectRepo::ProjectRepo(QObject* parent)
    : QObject(parent), mNextId(0), mTreeModel(new ProjectTreeModel(this, new ProjectRootNode(this)))
{
    addToIndex(mTreeModel->rootNode());
    mRunAnimateTimer.setInterval(150);
    mRunIcons << QIcon(":/img/folder-run1");
    mRunIcons << QIcon(":/img/folder-run2");
    mRunIcons << QIcon(":/img/folder-run3");
    mRunIcons << QIcon(":/img/folder-run4");
    connect(&mRunAnimateTimer, &QTimer::timeout, this, &ProjectRepo::stepRunAnimation);
}

ProjectRepo::~ProjectRepo()
{
    mRunAnimateTimer.stop();
    FileType::clear();
    delete mTreeModel;
}

void ProjectRepo::init(ProjectTreeView *treeView, FileMetaRepo *fileRepo, TextMarkRepo *textMarkRepo)
{
    Q_ASSERT_X(!mFileRepo && !mTextMarkRepo, "ProjectRepo initialization", "The ProjectRepo already has been initialized");
    Q_ASSERT_X(treeView, "ProjectRepo initialization", "The ProjectTreeView must not be null");
    Q_ASSERT_X(fileRepo, "ProjectRepo initialization", "The FileMetaRepo must not be null");
    Q_ASSERT_X(textMarkRepo, "ProjectRepo initialization", "The TextMarkRepo must not be null");
    mTreeView = treeView;
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
    NodeId groupId = ViewHelper::groupId(editWidget);
    ProjectAbstractNode *node = groupId.isValid() ? mNodes.value(groupId) : nullptr;
    ProjectGroupNode *group = node ? node->toGroup() : nullptr;
    if (!group) return nullptr;

    return group->findFile(fileMeta, true);
}

ProjectAbstractNode *ProjectRepo::next(ProjectAbstractNode *node)
{
    if (!node || node->toRoot()) return nullptr;
    // for non-empty groups the next node is the first child
    if (node->toGroup() && node->toGroup()->childCount())
        return node->toGroup()->childNode(0);
    // for last-children
    ProjectGroupNode *group = node->parentNode();
    while (group->indexOf(node) == group->childCount()-1) {
        if (group->toRoot()) return group->toRoot()->childNode(0);
        node = group;
        group = node->parentNode();
    }
    return group->childNode(group->indexOf(node)+1);
}

ProjectAbstractNode *ProjectRepo::previous(ProjectAbstractNode *node)
{
    if (!node || node->toRoot()) return nullptr;
    int i = node->parentNode()->indexOf(node);
    if (i > 0) {
        node = node->parentNode()->childNode(i-1);
    } else if (node->parentNode()->toRoot()) {
        node = node->parentNode()->childNode(node->parentNode()->childCount()-1);
    } else {
        return node->parentNode();
    }
    ProjectGroupNode *group = node->toGroup();
    while (group && group->childCount()) {
        node = group->childNode(group->childCount()-1);
        group = node->toGroup();
        if (!group) return node;
    }
    return node;
}

inline ProjectLogNode *ProjectRepo::asLogNode(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (res && res->type() == NodeType::log) ? static_cast<ProjectLogNode*>(res) : nullptr;
}

ProjectLogNode* ProjectRepo::asLogNode(ProjectAbstractNode* node)
{
    if (!node) return nullptr;
    ProjectGroupNode* group = node->toGroup();
    if (!group) group = node->parentNode();
    while (group && !group->toRunGroup())
        group = group->parentNode();
    if (group && group->toRunGroup() && group->toRunGroup()->hasLogNode())
        return group->toRunGroup()->logNode();
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
    Q_ASSERT_X(mFileRepo, "ProjectRepo", "FileMetaRepo not initialized");
    return mFileRepo;
}

TextMarkRepo *ProjectRepo::textMarkRepo() const
{
    Q_ASSERT_X(mTextMarkRepo, "ProjectRepo", "TextMarkRepo not initialized");
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
        QJsonObject nodeObject = jsonArray[i].toObject();
        QString name = nodeObject["name"].toString("");
        QString file = nodeObject["file"].toString("");
        QString path = nodeObject["path"].toString("");
        if (path.isEmpty()) path = QFileInfo(file).absolutePath();
        if (nodeObject.contains("nodes")) {
            // group
            QJsonArray gprArray = nodeObject["nodes"].toArray();
            if (!gprArray.isEmpty() && (!name.isEmpty() || !path.isEmpty())) {
                ProjectGroupNode* subGroup = createGroup(name, path, file, group);
                if (subGroup) {
                    readGroup(subGroup, gprArray);
                    if (subGroup->isEmpty()) {
                        closeGroup(subGroup);
                    } else {
                        bool expand = nodeObject["expand"].toBool(true);
                        emit setNodeExpanded(mTreeModel->index(subGroup), expand);
                    }
                }
                QJsonArray optArray = nodeObject["options"].toArray();
                if (!optArray.isEmpty() && subGroup->toRunGroup()) {
                    for (QVariant opt : optArray.toVariantList()) {
                        ProjectRunGroupNode *prgn = subGroup->toRunGroup();
                        QString par = opt.toString();
                        prgn->addRunParametersHistory(par);
                    }
                }
            }
        } else {
            // file
            if (!name.isEmpty() || !file.isEmpty()) {
                FileType *ft = &FileType::from(nodeObject["type"].toString());
                if (QFileInfo(file).exists()) {
                    ProjectFileNode * node = findOrCreateFileNode(file, group, ft, name);
                    if (nodeObject.contains("codecMib")) {
                        node->file()->setCodecMib(nodeObject["codecMib"].toInt(-1));
                    }
                }
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
        QJsonObject nodeObject;
        bool expand = true;
        if (node->toGroup()) {
            if (ProjectRunGroupNode *runGroup = node->toRunGroup()) {
                if (runGroup->runnableGms())
                    nodeObject["file"] = node->toRunGroup()->runnableGms()->location();
            }
            const ProjectGroupNode *subGroup = node->toGroup();
            nodeObject["path"] = subGroup->location();
            nodeObject["name"] = node->name();
            if (subGroup->toRunGroup())
                nodeObject["options"] = QJsonArray::fromStringList(subGroup->toRunGroup()->getRunParametersHistory());
            emit isNodeExpanded(mTreeModel->index(subGroup), expand);
            if (!expand) nodeObject["expand"] = false;
            QJsonArray subArray;
            writeGroup(subGroup, subArray);
            nodeObject["nodes"] = subArray;

        } else {
            const ProjectFileNode *file = node->toFile();
            nodeObject["file"] = file->location();
            nodeObject["name"] = file->name();
            if (node->toFile()) {
                ProjectFileNode * fileNode = node->toFile();
                nodeObject["type"] = fileNode->file()->kindAsStr();
                int mib = fileNode->file()->codecMib();
                if (mib) nodeObject["codecMib"] = mib;
            }
        }
        jsonArray.append(nodeObject);
    }
}

void ProjectRepo::renameGroup(ProjectGroupNode* group)
{
    mTreeView->edit(mTreeModel->index(group));
}

ProjectGroupNode* ProjectRepo::createGroup(QString name, QString path, QString runFileName, ProjectGroupNode *_parent)
{
    if (!_parent) _parent = mTreeModel->rootNode();
    if (!_parent) FATAL() << "Can't get tree-model root-node";

    ProjectGroupNode* group;
    ProjectRunGroupNode* runGroup = nullptr;
    if (_parent == mTreeModel->rootNode()) {
        FileMeta* runFile = runFileName.isEmpty() ? nullptr : mFileRepo->findOrCreateFileMeta(runFileName);
        runGroup = new ProjectRunGroupNode(name, path, runFile);
        group = runGroup;
        connect(runGroup, &ProjectRunGroupNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChange);
        connect(runGroup, &ProjectRunGroupNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChanged);
    } else
        group = new ProjectGroupNode(name, path);
    addToIndex(group);
    mTreeModel->insertChild(_parent->childCount(), _parent, group);
    connect(group, &ProjectGroupNode::changed, this, &ProjectRepo::nodeChanged);
    emit changed();
    mTreeView->setExpanded(mTreeModel->index(group), true);
    mTreeModel->sortChildNodes(_parent);
    return group;
}

void ProjectRepo::closeGroup(ProjectGroupNode* group)
{
    // remove normal children
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
    }
}

void ProjectRepo::closeNode(ProjectFileNode *node)
{
    ProjectGroupNode *group = node->parentNode();
    ProjectRunGroupNode *runGroup = node->assignedRunGroup();
    FileMeta *fm = node->file();
    int nodeCountToFile = fileNodes(fm->id()).count();

    if (node->file()->isOpen() && fileNodes(node->file()->id()).size() == 1) {
        DEB() << "Close error: Node has open editors";
        return;
    }

    // Remove reference (if this is a lst file referenced in a log)
    if (runGroup && runGroup->hasLogNode() && runGroup->logNode()->lstNode() == node)
        runGroup->logNode()->resetLst();

    // close actual file and remove repo node
    if (mNodes.contains(node->id())) {
        mTreeModel->removeChild(node);
        removeFromIndex(node);
    }

    // if this file is marked as runnable remove reference
    if (runGroup && runGroup->runnableGms() == node->file()) {
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
    node->deleteLater();
    if (nodeCountToFile == 1) {
        fm->deleteLater();
    }
    purgeGroup(group);
}

void ProjectRepo::purgeGroup(ProjectGroupNode *group)
{
    if (!group || group->toRoot()) return;
    ProjectGroupNode *parGroup = group->parentNode();
    if (group->isEmpty()) {
        closeGroup(group);
        if (parGroup) purgeGroup(parGroup);
    }
}

ProjectFileNode *ProjectRepo::findOrCreateFileNode(QString location, ProjectGroupNode *fileGroup, FileType *knownType,
                                                   QString explicitName)
{
    if (location.isEmpty())
        return nullptr;
    if (location.contains('\\'))
        location = QDir::fromNativeSeparators(location);

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

        ProjectFileNode *pfn = findFile(fileMeta);
        if (pfn)
            fileGroup = pfn->parentNode();
        else
            fileGroup = createGroup(groupName, fi.absolutePath(), fi.filePath());

        if (!fileGroup) {
            DEB() << "The group must not be null";
            return nullptr;
        }
    }
    ProjectFileNode* file = findFile(fileMeta, fileGroup, false);
    if (!file) {
        mTreeModel->deselectAll();
        if (fileMeta->kind() == FileKind::Log) {
            ProjectRunGroupNode *runGroup = fileGroup->assignedRunGroup();
            return runGroup->logNode();
        }
        file = new ProjectFileNode(fileMeta);
        if (!explicitName.isNull())
            file->setName(explicitName);
        addToIndex(file);
        mTreeModel->insertChild(fileGroup->childCount(), fileGroup, file);
        mTreeModel->sortChildNodes(fileGroup);
    }
    connect(fileGroup, &ProjectGroupNode::changed, this, &ProjectRepo::nodeChanged);
    return file;
}

ProjectLogNode*ProjectRepo::logNode(ProjectAbstractNode* node)
{
    if (!node) return nullptr;
    // Find the runGroup
    ProjectRunGroupNode* runGroup = node->assignedRunGroup();
    if (!runGroup) return nullptr;
    ProjectLogNode* log = runGroup->logNode();
    if (!log) {
        DEB() << "Error while creating LOG node.";
        return nullptr;
    }
    return log;
}

void ProjectRepo::saveNodeAs(ProjectFileNode *node, const QString &target)
{
    FileMeta* sourceFM = node->file();
    QString oldFile = node->location();

    // set location to new file
    sourceFM->save(target);

    // re-add old file
    findOrCreateFileNode(oldFile, node->assignedRunGroup());
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
        if (fileId.isValid()) {
            ProjectFileNode* fileNode = i.value()->toFile();
            if (fileNode && fileNode->file()->id() == fileId) {
                ProjectRunGroupNode *runGroup = fileNode->assignedRunGroup();
                if (runGroup && !res.contains(runGroup)) {
                    res << runGroup;
                }
            }
        } else {
            ProjectRunGroupNode* runGroup = i.value()->toRunGroup();
            if (runGroup) {
                res << runGroup;
            }
        }
    }
    return res;
}

QVector<AbstractProcess*> ProjectRepo::listProcesses()
{
    QVector<AbstractProcess*> res;
    QHashIterator<NodeId, ProjectAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        ProjectRunGroupNode* runGroup = i.value()->toRunGroup();
        if (runGroup && runGroup->process()) {
            res << runGroup->process();
        }
    }
    return res;
}

void ProjectRepo::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    mTreeModel->selectionChanged(selected, deselected);
    QVector<QModelIndex> groups;
    for (QModelIndex ind: mTreeModel->popAddGroups()) {
        if (!mTreeView->isExpanded(ind))
            groups << ind;
    }
    QItemSelectionModel *selModel = mTreeView->selectionModel();
    for (QModelIndex ind: mTreeModel->popDeclined()) {
        selModel->select(ind, QItemSelectionModel::Deselect);
    }
    for (QModelIndex group: groups) {
        if (!mTreeView->isExpanded(group)) {
            mTreeView->setExpanded(group, true);
            for (int row = 0; row < mTreeModel->rowCount(group); ++row) {
                QModelIndex ind = mTreeModel->index(row, 0, group);
                selModel->select(ind, QItemSelectionModel::Select);
            }
        }
    }
}

//void ProjectRepo::markTexts(NodeId groupId, const QList<TextMark *> &marks, QStringList &result)
//{
//    ProjectRunGroupNode *runGroup = asRunGroup(groupId);
//    if (runGroup)
//        runGroup->markTexts(marks, result);
//}

void ProjectRepo::errorTexts(NodeId groupId, const QVector<int> &lstLines, QStringList &result)
{
    ProjectRunGroupNode *runGroup = asRunGroup(groupId);
    if (runGroup)
        runGroup->errorTexts(lstLines, result);
}

void ProjectRepo::stepRunAnimation()
{
    mRunAnimateIndex = ((mRunAnimateIndex+1) % mRunIcons.size());
    for (ProjectRunGroupNode* runGroup: mRunnigGroups) {
        QModelIndex ind = mTreeModel->index(runGroup);
        if (ind.isValid())
            emit mTreeModel->dataChanged(ind, ind);
    }
}

void ProjectRepo::dropFiles(QModelIndex idx, QStringList files, QList<NodeId> knownIds, Qt::DropAction act
                            , QList<QModelIndex> &newSelection)
{
    ProjectGroupNode *group = nullptr;
    if (idx.isValid()) {
        ProjectAbstractNode *aNode = node(idx);
        group = aNode->toGroup();
        if (!group) group = aNode->parentNode();
    } else {
        QFileInfo firstFile(files.first());
        group = createGroup(firstFile.baseName(), firstFile.absolutePath(), "");
    }
    if (!group) return;

    QStringList filesNotFound;
    QList<ProjectFileNode*> gmsFiles;
    QList<NodeId> newIds;
    for (QString item: files) {
        if (QFileInfo(item).exists()) {
            ProjectFileNode* file = findOrCreateFileNode(item, group);
            if (file->file()->kind() == FileKind::Gms) gmsFiles << file;
            if (!newIds.contains(file->id())) newIds << file->id();
        } else {
            filesNotFound << item;
        }
    }
    for (NodeId id: newIds) {
        QModelIndex mi = mTreeModel->index(id);
        newSelection << mi;
    }
    if (!filesNotFound.isEmpty()) {
        DEB() << "Files not found:\n" << filesNotFound.join("\n");
    }
    ProjectRunGroupNode *runGroup = group->toRunGroup();
    if (runGroup && !runGroup->runnableGms() && !gmsFiles.isEmpty()) {
        runGroup->setParameter("gms", gmsFiles.first()->location());
    }
    if (act & Qt::MoveAction) {
        for (NodeId nodeId: knownIds) {
            ProjectAbstractNode* aNode = node(nodeId);
            ProjectFileNode* file = aNode->toFile();
            if (!file) continue;
            if (file->parentNode() != group)
                closeNode(file);
        }
    }
}

void ProjectRepo::editorActivated(QWidget* edit, bool select)
{
    ProjectFileNode *node = findFileNode(edit);
    if (!node) return;
    QModelIndex mi = mTreeModel->index(node);
    mTreeModel->setCurrent(mi);
    mTreeView->setCurrentIndex(mi);
    if (select) mTreeView->selectionModel()->select(mi, QItemSelectionModel::ClearAndSelect);
}

void ProjectRepo::nodeChanged(NodeId nodeId)
{
    ProjectAbstractNode* nd = node(nodeId);
    if (!nd) return;
    QModelIndex ndIndex = mTreeModel->index(nd);
    emit mTreeModel->dataChanged(ndIndex, ndIndex);
}

void ProjectRepo::closeNodeById(NodeId nodeId)
{
    ProjectAbstractNode *aNode = node(nodeId);
    ProjectGroupNode *group = aNode ? aNode->parentNode() : nullptr;
    if (aNode->toFile()) closeNode(aNode->toFile());
    if (aNode->toGroup()) closeGroup(aNode->toGroup());
    if (group) purgeGroup(group);
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

QIcon ProjectRepo::runAnimateIcon() const
{
    return mRunIcons.at(mRunAnimateIndex);
}

void ProjectRepo::gamsProcessStateChange(ProjectGroupNode *group)
{
    ProjectRunGroupNode *runGroup = group->toRunGroup();
    QModelIndex ind = mTreeModel->index(runGroup);
    if (runGroup->process()->state() == QProcess::NotRunning) {
        mRunnigGroups.removeAll(runGroup);
        if (ind.isValid()) emit mTreeModel->dataChanged(ind, ind);
    } else if (!mRunnigGroups.contains(runGroup)) {
        mRunnigGroups << runGroup;
        if (ind.isValid()) emit mTreeModel->dataChanged(ind, ind);
    }
    if (mRunnigGroups.isEmpty() && mRunAnimateTimer.isActive()) {
        mRunAnimateTimer.stop();
        mRunAnimateIndex = 0;
    } else if (!mRunnigGroups.isEmpty() && !mRunAnimateTimer.isActive()) {
        mRunAnimateIndex = 0;
        mRunAnimateTimer.start();
    }
}

bool ProjectRepo::debugMode() const
{
    return mDebugMode;
}

void ProjectRepo::fileChanged(FileId fileId)
{
    QVector<ProjectGroupNode*> groups;
    for (ProjectFileNode *node: fileNodes(fileId)) {
        ProjectGroupNode *group = node->parentNode();
        while (group && group != mTreeModel->rootNode()) {
            if (groups.contains(group)) break;
            groups << group;
            group = group->parentNode();
        }
        nodeChanged(node->id());
    }
    for (ProjectGroupNode *group: groups) {
        nodeChanged(group->id());
    }
}

void ProjectRepo::setDebugMode(bool debug)
{
    mDebugMode = debug;
    mTreeModel->setDebugMode(debug);
    mFileRepo->setDebugMode(debug);
    mTextMarkRepo->setDebugMode(debug);
    for (ProjectAbstractNode *node: mNodes.values()) {
        ProjectLogNode *log = asLogNode(node);
        if (log && log->file()->editors().size()) {
            TextView *tv = ViewHelper::toTextView(log->file()->editors().first());
            if (tv) tv->setDebugMode(debug);
        }
    }
}

} // namespace studio
} // namespace gams
