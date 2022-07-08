/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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

const QString ProjectRepo::CIgnoreSuffix(".lst.lxi.log.");

ProjectRepo::ProjectRepo(QObject* parent)
    : QObject(parent), mNextId(0), mTreeModel(new ProjectTreeModel(this, new ProjectRootNode(this)))
{
    addToIndex(mTreeModel->rootNode());
    mRunAnimateTimer.setInterval(250);
    runAnimateIcon(QIcon::Normal, 100);
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

PExFileNode *ProjectRepo::findFile(FileMeta *fileMeta, PExGroupNode *fileGroup) const
{
    PExGroupNode *group = fileGroup ? fileGroup : mTreeModel->rootNode();
    return group->findFile(fileMeta);
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
    return group->findFile(fileMeta);
}

PExProjectNode *ProjectRepo::findProjectForOptions(QWidget *projectOptionsWidget) const
{
    FileMeta *fileMeta = mFileRepo->fileMeta(projectOptionsWidget);
    if (!fileMeta || fileMeta->kind() != FileKind::PrO) return nullptr;
    NodeId groupId = ViewHelper::groupId(projectOptionsWidget);
    if (!groupId.isValid()) return nullptr;
    PExAbstractNode *node = mNodes.value(groupId);
    return node ? node->toProject() : nullptr;
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

bool ProjectRepo::checkRead(const QVariantList &data, int &count, int &ignored, QStringList &missed, const QString &sysWorkDir)
{
    count = 0;
    ignored = 0;
    missed.clear();
    for (int i = 0; i < data.size(); ++i) {
        QVariantMap child = data.at(i).toMap();
        QString path = child.value("path").toString();
        if (path == "." || path.isEmpty())
            path = sysWorkDir.isEmpty() ? CommonPaths::defaultWorkingDir() : sysWorkDir;
        QDir localBaseDir(path);
        QString fileName = QDir::cleanPath(localBaseDir.absoluteFilePath(child.value("file").toString()));

        if (path.isEmpty()) path = QFileInfo(fileName).absolutePath();
        QVariantList children = child.value("nodes").toList();
        if (!children.isEmpty() && !path.isEmpty()) {
            for (int i = 0; i < children.size(); ++i) {
                QVariantMap child = children.at(i).toMap();
                fileName = QDir::cleanPath(localBaseDir.absoluteFilePath(child.value("file").toString()));
                QFileInfo file(fileName);
                if (!file.exists()) {
                    if (CIgnoreSuffix.contains('.'+file.suffix()+'.')) ++ignored;
                    else missed << fileName;
                }
                ++count;
            }
        }
    }
    return missed.isEmpty();
}

bool ProjectRepo::read(const QVariantList &data, const QString &sysWorkDir)
{
    bool res = true;
    for (int i = 0; i < data.size(); ++i) {
        QVariantMap child = data.at(i).toMap();
        QString name = child.value("name").toString();
        QString path = child.value("path").toString();
        if (path == "." || path.isEmpty())
            path = sysWorkDir.isEmpty() ? CommonPaths::defaultWorkingDir() : sysWorkDir;
        QDir localBaseDir(path);
        QString workDir = QDir::cleanPath(localBaseDir.absoluteFilePath(child.value("workDir").toString()));
        if (workDir.isEmpty()) workDir = path;

        QString file = QDir::cleanPath(localBaseDir.absoluteFilePath(child.value("file").toString()));
        if (path.isEmpty()) path = QFileInfo(file).absolutePath();
        QVariantList subChildren = child.value("nodes").toList();
        if (!subChildren.isEmpty() && (!name.isEmpty() || !path.isEmpty())) {
            PExProjectNode* project = createProject(name, path, file, workDir);
            if (project) {
                if (!readProjectFiles(project, subChildren, localBaseDir.path()))
                    res = false;
                if (project->isEmpty()) {
                    closeGroup(project);
                } else {
                    bool expand = child.contains("expand") ? child.value("expand").toBool() : true;
                    emit setNodeExpanded(mTreeModel->index(project), expand);
                }
            }
            QVariantList optList = child.value("options").toList();
            if (!optList.isEmpty() && project->toProject()) {
                for (QVariant opt : optList) {
                    PExProjectNode *prgn = project->toProject();
                    QString par = opt.toString();
                    prgn->addRunParametersHistory(par);
                }
            }
        }
    }
    return res;
}

bool ProjectRepo::readProjectFiles(PExProjectNode *project, const QVariantList &children, const QString &workDir)
{
    bool res = true;
    if (!project)
        EXCEPT() << "Missing project node, can't add file nodes";
    QDir localBaseDir(workDir);
    for (int i = 0; i < children.size(); ++i) {
        QVariantMap child = children.at(i).toMap();
        QString name = child.value("name").toString();
        QString file = QDir::cleanPath(localBaseDir.absoluteFilePath(child.value("file").toString()));
        if (!name.isEmpty() || !file.isEmpty()) {
            QString suf = child["type"].toString();
            if (suf == "gms") suf = QFileInfo(name).fileName();
            FileType *ft = &FileType::from(suf);
            if (QFileInfo(file).exists()) {
                PExFileNode * node = findOrCreateFileNode(file, project, ft, name);
                if (child.contains("codecMib")) {
                    int codecMib = Settings::settings()->toInt(skDefaultCodecMib);
                    node->file()->setCodecMib(child.contains("codecMib") ? child.value("codecMib").toInt()
                                                                         : codecMib);
                }
            } else if (!CIgnoreSuffix.contains('.'+QFileInfo(file).suffix()+'.')) {
                emit addWarning("File not found: " + file);
                res = false;
            }
        }
    }
    return res;
}

void ProjectRepo::write(QVariantList &projects) const
{
    for (int i = 0; i < mTreeModel->rootNode()->childCount(); ++i) {
        PExProjectNode *project = mTreeModel->rootNode()->childNode(i)->toProject();
        write(project, projects);
    }
}

void ProjectRepo::write(PExProjectNode *project, QVariantList &projects, bool relativePaths) const
{
    if (!project) return;
    QVariantMap nodeObject;
    bool expand = true;
    QDir dir(project->location());
    if (project->runnableGms()) {
        QString filePath = project->toProject()->runnableGms()->location();
        nodeObject.insert("file", relativePaths ? dir.relativeFilePath(filePath) : filePath);
    }
    nodeObject.insert("path", relativePaths ? "." : project->location() );
    nodeObject.insert("workDir", relativePaths ? dir.relativeFilePath(project->workDir()) : project->workDir() );
    nodeObject.insert("name", project->name());
    nodeObject.insert("options", project->toProject()->getRunParametersHistory());
    emit isNodeExpanded(mTreeModel->index(project), expand);
    if (!expand) nodeObject.insert("expand", false);
    QVariantList subArray;
    writeProjectFiles(project, subArray, relativePaths);
    nodeObject.insert("nodes", subArray);
    projects.append(nodeObject);
}

void ProjectRepo::writeProjectFiles(const PExProjectNode* project, QVariantList& childList, bool relativePaths) const
{
    QDir dir(project->location());
    for (PExFileNode *file : project->listFiles()) {
        QVariantMap nodeObject;
        nodeObject.insert("file", relativePaths ? dir.relativeFilePath(file->location()) : file->location());
        nodeObject.insert("name", file->name());
        nodeObject.insert("type", file->file()->kindAsStr());
        int mib = file->file()->codecMib();
        nodeObject.insert("codecMib", mib);
        childList.append(nodeObject);
    }
}

void ProjectRepo::addToProject(PExProjectNode *project, PExFileNode *file, bool withFolders)
{
    PExGroupNode *oldParent = nullptr;
    if (mNodes.contains(file->id()))
        oldParent = file->parentNode()->toGroup();
    else
        addToIndex(file);
    // create missing group node for folders
    PExGroupNode *newParent = project;
    if (withFolders) {
        QDir prjPath(project->location());
        QString relPath = prjPath.relativeFilePath(file->location());
        bool isAbs = QDir(relPath).isAbsolute();
        QStringList folders;
        folders = relPath.split('/');
        folders.removeLast();
        for (const QString &folderName : folders) {
            newParent = findOrCreateFolder(folderName, newParent, isAbs);
            isAbs = false;
        }
    }
    // add to (new) destination
    mTreeModel->insertChild(newParent->childCount(), newParent, file);
    mTreeModel->sortChildNodes(project);
    purgeGroup(oldParent);
}

QString ProjectRepo::uniqueNodeName(PExGroupNode *parentNode, const QString &name, PExAbstractNode *node)
{
    // Project name should be unique, append number in case
    if (!parentNode) return name;
    QString res = name;
    int nr = 0;
    bool conflict = true;
    while (conflict) {
        conflict = false;
        for (PExAbstractNode * n : parentNode->childNodes()) {
            if (n != node && n->name() == res) {
                ++nr;
                res = name + QString::number(nr);
                conflict = true;
                break;
            }
        }
    }
    return res;
}

PExProjectNode* ProjectRepo::createProject(QString name, QString path, QString runFileName, QString workDir)
{
    PExGroupNode *root = mTreeModel->rootNode();
    if (!root) FATAL() << "Can't get tree-model root-node";

    PExProjectNode* project = nullptr;
    FileMeta* runFile = runFileName.isEmpty() ? nullptr : mFileRepo->findOrCreateFileMeta(runFileName);

    project = new PExProjectNode(uniqueNodeName(mTreeModel->rootNode(), name), path, runFile, workDir);
    connect(project, &PExProjectNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChange);
    connect(project, &PExProjectNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChanged);
    connect(project, &PExProjectNode::getParameterValue, this, &ProjectRepo::getParameterValue);
    connect(project, &PExProjectNode::baseDirChanged, this, &ProjectRepo::reassignFiles);
    addToIndex(project);
    mTreeModel->insertChild(root->childCount(), root, project);
    connect(project, &PExGroupNode::changed, this, &ProjectRepo::nodeChanged);
    emit changed();
    mTreeView->setExpanded(mTreeModel->index(project), true);
    mTreeModel->sortChildNodes(root);
    return project;
}

PExGroupNode *ProjectRepo::findOrCreateFolder(QString folderName, PExGroupNode *parentNode, bool isAbs)
{
    if (!parentNode) FATAL() << "Parent-node missing";
    if (parentNode == mTreeModel->rootNode()) FATAL() << "Folder-node must not exist on top level";

    for (int i = 0; i < parentNode->childCount(); ++i) {
        PExAbstractNode *node = parentNode->childNode(i);
        if (node->name().compare(folderName, FileType::fsCaseSense()) == 0) {
            PExGroupNode* folder = node->toGroup();
            if (!folder)
                EXCEPT() << "Folder node '" << folderName << "' already exists as file node";
            return node->toGroup();
        }
    }
    PExGroupNode* folder = new PExGroupNode(folderName, isAbs ? folderName
                                                              : QDir::cleanPath(parentNode->location()+'/'+folderName));
    addToIndex(folder);
    mTreeModel->insertChild(parentNode->childCount(), parentNode, folder);
    connect(folder, &PExGroupNode::changed, this, &ProjectRepo::nodeChanged);
    emit changed();
    mTreeModel->sortChildNodes(parentNode);
    return folder;
}

void ProjectRepo::closeGroup(PExGroupNode* group)
{
    if (group->childCount()) EXCEPT() << "Can't close project that isn't empty";
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
    if (group->isEmpty() && !group->toProject()) {
        closeGroup(group);
        if (parGroup) purgeGroup(parGroup);
    }
}

PExFileNode *ProjectRepo::findOrCreateFileNode(QString location, PExProjectNode *project, FileType *knownType,
                                                   QString explicitName)
{
    if (location.isEmpty())
        return nullptr;
    if (location.contains('\\'))
        location = QDir::fromNativeSeparators(location);

    if (!knownType || knownType->kind() == FileKind::None)
        knownType = parseGdxHeader(location) ? &FileType::from(FileKind::Gdx) : nullptr;

    FileMeta* fileMeta = mFileRepo->findOrCreateFileMeta(location, knownType);
    return findOrCreateFileNode(fileMeta, project, explicitName);
}

PExFileNode* ProjectRepo::findOrCreateFileNode(FileMeta* fileMeta, PExProjectNode* project, QString explicitName)
{
    if (!fileMeta) {
        DEB() << "The file meta must not be null";
        return nullptr;
    }
    if (!project) {
        QFileInfo fi(fileMeta->location());
        QString groupName = explicitName.isNull() ? fi.completeBaseName() : explicitName;

        if (PExFileNode *pfn = findFile(fileMeta))
            project = pfn->assignedProject();
        else
            project = createProject(groupName, fi.absolutePath(), fi.filePath());

        if (!project) {
            DEB() << "The project must not be null";
            return nullptr;
        }
    }
    PExFileNode* file = findFile(fileMeta, project);
    if (!file) {
        mTreeModel->deselectAll();
        if (fileMeta->kind() == FileKind::Log)
            return project->logNode();
        file = new PExFileNode(fileMeta);
        if (!explicitName.isNull())
            file->setName(explicitName);
        addToProject(project, file, true);
        for (QWidget *w: fileMeta->editors())
            ViewHelper::setGroupId(w, project->id());
    }
    if (!project->runnableGms() && fileMeta->kind() == FileKind::Gms)
        project->setRunnableGms(fileMeta);
    connect(project, &PExGroupNode::changed, this, &ProjectRepo::nodeChanged);
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
    for (QModelIndex ind: mTreeModel->popAddProjects()) {
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
    mRunAnimateIndex = ((mRunAnimateIndex+1) % mRunIconCount);
    for (PExProjectNode* project: mRunnigGroups) {
        QModelIndex ind = mTreeModel->index(project);
        if (ind.isValid())
            emit mTreeModel->dataChanged(ind, ind);
    }
}

void ProjectRepo::dropFiles(QModelIndex idx, QStringList files, QList<NodeId> knownIds, Qt::DropAction act
                            , QList<QModelIndex> &newSelection)
{
    while (files.count() && files.first().isEmpty())
        files.removeFirst();

    QList<NodeId> addIds;
    for (NodeId id : knownIds) {
        PExGroupNode *group = asGroup(id);
        if (group && group->type() == NodeType::group) {
            QVector<PExFileNode*> groupFiles = group->listFiles();
            for (PExFileNode* file: groupFiles) {
                files << file->location();
                addIds << file->id();
            }
        }
    }
    knownIds.append(addIds);

    PExProjectNode *project = nullptr;
    if (idx.isValid()) {
        PExAbstractNode *aNode = node(idx);
        project = aNode->assignedProject();
    } else {
        QFileInfo firstFile(files.first());
        project = createProject(firstFile.completeBaseName(), firstFile.absolutePath(), files.first());
    }

    QStringList filesNotFound;
    QList<PExFileNode*> gmsFiles;
    QList<NodeId> newIds;
    for (const QString &item: qAsConst(files)) {
        QFileInfo f(item);
        QDir d(item);

        if (f.isFile()) {
            if (item.endsWith(".gsp", Qt::CaseInsensitive)) {
                emit openProject(item);
                continue;
            }
            PExFileNode* file = findOrCreateFileNode(item, project);
            if (knownIds.contains(file->id())) knownIds.removeAll(file->id());
            if (file->file()->kind() == FileKind::Gms) gmsFiles << file;
            if (!newIds.contains(file->id())) newIds << file->id();
        } else if (d.exists()) {
            emit openFolder(item, project);
        } else {
            filesNotFound << item;
        }
    }
    for (const NodeId &id: qAsConst(newIds)) {
        QModelIndex mi = mTreeModel->index(id);
        newSelection << mi;
    }
    if (!filesNotFound.isEmpty()) {
        DEB() << "Files not found:\n" << filesNotFound.join("\n");
    }
    if (project && !project->runnableGms() && !gmsFiles.isEmpty()) {
        project->setParameter("gms", gmsFiles.first()->location());
    }
    if (act & Qt::MoveAction) {
        for (const NodeId &nodeId: qAsConst(knownIds)) {
            PExAbstractNode* aNode = node(nodeId);
            PExFileNode* file = aNode->toFile();
            if (!file) continue;
            if (file->parentNode() != project)
                closeNode(file);
        }
    }
    emit openRecentFile();
}

void ProjectRepo::reassignFiles(PExProjectNode *project)
{
    QVector<PExFileNode *> files = project->listFiles();
    FileMeta *runGms = project->runnableGms();
    for (PExFileNode *file: qAsConst(files)) {
        addToProject(project, file, true);
    }
    emit openRecentFile();
    project->setRunnableGms(runGms);
}

void ProjectRepo::editorActivated(QWidget* edit, bool select)
{
    PExAbstractNode *node = findProjectForOptions(edit);
    if (!node)
        node = findFileNode(edit);
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
        runIcons << Theme::icon(":/img/project-run1", mode, alpha);
        runIcons << Theme::icon(":/img/project-run2", mode, alpha);
        runIcons << Theme::icon(":/img/project-run3", mode, alpha);
        runIcons << Theme::icon(":/img/project-run4", mode, alpha);
        runIcons << Theme::icon(":/img/project-run5", mode, alpha);
        runIcons << Theme::icon(":/img/project-run6", mode, alpha);
        mRunIcons.insert(key, runIcons);
        mRunIconCount = runIcons.count();
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
