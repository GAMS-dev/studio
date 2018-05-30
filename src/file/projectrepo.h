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
    inline ProjectGroupNode* groupNode(NodeId id) const;

    /// \brief Get the <c>ProjectGroupNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectGroupNode</c> or a <c>nullptr</c>.
    inline ProjectGroupNode* groupNode(const QModelIndex& index) const;

    /// Get the <c>ProjectFileNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectFileNode</c>.
    /// \return The associated <c>ProjectFileNode</c> or a <c>nullptr</c>.
    inline ProjectFileNode* fileNode(NodeId id) const;

    /// \brief Get the <c>ProjectFileNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectFileNode</c>.
    /// \return The associated <c>ProjectFileNode</c> or a <c>nullptr</c>.
    inline ProjectFileNode* fileNode(const QModelIndex& index) const;

    /// Get the <c>ProjectLogNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectLogNode</c>.
    /// \return The associated <c>ProjectLogNode</c> or a <c>nullptr</c>.
    inline ProjectLogNode* logNode(NodeId id) const;

    /// \brief Get the <c>ProjectLogNode</c> related to a parent or sibling <c>ProjectAbstractNode</c>.
    /// \param node The <c>ProjectAbstractNode</c> to find the associated <c>ProjectLogNode</c> for.
    /// \return The associated <c>ProjectLogNode</c> or a <c>nullptr</c>.
    inline ProjectLogNode* logNode(ProjectAbstractNode* node);

//    ProjectFileNode* fileNode(QWidget* edit) const;
//    ProjectLogNode* logNode(QWidget* edit);

private:
    inline void storeNode(ProjectAbstractNode* node) {
        mNode.insert(node->id(), node);
    }
    inline void deleteNode(ProjectAbstractNode* node) {
        node->setParentEntry(nullptr);
        mNode.remove(node->id());
        delete node;
    }

private:
    NodeId mNextId;
    ProjectTreeModel* mTreeModel = nullptr;
    QHash<NodeId, ProjectAbstractNode*> mNode;

/*



    QWidgetList editors(NodeId nodeId = -1);

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
    ProjectFileNode* addFile(QString name, QString location, ProjectGroupNode* parent = nullptr);

    ProjectGroupNode* ensureGroup(const QString& filePath);
    void close(NodeId nodeId);
    void setSuffixFilter(QStringList filter);
    void dump(ProjectAbstractNode* fc, int lv = 0);
    QModelIndex findEntry(QString name, QString location, QModelIndex parentIndex);
    ProjectAbstractNode* findNode(QString filePath, ProjectGroupNode* fileGroup = nullptr);
    ProjectGroupNode* findGroup(const QString &fileName);
    QList<ProjectFileNode*> modifiedFiles(ProjectGroupNode* fileGroup = nullptr);
    int saveAll();
    void editorActivated(QWidget* edit);
    ProjectTreeModel* treeModel() const;
    void removeMarks(ProjectGroupNode* group);

    void updateLinkDisplay(AbstractEditor* editUnderCursor);
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;


signals:
    void fileClosed(NodeId nodeId, QPrivateSignal);
    void fileChangedExtern(NodeId nodeId);
    void fileDeletedExtern(NodeId nodeId);
    void openFile(ProjectFileNode* fileNode, bool focus = true, int codecMib = -1);
    void gamsProcessStateChanged(ProjectGroupNode* group);
    void setNodeExpanded(const QModelIndex &mi, bool expanded = true);
    void getNodeExpanded(const QModelIndex &mi, bool *expanded);

public slots:
    void nodeChanged(NodeId nodeId);
    void findFile(QString filePath, ProjectFileNode** resultFile, ProjectGroupNode* fileGroup = nullptr);
    void findOrCreateFileNode(QString filePath, ProjectFileNode *&resultFile, ProjectGroupNode* fileGroup = nullptr);
    void setSelected(const QModelIndex& ind);
    void removeGroup(ProjectGroupNode* fileGroup);
    void removeFile(ProjectFileNode* file);

private slots:
    void onFileChangedExtern(NodeId nodeId);
    void onFileDeletedExtern(NodeId nodeId);
    void processExternFileEvents();
    void addNode(QString name, QString location, ProjectGroupNode* parent = nullptr);
    void removeNode(ProjectAbstractNode *node);

private:
    void writeGroup(const ProjectGroupNode* group, QJsonArray &jsonArray) const;
    void readGroup(ProjectGroupNode* group, const QJsonArray &jsonArray);

private:
    QStringList mSuffixFilter;
    QList<NodeId> mChangedIds;
    QList<NodeId> mDeletedIds;

*/
};

} // namespace studio
} // namespace gams

#endif // PROJECTREPO_H
