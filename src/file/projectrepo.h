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
//#include "projectfilenode.h"
//#include "projectgroupnode.h"
#include "filetype.h"

namespace gams {
namespace studio {

class AbstractProcess;

// TODO(AF)
// - a file which is in different groups
// - naming and documentation of functions
// - function to find a group for a GAMSProcess
// - review class dependencies of FileRepository
// - review function argument, i.e. const strings

///
/// The ProjectRepo handles all open and assigned nodes of projects or simple gms-runables. It is based on an
/// QAbstractItemModel to provide a model for a QTreeView.
///
class ProjectRepo : public QObject
{
    Q_OBJECT
public:
    explicit ProjectRepo(QObject *parent = nullptr);
    ~ProjectRepo();
    void init(FileMetaRepo* fileRepo, TextMarkRepo* textMarkRepo);

    const ProjectGroupNode *findGroup(const QString& filePath);
    ProjectRunGroupNode *findRunGroup(FileId runId, ProjectGroupNode *group = nullptr) const;
    ProjectRunGroupNode *findRunGroup(const AbstractProcess* process, ProjectGroupNode *group = nullptr) const;
    const ProjectAbstractNode *findNode(QString filePath, ProjectGroupNode *fileGroup = nullptr) const;

    /// Get the <c>ProjectAbstractNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectAbstractNode</c>.
    /// \return The associated <c>ProjectAbstractNode</c> or a <c>nullptr</c>.
    inline ProjectAbstractNode* node(NodeId id) const;

    /// Get the <c>ProjectAbstractNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectAbstractNode</c>.
    /// \return The associated <c>ProjectAbstractNode</c> or a <c>nullptr</c>.
    inline ProjectAbstractNode* node(const QModelIndex& index) const;

    /// Get the <c>ProjectGroupNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectGroupNode</c> or a <c>nullptr</c>.
    inline ProjectGroupNode* asGroup(NodeId id) const;

    /// \brief Get the <c>ProjectGroupNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectGroupNode</c> or a <c>nullptr</c>.
    inline ProjectGroupNode* asGroup(const QModelIndex& index) const;

    /// Get the <c>ProjectRunGroupNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectRunGroupNode</c> or a <c>nullptr</c>.
    inline ProjectRunGroupNode* asRunGroup(NodeId id) const;

    /// \brief Get the <c>ProjectRunGroupNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectRunGroupNode</c> or a <c>nullptr</c>.
    inline ProjectRunGroupNode* asRunGroup(const QModelIndex& index) const;

    /// Get the <c>ProjectFileNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectFileNode</c>.
    /// \return The associated <c>ProjectFileNode</c> or a <c>nullptr</c>.
    inline ProjectFileNode* asFile(NodeId id) const;

    /// \brief Get the <c>ProjectFileNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectFileNode</c>.
    /// \return The associated <c>ProjectFileNode</c> or a <c>nullptr</c>.
    inline ProjectFileNode* asFile(const QModelIndex& index) const;

    /// Get the <c>ProjectLogNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectLogNode</c>.
    /// \return The associated <c>ProjectLogNode</c> or a <c>nullptr</c>.
    inline ProjectLogNode* asLog(NodeId id) const;

    /// \brief Get the <c>ProjectLogNode</c> related to a parent or sibling <c>ProjectAbstractNode</c>.
    /// \param node The <c>ProjectAbstractNode</c> to find the associated <c>ProjectLogNode</c> for.
    /// \return The associated <c>ProjectLogNode</c> or a <c>nullptr</c>.
    inline ProjectLogNode* asLog(ProjectAbstractNode* node);

    inline bool isActive(const ProjectAbstractNode *node) const;
    inline void setActive(ProjectAbstractNode* node);

