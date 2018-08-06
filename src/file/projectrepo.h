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
#ifndef PROJECTREPO_H
#define PROJECTREPO_H

#include <QStringList>
#include <QWidgetList>
#include <QModelIndex>
#include "projecttreemodel.h"
#include "projectlognode.h"
#include "projectabstractnode.h"
#include "projectfilenode.h"
#include "projectgroupnode.h"
#include "filetype.h"

namespace gams {
namespace studio {

// TODO(AF)
// - a file which is in different groups
// - naming and documentation of functions
// - function to find a group for a GAMSProcess
// - review class dependencies of FileRepository
// - review function argument, i.e. const strings

///
/// The FileRepository handles all open and assigned files of projects or simple gms-runables. It is based on an
/// QAbstractItemModel to provide a model for a QTreeView. The model has two default nodes: the **root** as base node
/// and the **tree root** as the first child of root. The normal tree view should use the tree root as base node.
///
class ProjectRepo : public QObject
{
    Q_OBJECT
public:
    explicit ProjectRepo(QObject *parent = nullptr);
    ~ProjectRepo();

    /// \brief Get the <c>ProjectAbstractNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectAbstractNode</c>.
    /// \return The associated <c>ProjectAbstractNode</c>.
    ProjectAbstractNode* node(const QModelIndex& index) const;

    /// \brief Get the <c>ProjectFileNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectFileNode</c>.
    /// \return The associated <c>ProjectFileNode</c>, otherwise <c>nullptr</c>.
    ProjectFileNode* fileNode(const QModelIndex& index) const;

    /// \brief Get the <c>ProjectFileNode</c> related to a <c>QWidget</c>.
    /// \param edit The <c>QWidget</c> assigned to the <c>ProjectFileNode</c>.
    /// \return The associated <c>ProjectFileNode</c>, otherwise <c>nullptr</c>.
    ProjectFileNode* fileNode(QWidget* edit) const;

    /// \brief Get the <c>ProjectGroupNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectGroupNode</c>, otherwise <c>nullptr</c>.
    ProjectGroupNode* groupNode(const QModelIndex& index) const;

    QWidgetList editors(FileId fileId = -1);

    /// Adds a group node to the file repository. This will watch the location for changes.
    /// \param name The name of the project (or gist).
    /// \param location The location of the directory.
    /// \param projectFile The file name w/o path of the project OR gms-start-file
    /// \param parentIndex The parent of this node (default: rootTreeModelIndex)
    /// \return Model index to the new <c>ProjectGroupNode</c>.
    ProjectGroupNode* addGroup(QString name, QString location, QString runInfo, QModelIndex parentIndex = QModelIndex());

    /// Adds a file node to the project repository.
    /// \param name The filename without path.
    /// \param location The filename with full path.
    /// \param parentIndex The parent index to assign the file. If invalid the root model index is taken.
    /// \return a <c>QModelIndex</c> to the new node.
    ProjectFileNode* addFile(QString name, QString location, ProjectGroupNode* parent, FileType* fileType = nullptr);

    ProjectGroupNode* ensureGroup(const QString& filePath, const QString& groupName = "");
    void close(FileId fileId);
    void setSuffixFilter(QStringList filter);
    void dump(ProjectAbstractNode* fc, int lv = 0);
    QModelIndex findEntry(QString name, QString location, QModelIndex parentIndex);
    ProjectAbstractNode* findNode(QString filePath, ProjectGroupNode* fileGroup = nullptr);
    ProjectGroupNode* findGroup(const QString &fileName);
    QList<ProjectFileNode*> modifiedFiles(ProjectGroupNode* fileGroup = nullptr);
    int saveAll();
    void editorActivated(QWidget* edit);
    ProjectTreeModel* treeModel() const;
    ProjectLogNode* logNode(QWidget* edit);
    ProjectLogNode* logNode(ProjectAbstractNode* node);
    void removeMarks(ProjectGroupNode* group);

    void updateLinkDisplay(AbstractEdit* editUnderCursor);
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    inline ProjectAbstractNode* node(FileId id) const {
        return mNode.value(id, nullptr);
    }
    inline ProjectGroupNode* groupNode(FileId id) const {
        ProjectAbstractNode* res = mNode.value(id, nullptr);
        return (res && res->type() == ProjectAbstractNode::FileGroup) ? static_cast<ProjectGroupNode*>(res) : nullptr;
    }
    inline ProjectFileNode* fileNode(FileId id) const {
        ProjectAbstractNode* res = mNode.value(id, nullptr);
        return (res && res->type() == ProjectAbstractNode::File) ? static_cast<ProjectFileNode*>(res) : nullptr;
    }
    inline ProjectLogNode* logNode(FileId id) const {
        ProjectAbstractNode* res = mNode.value(id, nullptr);
        return (res && res->type() == ProjectAbstractNode::Log) ? static_cast<ProjectLogNode*>(res) : nullptr;
    }

signals:
    void fileClosed(FileId fileId, QPrivateSignal);
    void fileChangedExtern(FileId fileId);
    void fileDeletedExtern(FileId fileId);
    void openFile(ProjectFileNode* fileNode, bool focus = true, int codecMib = -1);
    void gamsProcessStateChanged(ProjectGroupNode* group);
    void setNodeExpanded(const QModelIndex &mi, bool expanded = true);
    void getNodeExpanded(const QModelIndex &mi, bool *expanded);

public slots:
    void nodeChanged(FileId fileId);
    void findFile(QString filePath, ProjectFileNode** resultFile, ProjectGroupNode* fileGroup = nullptr);
    void findOrCreateFileNode(QString filePath, ProjectFileNode *&resultFile, ProjectGroupNode* fileGroup);
    void setSelected(const QModelIndex& ind);
    void removeGroup(ProjectGroupNode* fileGroup);
    void removeFile(ProjectFileNode* file);

private slots:
    void onFileChangedExtern(FileId fileId);
    void onFileDeletedExtern(FileId fileId);
    void processExternFileEvents();
    void addNode(QString name, QString location, ProjectGroupNode* parent = nullptr);
    void removeNode(ProjectAbstractNode *node);

private:
    void writeGroup(const ProjectGroupNode* group, QJsonArray &jsonArray) const;
    void readGroup(ProjectGroupNode* group, const QJsonArray &jsonArray);
    inline void storeNode(ProjectAbstractNode* node) {
        mNode.insert(node->id(), node);
    }
    inline void deleteNode(ProjectAbstractNode* node) {
        node->setParentEntry(nullptr);
        mNode.remove(node->id());
        delete node;
    }
    bool parseGdxHeader(QString location);

private:
    FileId mNextId;
    ProjectTreeModel* mTreeModel = nullptr;
    QStringList mSuffixFilter;
    QList<FileId> mChangedIds;
    QList<FileId> mDeletedIds;
    QHash<FileId, ProjectAbstractNode*> mNode;
};

} // namespace studio
} // namespace gams

#endif // PROJECTREPO_H
