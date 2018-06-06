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

namespace gams {
namespace studio {

ProjectRepo::ProjectRepo(QObject* parent)
    : QObject(parent), mTreeModel(new ProjectTreeModel(this, new ProjectRootNode()))
{
    storeNode(mTreeModel->rootNode());
}

ProjectRepo::~ProjectRepo()
{
    delete mTreeModel;
}

ProjectGroupNode *ProjectRepo::createGroup(const QString &filePath, ProjectGroupNode *parent)
{
    QFileInfo fi(filePath);
    if (!parent || parent == mTreeModel->rootNode()) {
        if (fi.isFile())
            return createGroup(fi.completeBaseName(), fi.path());
        else
            return createGroup("-unknown-", fi.filePath());
    }

}

ProjectGroupNode* ProjectRepo::findGroup(const QString &filePath, bool createIfMissing)
{
    QFileInfo fi(filePath);
    QFileInfo di(CommonPaths::absolutFilePath(fi.path()));
    ProjectAbstractNode* node = mTreeModel->rootNode()->findNode(di.filePath(), false);
    ProjectGroupNode* group = node ? node->toGroup() : nullptr;
    if (group) return group;
    if (!createIfMissing) return nullptr;

    group = createGroup(fi.completeBaseName(), fi.path(), fi.fileName(), mTreeModel->rootModelIndex());
    group->updateChildNodes();
    return group;
}

ProjectAbstractNode* ProjectRepo::findNode(QString filePath, ProjectGroupNode* fileGroup)
{
    ProjectGroupNode *group = fileGroup ? fileGroup : mTreeModel->rootNode();
    ProjectAbstractNode* fsc = group->findNode(filePath);
    return fsc;
}

inline ProjectAbstractNode *ProjectRepo::node(NodeId id) const
{
    return mNodes.value(id, nullptr);
}

inline ProjectAbstractNode*ProjectRepo::node(const QModelIndex& index) const
{
    return node(index.internalId());
}

inline ProjectGroupNode *ProjectRepo::asGroup(NodeId id) const
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

inline ProjectFileNode *ProjectRepo::asFile(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (res && res->type() == NodeType::file) ? static_cast<ProjectFileNode*>(res) : nullptr;
}

inline ProjectFileNode*ProjectRepo::asFile(const QModelIndex& index) const
{
    return asFile(index.internalId());
}

inline ProjectLogNode *ProjectRepo::asLog(NodeId id) const
{
    ProjectAbstractNode* res = mNodes.value(id, nullptr);
    return (res && res->type() == NodeType::log) ? static_cast<ProjectLogNode*>(res) : nullptr;
}

inline ProjectLogNode* ProjectRepo::asLog(ProjectAbstractNode* node)
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
        QString location = jsonObject["file"].toString("");
        if (jsonObject.contains("nodes")) {
            // group
            if (!name.isEmpty() || !location.isEmpty()) {
                ProjectGroupNode* subGroup = createGroup(name, location, group);
                if (subGroup) {
                    QJsonArray gprArray = jsonObject["nodes"].toArray();
                    readGroup(subGroup, gprArray);
                    if (subGroup->childCount()) {
                        bool expand = jsonObject["expand"].toBool(true);
                        emit setNodeExpanded(mTreeModel->index(subGroup), expand);
                    } else {
                        removeGroup(subGroup); // dont open empty groups
                    }
                }
            }
        } else {
            // file
            if (!name.isEmpty() || !location.isEmpty()) {
                FileMeta* file = fileRepo()->findOrCreateFileMeta(location);
                ProjectAbstractNode* node = group->findNode(location, false);
                if (!node) storeNode(new ProjectFileNode(file, group));
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
        const ProjectRunGroupNode *runGroup = node->toRunGroup();
        bool expand = true;
        if (runGroup) {
            jsonObject["file"] = (!runGroup->runnableGms().isEmpty() ? runGroup->runnableGms() : runGroup->location());
            jsonObject["name"] = node->name();
            const QModelIndex mi = mTreeModel->index(runGroup);
            emit isNodeExpanded(mi, expand);
            if (!expand) jsonObject["expand"] = false;
            QJsonArray subArray;
            writeGroup(runGroup, subArray);
            jsonObject["nodes"] = subArray;
        } else {
            ProjectGroupNode *subGroup = node->toGroup();
            if (subGroup) {
                jsonObject["file"] = subGroup->location();
                jsonObject["name"] = node->name();
                emit isNodeExpanded(mTreeModel->index(runGroup), expand);
                if (!expand) jsonObject["expand"] = false;
                QJsonArray subArray;
                writeGroup(subGroup, subArray);
                jsonObject["nodes"] = subArray;
            } else {
                ProjectFileNode *file = node->toFile();
                jsonObject["file"] = file->location();
                jsonObject["name"] = file->name();
            }
        }
        jsonArray.append(jsonObject);
    }
}

ProjectRunGroupNode* ProjectRepo::createGroup(QString name, QString location, ProjectGroupNode *_parent)
{
    if (!_parent) _parent = mTreeModel->rootNode();
    if (!_parent) FATAL() << "Can't get parent object";

    bool hit;
    int offset = _parent->peekIndex(name, &hit);
    if (hit) offset++;

    ProjectGroupNode* group;
    ProjectRunGroupNode* runGroup = nullptr;
    if (_parent == mTreeModel->rootNode()) {
        FileMeta* file = mFileRepo->findOrCreateFileMeta(location);
        runGroup = new ProjectRunGroupNode(name, location, file);
        group = runGroup;
    } else
        group = new ProjectGroupNode(name, location);

    storeNode(group);
    mTreeModel->insertChild(offset, _parent, group);
    connect(group, &ProjectGroupNode::changed, this, &ProjectRepo::nodeChanged);

//    connect(group, &ProjectGroupNode::removeNode, this, &ProjectRepo::removeNode);
//    connect(group, &ProjectGroupNode::requestNode, this, &ProjectRepo::addNode);
//    connect(group, &ProjectGroupNode::findOrCreateFileNode, this, &ProjectRepo::findOrCreateFileNode);

    if (runGroup)
        connect(runGroup, &ProjectRunGroupNode::gamsProcessStateChanged, this, &ProjectRepo::gamsProcessStateChanged);

    return group;
}

void ProjectRepo::init(FileMetaRepo *fileRepo, TextMarkRepo *textMarkRepo)
{
    if (!fileRepo) FATAL() << "The FileMetaRepo must not be null";
    if (!textMarkRepo) FATAL() << "The TextMarkRepo must not be null";
    mFileRepo = fileRepo;
    mTextMarkRepo = textMarkRepo;
}

void ProjectRepo::removeGroup(ProjectGroupNode* group)
{
    for (int i = 0; i < group->childCount(); ++i) {
        ProjectAbstractNode *node = group->childNode(i);
        ProjectGroupNode* subGroup = node->toGroup();
        if (subGroup) removeGroup(subGroup);
        else {
            mTreeModel->removeChild(node);
            deleteNode(node);
        }
    }
    mTreeModel->removeChild(group);
    deleteNode(group);
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
