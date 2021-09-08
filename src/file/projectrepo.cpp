/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include <QApplication>

#include "projectrepo.h"
#include "exception.h"
#include "syntax.h"
#include "logger.h"
#include "commonpaths.h"
#include "filemetarepo.h"
#include "process/abstractprocess.h"
#include "projecttreeview.h"
#include "viewhelper.h"
#include "settings.h"

namespace gams {
namespace studio {

ProjectRepo::ProjectRepo(QObject* parent)
    : QObject(parent), mNextId(0), mTreeModel(new ProjectTreeModel(this, new ProjectRootNode(this)))
{
    addToIndex(mTreeModel->rootNode());
    mRunAnimateTimer.setInterval(150);
    QVector<QIcon> runIcons;
    runIcons << Theme::icon(":/img/folder-run1", QIcon::Normal, 100);
    runIcons << Theme::icon(":/img/folder-run2", QIcon::Normal, 100);
    runIcons << Theme::icon(":/img/folder-run3", QIcon::Normal, 100);
    runIcons << Theme::icon(":/img/folder-run4", QIcon::Normal, 100);
    mRunIcons.insert(QPair<QIcon::Mode, int>(QIcon::Normal, 100), runIcons);
    connect(&mRunAnimateTimer, &QTimer::timeout, this, &ProjectRepo::stepRunAnimation);
}

ProjectRepo::~ProjectRepo()
{
    mRunAnimateTimer.stop();
    mRunIcons.clear();
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
    connect(mTreeModel, &ProjectTreeModel::childrenChanged, this, &ProjectRepo::childrenChanged);
    connect(mTreeModel, &ProjectTreeModel::parentAssigned, this, &ProjectRepo::parentAssigned);
}

PExProjectNode *ProjectRepo::findProject(NodeId nodeId) const
{
    PExAbstractNode *node = mNodes.value(nodeId);
    if (!node) return nullptr;
    return node->assignedProject();
}

PExProjectNode *ProjectRepo::findProject(const AbstractProcess *process, PExGroupNode *group) const
{
    if (!group) group = mTreeModel->rootNode();
    return group->findProject(process);
}

PExFileNode *ProjectRepo::findFile(QString filePath, PExGroupNode *fileGroup) const
{
    FileMeta* fm = mFileRepo->fileMeta(filePath);
    return findFile(fm, fileGroup);
}

PExFileNode *ProjectRepo::findFile(FileMeta *fileMeta, PExGroupNode *fileGroup, bool recurse) const
{
    PExGroupNode *group = fileGroup ? fileGroup : mTreeModel->rootNode();
    return group->findFile(fileMeta, recurse);
}

PExAbstractNode *ProjectRepo::node(NodeId id) const
{
    return mNodes.value(id, nullptr);
}

PExAbstractNode*ProjectRepo::node(const QModelIndex& index) const
{
    return node(NodeId(int(index.internalId())));
}

PExGroupNode *ProjectRepo::asGroup(NodeId id) const
{
    PExAbstractNode* res = mNodes.value(id, nullptr);
    return (!res ? nullptr : res->toGroup());
}

PExGroupNode*ProjectRepo::asGroup(const QModelIndex& index) const
{
    return asGroup(NodeId(int(index.internalId())));
}

PExProjectNode *ProjectRepo::asProject(NodeId id) const
{
    PExAbstractNode* res = mNodes.value(id, nullptr);
    return (!res ? nullptr : res->toProject());
}

PExProjectNode *ProjectRepo::asProject(const QModelIndex &index) const
{
    return asProject(NodeId(int(index.internalId())));
}

PExFileNode *ProjectRepo::asFileNode(NodeId id) const
{
    PExAbstractNode* res = mNodes.value(id, nullptr);
    return (!res ? nullptr : res->toFile());
}

PExFileNode*ProjectRepo::asFileNode(const QModelIndex& index) const
{
    return asFileNode(NodeId(int(index.internalId())));
}

PExFileNode *ProjectRepo::findFileNode(QWidget *editWidget) const
{
    FileMeta *fileMeta = mFileRepo->fileMeta(editWidget);
    if (!fileMeta) return nullptr;
    NodeId groupId = ViewHelper::groupId(editWidget);
    PExAbstractNode *node = groupId.isValid() ? mNodes.value(groupId) : nullptr;
    PExGroupNode *group = node ? node->toGroup() : nullptr;
    if (!group) return nullptr;

    return group->findFile(fileMeta, true);
}

PExAbstractNode *ProjectRepo::next(PExAbstractNode *node)
{
    if (!node || node->toRoot()) return nullptr;
    // for non-empty groups the next node is the first child
    if (node->toGroup() && node->toGroup()->childCount())
        return node->toGroup()->childNode(0);
    // for last-children
    PExGroupNode *group = node->parentNode();
    while (group->indexOf(node) == group->childCount()-1) {
        if (group->toRoot()) return group->toRoot()->childNode(0);
        node = group;
        group = node->parentNode();
    }
    return group->childNode(group->indexOf(node)+1);
}

PExAbstractNode *ProjectRepo::previous(PExAbstractNode *node)
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
    PExGroupNode *group = node->toGroup();
    while (group && group->childCount()) {
        node = group->childNode(group->childCount()-1);
        group = node->toGroup();
        if (!group) return node;
    }
    return node;
}

inline PExLogNode *ProjectRepo::asLogNode(NodeId id) const
{
    PExAbstractNode* res = mNodes.value(id, nullptr);
    return (res && res->type() == NodeType::log) ? static_cast<PExLogNode*>(res) : nullptr;
}

PExLogNode* ProjectRepo::asLogNode(PExAbstractNode* node)
{
    if (!node) return nullptr;
    PExGroupNode* group = node->toGroup();
    if (!group) group = node->parentNode();
    while (group && !group->toProject())
        group = group->parentNode();
    if (group && group->toProject() && group->toProject()->hasLogNode())
        return group->toProject()->logNode();
    return nullptr;
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

void ProjectRepo::read(const QVariantList &data)
{
    readGroup(mTreeModel->rootNode(), data);
}

void ProjectRepo::readGroup(PExGroupNode* group, const QVariantList& children)
{
    for (int i = 0; i < children.size(); ++i) {
        QVariantMap child = children.at(i).toMap();
        QString name = child.value("name").toString();
        QString file = child.value("file").toString();
        QString path = child.value("path").toString();
        if (path.isEmpty()) path = QFileInfo(file).absolutePath();
        if (child.contains("nodes")) {
            // group
            QVariantList subChildren = child.value("nodes").toList();
            if (!subChildren.isEmpty() && (!name.isEmpty() || !path.isEmpty())) {
                PExGroupNode* subGroup = createProject(name, path, file, group);
                if (subGroup) {
                    readGroup(subGroup, subChildren);
                    if (subGroup->isEmpty()) {
                        closeGroup(subGroup);
                    } else {
                        bool expand = child.contains("expand") ? child.value("expand").toBool() : true;
                        emit setNodeExpanded(mTreeModel->index(subGroup), expand);
                    }
                }
                QVariantList optList = child.value("options").toList();
                if (!optList.isEmpty() && subGroup->toProject()) {
                    for (QVariant opt : optList) {
                        PExProjectNode *prgn = subGroup->toProject();
                        QString par = opt.toString();
                        prgn->addRunParametersHistory(par);
                    }
                }
            }
        } else {
            // file
            if (!name.isEmpty() || !file.isEmpty()) {
                QString suf = child["type"].toString();
                if (suf == "gms") suf = QFileInfo(name).suffix();
                FileType *ft = &FileType::from(suf);
                if (QFileInfo(file).exists()) {
                    PExFileNode * node = findOrCreateFileNode(file, group, ft, name);
                    if (child.contains("codecMib")) {
                        int codecMib = Settings::settings()->toInt(skDefaultCodecMib);
                        node->file()->setCodecMib(child.contains("codecMib") ? child.value("codecMib").toInt()
                                                                             : codecMib);
                    }
                }
            }
        }
    }
}

void ProjectRepo::write(QVariantList &projects) const
{
    writeGroup(mTreeModel->rootNode(), projects);
}

void ProjectRepo::writeGroup(const PExGroupNode* group, QVariantList& childList) const
{
    for (int i = 0; i < group->childCount(); ++i) {
        PExAbstractNode *node = group->childNode(i);
        QVariantMap nodeObject;
        bool expand = true;
        if (node->toGroup()) {
            if (PExProjectNode *project = node->toProject()) {
                if (project->runnableGms())
                    nodeObject.insert("file", node->toProject()->runnableGms()->location());
            }
            const PExGroupNode *subGroup = node->toGroup();
            nodeObject.insert("path", subGroup->location());
            nodeObject.insert("name", node->name());
            if (subGroup->toProject())
                nodeObject.insert("options", subGroup->toProject()->getRunParametersHistory());
            emit isNodeExpanded(mTreeModel->index(subGroup), expand);
            if (!expand) nodeObject.insert("expand", false);
            QVariantList subArray;
            writeGroup(subGroup, subArray);
            nodeObject.insert("nodes", subArray);

        } else {
            const PExFileNode *file = node->toFile();
            nodeObject.insert("file", file->location());
            nodeObject.insert("name", file->name());
            if (node->toFile()) {
                PExFileNode *fileNode = node->toFile();
                nodeObject.insert("type", fileNode->file()->kindAsStr());
                int mib = fileNode->file()->codecMib();
                nodeObject.insert("codecMib", mib);
            }
        }
        childList.append(nodeObject);
    }
}

void ProjectRepo::renameGroup(PExGroupNode* group)
{
    mTreeView->edit(mTreeModel->index(group));
}

PExGroupNode* ProjectRepo::createProject(QString name, QString path, QString runFileName, PExGroupNode *_parent)
{
    if (!_parent) _parent = mTreeModel->rootNode();
    if (!_parent) FATAL() << "Can't get tree-model root-node";

    PExGroupNode* group;
    PExProjectNode* project = nullptr;
    if (_parent == mTreeModel->rootNode()) {
        FileMeta* runFile = runFileName.isEmpty() ? nullptr : mFileRepo->findOrCreateFileMeta(runFileName);
        project = new PExProjectNode(name, path, runFile);
        group = project;
        connect(project, &PExProjectNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChange);
        connect(project, &PExProjectNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChanged);
        connect(project, &PExProjectNode::getParameterValue, this, &ProjectRepo::getParameterValue);
    } else
        group = new PExGroupNode(name, path);
    addToIndex(group);
    mTreeModel->insertChild(_parent->childCount(), _parent, group);
    connect(group, &PExGroupNode::changed, this, &ProjectRepo::nodeChanged);
    emit changed();
    mTreeView->setExpanded(mTreeModel->index(group), true);
    mTreeModel->sortChildNodes(_parent);
    return group;
}

void ProjectRepo::closeGroup(PExGroupNode* group)
{
    // remove normal children
    for (int i = group->childCount()-1; i >= 0; --i) {
        PExAbstractNode *node = group->childNode(i);
        PExGroupNode* subGroup = node->toGroup();
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

void ProjectRepo::closeNode(PExFileNode *node)
{
    PExGroupNode *group = node->parentNode();
    PExProjectNode *project = node->assignedProject();
    FileMeta *fm = node->file();
    int nodeCountToFile = fileNodes(fm->id()).count();

    if (node->file()->isOpen() && fileNodes(node->file()->id()).size() == 1) {
        DEB() << "Close error: Node has open editors";
        return;
    }

    // Remove reference (if this is a lst file referenced in a log)
    if (project && project->hasLogNode() && project->logNode()->lstNode() == node)
        project->logNode()->resetLst();

    // close actual file and remove repo node
    if (mNodes.contains(node->id())) {
        mTreeModel->removeChild(node);
        removeFromIndex(node);
    }

    // if this file is marked as runnable remove reference
    if (project && project->runnableGms() == node->file()) {
        project->setRunnableGms();
        for (int i = 0; i < project->childCount(); i++) {
            // choose next as main gms file
            PExFileNode *nextRunable = project->childNode(i)->toFile();
            if (nextRunable && nextRunable->location().endsWith(".gms", Qt::CaseInsensitive)) {
                project->setRunnableGms(nextRunable->file());
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

void ProjectRepo::purgeGroup(PExGroupNode *group)
{
    if (!group || group->toRoot()) return;
    PExGroupNode *parGroup = group->parentNode();
    if (group->isEmpty()) {
        closeGroup(group);
        if (parGroup) purgeGroup(parGroup);
    }
}

PExFileNode *ProjectRepo::findOrCreateFileNode(QString location, PExGroupNode *fileGroup, FileType *knownType,
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

PExFileNode* ProjectRepo::findOrCreateFileNode(FileMeta* fileMeta, PExGroupNode* fileGroup, QString explicitName)
{
    if (!fileMeta) {
        DEB() << "The file meta must not be null";
        return nullptr;
    }
    if (!fileGroup) {
        QFileInfo fi(fileMeta->location());
        QString groupName = explicitName.isNull() ? fi.completeBaseName() : explicitName;

        PExFileNode *pfn = findFile(fileMeta);
        if (pfn)
            fileGroup = pfn->parentNode();
        else
            fileGroup = createProject(groupName, fi.absolutePath(), fi.filePath());

        if (!fileGroup) {
            DEB() << "The group must not be null";
            return nullptr;
        }
    }
    PExFileNode* file = findFile(fileMeta, fileGroup, false);
    if (!file) {
        mTreeModel->deselectAll();
        if (fileMeta->kind() == FileKind::Log) {
            PExProjectNode *project = fileGroup->assignedProject();
            return project->logNode();
        }
        file = new PExFileNode(fileMeta);
        if (!explicitName.isNull())
            file->setName(explicitName);
        addToIndex(file);
        mTreeModel->insertChild(fileGroup->childCount(), fileGroup, file);
        mTreeModel->sortChildNodes(fileGroup);
        for (QWidget *w: fileMeta->editors())
            ViewHelper::setGroupId(w, fileGroup->id());
    }
    connect(fileGroup, &PExGroupNode::changed, this, &ProjectRepo::nodeChanged);
    return file;
}

PExLogNode*ProjectRepo::logNode(PExAbstractNode* node)
{
    if (!node) return nullptr;
    // Find the project
    PExProjectNode* project = node->assignedProject();
    if (!project) return nullptr;
    PExLogNode* log = project->logNode();
    if (!log) {
        DEB() << "Error while creating LOG node.";
        return nullptr;
    }
    return log;
}

void ProjectRepo::saveNodeAs(PExFileNode *node, const QString &target)
{
    FileMeta* sourceFM = node->file();
    QString oldFile = node->location();

    // set location to new file
    sourceFM->save(target);

    // re-add old file
    findOrCreateFileNode(oldFile, node->assignedProject());
}

QVector<PExFileNode*> ProjectRepo::fileNodes(const FileId &fileId, const NodeId &groupId) const
{
    QVector<PExFileNode*> res;
    QHashIterator<NodeId, PExAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        PExFileNode* fileNode = i.value()->toFile();
        if (fileNode && fileNode->file()->id() == fileId) {
            if (!groupId.isValid() || fileNode->projectId() == groupId) {
                res << fileNode;
            }
        }
    }
    return res;
}

QVector<PExProjectNode *> ProjectRepo::projects(const FileId &fileId) const
{
    QVector<PExProjectNode *> res;
    QHashIterator<NodeId, PExAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        if (fileId.isValid()) {
            PExFileNode* fileNode = i.value()->toFile();
            if (fileNode && fileNode->file()->id() == fileId) {
                PExProjectNode *project = fileNode->assignedProject();
                if (project && !res.contains(project)) {
                    res << project;
                }
            }
        } else {
            PExProjectNode* project = i.value()->toProject();
            if (project) {
                res << project;
            }
        }
    }
    return res;
}

QVector<AbstractProcess*> ProjectRepo::listProcesses()
{
    QVector<AbstractProcess*> res;
    QHashIterator<NodeId, PExAbstractNode*> i(mNodes);
    while (i.hasNext()) {
        i.next();
        PExProjectNode* project = i.value()->toProject();
        if (project && project->process()) {
            res << project->process();
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
//    PExProjectNode *project = asProject(groupId);
//    if (project)
//        project->markTexts(marks, result);
//}

void ProjectRepo::errorTexts(NodeId groupId, const QVector<int> &lstLines, QStringList &result)
{
    PExProjectNode *project = asProject(groupId);
    if (project)
        project->errorTexts(lstLines, result);
}

void ProjectRepo::stepRunAnimation()
{
    mRunAnimateIndex = ((mRunAnimateIndex+1) % mRunIcons.size());
    for (PExProjectNode* project: mRunnigGroups) {
        QModelIndex ind = mTreeModel->index(project);
        if (ind.isValid())
            emit mTreeModel->dataChanged(ind, ind);
    }
}

void ProjectRepo::dropFiles(QModelIndex idx, QStringList files, QList<NodeId> knownIds, Qt::DropAction act
                            , QList<QModelIndex> &newSelection)
{
    PExGroupNode *group = nullptr;
    if (idx.isValid()) {
        PExAbstractNode *aNode = node(idx);
        group = aNode->toGroup();
        if (!group) group = aNode->parentNode();
    } else {
        QFileInfo firstFile(files.first());
        group = createProject(firstFile.completeBaseName(), firstFile.absolutePath(), "");
    }
    if (!group) return;

    QStringList filesNotFound;
    QList<PExFileNode*> gmsFiles;
    QList<NodeId> newIds;
    for (QString item: files) {
        if (QFileInfo(item).exists()) {
            PExFileNode* file = findOrCreateFileNode(item, group);
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
    PExProjectNode *project = group->toProject();
    if (project && !project->runnableGms() && !gmsFiles.isEmpty()) {
        project->setParameter("gms", gmsFiles.first()->location());
    }
    if (act & Qt::MoveAction) {
        for (NodeId nodeId: knownIds) {
            PExAbstractNode* aNode = node(nodeId);
            PExFileNode* file = aNode->toFile();
            if (!file) continue;
            if (file->parentNode() != group)
                closeNode(file);
        }
    }
}

void ProjectRepo::editorActivated(QWidget* edit, bool select)
{
    PExFileNode *node = findFileNode(edit);
    if (!node) return;
    QModelIndex mi = mTreeModel->index(node);
    mTreeModel->setCurrent(mi);
    mTreeView->setCurrentIndex(mi);
    if (select) mTreeView->selectionModel()->select(mi, QItemSelectionModel::ClearAndSelect);
}

void ProjectRepo::nodeChanged(NodeId nodeId)
{
    PExAbstractNode* nd = node(nodeId);
    if (!nd) return;
    QModelIndex ndIndex = mTreeModel->index(nd);
    emit mTreeModel->dataChanged(ndIndex, ndIndex);
}

void ProjectRepo::closeNodeById(NodeId nodeId)
{
    PExAbstractNode *aNode = node(nodeId);
    PExGroupNode *group = aNode ? aNode->parentNode() : nullptr;
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

QIcon ProjectRepo::runAnimateIcon(QIcon::Mode mode, int alpha)
{
    QPair<QIcon::Mode, int> key(mode, alpha);
    if (!mRunIcons.contains(key)) {
        QVector<QIcon> runIcons;
        runIcons << Theme::icon(":/img/folder-run1", mode, alpha);
        runIcons << Theme::icon(":/img/folder-run2", mode, alpha);
        runIcons << Theme::icon(":/img/folder-run3", mode, alpha);
        runIcons << Theme::icon(":/img/folder-run4", mode, alpha);
        mRunIcons.insert(key, runIcons);
    }
    return mRunIcons.value(key).at(mRunAnimateIndex);
}

void ProjectRepo::gamsProcessStateChange(PExGroupNode *group)
{
    PExProjectNode *project = group->toProject();
    QModelIndex ind = mTreeModel->index(project);
    if (project->process()->state() == QProcess::NotRunning) {
        mRunnigGroups.removeAll(project);
        if (ind.isValid()) emit mTreeModel->dataChanged(ind, ind);
    } else if (!mRunnigGroups.contains(project)) {
        mRunnigGroups << project;
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
    QVector<PExGroupNode*> groups;
    for (PExFileNode *node: fileNodes(fileId)) {
        PExGroupNode *group = node->parentNode();
        while (group && group != mTreeModel->rootNode()) {
            if (groups.contains(group)) break;
            groups << group;
            group = group->parentNode();
        }
        nodeChanged(node->id());
    }
    for (PExGroupNode *group: groups) {
        nodeChanged(group->id());
    }
}

void ProjectRepo::setDebugMode(bool debug)
{
    mDebugMode = debug;
    mTreeModel->setDebugMode(debug);
    mFileRepo->setDebugMode(debug);
    mTextMarkRepo->setDebugMode(debug);
    for (PExAbstractNode *node: mNodes.values()) {
        PExLogNode *log = asLogNode(node);
        if (log && log->file()->editors().size()) {
            TextView *tv = ViewHelper::toTextView(log->file()->editors().first());
            if (tv) tv->setDebugMode(debug);
        }
    }
}

} // namespace studio
} // namespace gams