    ProjectTreeModel* treeModel() const;
    FileMetaRepo* fileRepo() const;
    TextMarkRepo* textMarkRepo() const;

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    /// Adds a group node to the file repository. This will watch the location for changes.
    /// \param name The name of the project (or gist).
    /// \param path The location of the directory.
    /// \param runFileName The file name w/o path of the project OR gms-start-file
    /// \param _parent The parent of this node (default: rootTreeModelIndex)
    /// \return the new <c>ProjectRunGroupNode</c>.
    ProjectGroupNode *createGroup(QString name, QString path, QString runFileName, ProjectGroupNode *_parent = nullptr);

signals:
    void gamsProcessStateChanged(ProjectGroupNode* group);
    void setNodeExpanded(const QModelIndex &mi, bool expanded = true);
    void isNodeExpanded(const QModelIndex &mi, bool *expanded) const;
    void openFile(FileMeta* fileMeta, bool focus = true, ProjectRunGroupNode *runGroup = nullptr, int codecMib = -1);

public slots:
    void nodeChanged(NodeId nodeId);
    void removeGroup(ProjectGroupNode* group);

private:
    friend class ProjectRunGroupNode;

    void writeGroup(const ProjectGroupNode* group, QJsonArray &jsonArray) const;
    void readGroup(ProjectGroupNode* group, const QJsonArray &jsonArray);
    inline void indexNode(ProjectAbstractNode* node) {
        mNodes.insert(node->id(), node);
    }
    inline void deleteNode(ProjectAbstractNode* node) {
        node->setParentNode(nullptr);
        mNodes.remove(node->id());
        delete node;
    }

private:
    ProjectTreeModel* mTreeModel = nullptr;
    QHash<NodeId, ProjectAbstractNode*> mNodes;
    QVector<ProjectAbstractNode*> mActiveStack;
    FileMetaRepo* mFileRepo = nullptr;
    TextMarkRepo* mTextMarkRepo = nullptr;

/*



    QWidgetList editors(NodeId nodeId = -1);

    /// Adds a file node to the project repository.
    /// \param name The filename without path.
    /// \param location The filename with full path.
    /// \param parentIndex The parent index to assign the file. If invalid the root model index is taken.
    /// \return a <c>QModelIndex</c> to the new node.
    ProjectFileNode* addFile(QString name, QString location, ProjectGroupNode* parent = nullptr);

    void close(NodeId nodeId);
    void setSuffixFilter(QStringList filter);
    void dump(ProjectAbstractNode* fc, int lv = 0);
    QModelIndex findEntry(QString name, QString location, QModelIndex parentIndex);
    ProjectAbstractNode* findNode(QString filePath, ProjectGroupNode* fileGroup = nullptr);
    QList<ProjectFileNode*> modifiedFiles(ProjectGroupNode* fileGroup = nullptr);
    int saveAll();
    void editorActivated(QWidget* edit);
    void removeMarks(ProjectGroupNode* group);

    void updateLinkDisplay(AbstractEditor* editUnderCursor);


signals:
    void fileClosed(NodeId nodeId, QPrivateSignal);
    void fileChangedExtern(NodeId nodeId);
    void fileDeletedExtern(NodeId nodeId);

public slots:
    void nodeChanged(NodeId nodeId);
    void findFile(QString filePath, ProjectFileNode** resultFile, ProjectGroupNode* fileGroup = nullptr);
    void findOrCreateFileNode(QString filePath, ProjectFileNode *&resultFile, ProjectGroupNode* fileGroup = nullptr);
    void setSelected(const QModelIndex& ind);
    void removeFile(ProjectFileNode* file);

private slots:
    void onFileChangedExtern(NodeId nodeId);
    void onFileDeletedExtern(NodeId nodeId);
    void processExternFileEvents();
    void addNode(QString name, QString location, ProjectGroupNode* parent = nullptr);
    void removeNode(ProjectAbstractNode *node);


private:
    QStringList mSuffixFilter;
    QList<NodeId> mChangedIds;
    QList<NodeId> mDeletedIds;

*/
};

} // namespace studio
} // namespace gams

#endif // PROJECTREPO_H
