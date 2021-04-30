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
#include "filemetarepo.h"
#include "projecttreeview.h"

namespace gams {
namespace studio {

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
    void init(ProjectTreeView *treeView, FileMetaRepo* fileRepo, TextMarkRepo* textMarkRepo);

    ProjectRunGroupNode *findRunGroup(NodeId nodeId) const;
    ProjectRunGroupNode *findRunGroup(const AbstractProcess* process, ProjectGroupNode *group = nullptr) const;
    ProjectFileNode *findFile(QString filePath, ProjectGroupNode *fileGroup = nullptr) const;
    ProjectFileNode *findFile(FileMeta *fileMeta, ProjectGroupNode *fileGroup = nullptr, bool recurse = true) const;

    /// Get the <c>ProjectAbstractNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectAbstractNode</c>.
    /// \return The associated <c>ProjectAbstractNode</c> or a <c>nullptr</c>.
    ProjectAbstractNode* node(NodeId id) const;
    ProjectAbstractNode* node(const QModelIndex& index) const;

    /// Get the <c>ProjectGroupNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectGroupNode</c> or a <c>nullptr</c>.
    ProjectGroupNode* asGroup(NodeId id) const;

    /// \brief Get the <c>ProjectGroupNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectGroupNode</c> or a <c>nullptr</c>.
    ProjectGroupNode* asGroup(const QModelIndex& index) const;

    /// Get the <c>ProjectRunGroupNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectRunGroupNode</c> or a <c>nullptr</c>.
    ProjectRunGroupNode* asRunGroup(NodeId id) const;

    /// \brief Get the <c>ProjectRunGroupNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectGroupNode</c>.
    /// \return The associated <c>ProjectRunGroupNode</c> or a <c>nullptr</c>.
    ProjectRunGroupNode* asRunGroup(const QModelIndex& index) const;

    /// Get the <c>ProjectFileNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectFileNode</c>.
    /// \return The associated <c>ProjectFileNode</c> or a <c>nullptr</c>.
    ProjectFileNode* asFileNode(NodeId id) const;

    /// \brief Get the <c>ProjectFileNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>ProjectFileNode</c>.
    /// \return The associated <c>ProjectFileNode</c> or a <c>nullptr</c>.
    ProjectFileNode* asFileNode(const QModelIndex& index) const;

    ProjectFileNode* findFileNode(QWidget *editWidget) const;

    ProjectAbstractNode* next(ProjectAbstractNode* node);
    ProjectAbstractNode* previous(ProjectAbstractNode* node);

    /// Get the <c>ProjectLogNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>ProjectLogNode</c>.
    /// \return The associated <c>ProjectLogNode</c> or a <c>nullptr</c>.
    inline ProjectLogNode* asLogNode(NodeId id) const;

    /// \brief Get the <c>ProjectLogNode</c> related to a parent or sibling <c>ProjectAbstractNode</c>.
    /// \param node The <c>ProjectAbstractNode</c> to find the associated <c>ProjectLogNode</c> for.
    /// \return The associated <c>ProjectLogNode</c> or a <c>nullptr</c>.
    ProjectLogNode* asLogNode(ProjectAbstractNode* node);

    ProjectTreeModel* treeModel() const;
    FileMetaRepo* fileRepo() const;
    TextMarkRepo* textMarkRepo() const;

    void read(const QVariantList &data);
    void write(QVariantList &projects) const;

    ProjectGroupNode *createGroup(QString name, QString path, QString runFileName, ProjectGroupNode *_parent = nullptr);
    ProjectFileNode *findOrCreateFileNode(QString location, ProjectGroupNode *fileGroup = nullptr, FileType *knownType = nullptr
            , QString explicitName = QString());
    ProjectFileNode *findOrCreateFileNode(FileMeta* fileMeta, ProjectGroupNode *fileGroup = nullptr, QString explicitName = QString());
    QVector<ProjectFileNode*> fileNodes(const FileId &fileId, const NodeId &groupId = NodeId()) const;
    QVector<ProjectRunGroupNode*> runGroups(const FileId &fileId = FileId()) const;
    QVector<AbstractProcess*> listProcesses();
    void editorActivated(QWidget *edit, bool select);

    ProjectLogNode *logNode(ProjectAbstractNode *node);
    void saveNodeAs(ProjectFileNode* node, const QString &target);
    void closeGroup(ProjectGroupNode* group);
    void closeNode(ProjectFileNode* node);
    void purgeGroup(ProjectGroupNode *group);

    void setDebugMode(bool debug);
    bool debugMode() const;
    QIcon runAnimateIcon(QIcon::Mode mode = QIcon::Normal, int alpha = 100);

signals:
    void gamsProcessStateChanged(ProjectGroupNode* group);
    void setNodeExpanded(const QModelIndex &mi, bool expanded = true);
    void isNodeExpanded(const QModelIndex &mi, bool &expanded) const;
    void openFile(FileMeta* fileMeta, bool focus = true, ProjectRunGroupNode *runGroup = nullptr, int codecMib = -1,
                  bool forcedAsTextEditor = false, NewTabStrategy tabStrategy = tabAfterCurrent);
    void changed();
    void childrenChanged();
    void parentAssigned(const ProjectAbstractNode *node);
    void deselect(const QVector<QModelIndex> &declined);
    void select(const QVector<QModelIndex> &selected);
    void closeFileEditors(FileId fileId);
    void getParameterValue(QString param, QString &value);

public slots:
    void gamsProcessStateChange(ProjectGroupNode* group);
    void fileChanged(FileId fileId);
    void nodeChanged(NodeId nodeId);
    void closeNodeById(NodeId nodeId);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
//    void markTexts(NodeId groupId, const QList<TextMark*> &marks, QStringList &result);
    void errorTexts(NodeId groupId, const QVector<int> &lstLines, QStringList &result);
    void stepRunAnimation();
    void dropFiles(QModelIndex idx, QStringList files, QList<NodeId> knownIds, Qt::DropAction act,
                   QList<QModelIndex> &newSelection);
    void renameGroup(ProjectGroupNode *group);

private:
    friend class ProjectRunGroupNode;

    void writeGroup(const ProjectGroupNode* group, QVariantList &childList) const;
    void readGroup(ProjectGroupNode* group, const QVariantList &children);
    inline void addToIndex(ProjectAbstractNode* node) {
        mNodes.insert(node->id(), node);
    }
    inline void removeFromIndex(ProjectAbstractNode* node) {
        mNodes.remove(node->id());
    }
    bool parseGdxHeader(QString location);

private:
    FileId mNextId;
    ProjectTreeView* mTreeView = nullptr;
    ProjectTreeModel* mTreeModel = nullptr;
    QHash<NodeId, ProjectAbstractNode*> mNodes;
    QVector<ProjectAbstractNode*> mActiveStack;
    FileMetaRepo* mFileRepo = nullptr;
    TextMarkRepo* mTextMarkRepo = nullptr;
    QVector<ProjectRunGroupNode*> mRunnigGroups;
    QTimer mRunAnimateTimer;
    QHash<QPair<QIcon::Mode, int>, QVector<QIcon>> mRunIcons;
    int mRunAnimateIndex = 0;
    bool mDebugMode = false;
};

} // namespace studio
} // namespace gams

#endif // PROJECTREPO_H
