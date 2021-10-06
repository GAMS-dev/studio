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
#include "pexlognode.h"
#include "pexabstractnode.h"
#include "pexfilenode.h"
#include "pexgroupnode.h"
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
    ~ProjectRepo() override;
    void init(ProjectTreeView *treeView, FileMetaRepo* fileRepo, TextMarkRepo* textMarkRepo);

    PExProjectNode *findProject(NodeId nodeId) const;
    PExProjectNode *findProject(const AbstractProcess* process, PExGroupNode *group = nullptr) const;
    PExFileNode *findFile(QString filePath, PExGroupNode *fileGroup = nullptr) const;
    PExFileNode *findFile(FileMeta *fileMeta, PExGroupNode *fileGroup = nullptr) const;

    /// Get the <c>PExAbstractNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>PExAbstractNode</c>.
    /// \return The associated <c>PExAbstractNode</c> or a <c>nullptr</c>.
    PExAbstractNode* node(NodeId id) const;
    PExAbstractNode* node(const QModelIndex& index) const;

    /// Get the <c>PExGroupNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>PExGroupNode</c>.
    /// \return The associated <c>PExGroupNode</c> or a <c>nullptr</c>.
    PExGroupNode* asGroup(NodeId id) const;

    /// \brief Get the <c>PExGroupNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>PExGroupNode</c>.
    /// \return The associated <c>PExGroupNode</c> or a <c>nullptr</c>.
    PExGroupNode* asGroup(const QModelIndex& index) const;

    /// Get the <c>PExProjectNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>PExGroupNode</c>.
    /// \return The associated <c>PExProjectNode</c> or a <c>nullptr</c>.
    PExProjectNode* asProject(NodeId id) const;

    /// \brief Get the <c>PExProjectNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>PExGroupNode</c>.
    /// \return The associated <c>PExProjectNode</c> or a <c>nullptr</c>.
    PExProjectNode* asProject(const QModelIndex& index) const;

    /// Get the <c>PExFileNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>PExFileNode</c>.
    /// \return The associated <c>PExFileNode</c> or a <c>nullptr</c>.
    PExFileNode* asFileNode(NodeId id) const;

    /// \brief Get the <c>PExFileNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>PExFileNode</c>.
    /// \return The associated <c>PExFileNode</c> or a <c>nullptr</c>.
    PExFileNode* asFileNode(const QModelIndex& index) const;

    PExFileNode* findFileNode(QWidget *editWidget) const;

    PExAbstractNode* next(PExAbstractNode* node);
    PExAbstractNode* previous(PExAbstractNode* node);

    /// Get the <c>PExLogNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>PExLogNode</c>.
    /// \return The associated <c>PExLogNode</c> or a <c>nullptr</c>.
    inline PExLogNode* asLogNode(NodeId id) const;

    /// \brief Get the <c>PExLogNode</c> related to a parent or sibling <c>PExAbstractNode</c>.
    /// \param node The <c>PExAbstractNode</c> to find the associated <c>PExLogNode</c> for.
    /// \return The associated <c>PExLogNode</c> or a <c>nullptr</c>.
    PExLogNode* asLogNode(PExAbstractNode* node);

    ProjectTreeModel* treeModel() const;
    FileMetaRepo* fileRepo() const;
    TextMarkRepo* textMarkRepo() const;

    void read(const QVariantList &data, const QString &workDir = QString());
    void write(QVariantList &projects) const;
    void write(PExProjectNode *project, QVariantList &projects, bool relativePaths = false) const;

    PExProjectNode *createProject(QString name, QString path, QString runFileName);
    PExGroupNode *findOrCreateFolder(QString folderName, PExGroupNode *parentNode, bool isAbs);
    PExFileNode *findOrCreateFileNode(QString location, PExProjectNode *project = nullptr, FileType *knownType = nullptr
            , QString explicitName = QString());
    PExFileNode *findOrCreateFileNode(FileMeta* fileMeta, PExProjectNode *project = nullptr, QString explicitName = QString());
    QVector<PExFileNode*> fileNodes(const FileId &fileId, const NodeId &groupId = NodeId()) const;
    QVector<PExProjectNode*> projects(const FileId &fileId = FileId()) const;
    QVector<AbstractProcess*> listProcesses();
    void editorActivated(QWidget *edit, bool select);

    PExLogNode *logNode(PExAbstractNode *node);
    void saveNodeAs(PExFileNode* node, const QString &target);
    void closeGroup(PExGroupNode* group);
    void closeNode(PExFileNode* node);
    void purgeGroup(PExGroupNode *group);

    void setDebugMode(bool debug);
    bool debugMode() const;
    QIcon runAnimateIcon(QIcon::Mode mode = QIcon::Normal, int alpha = 100);

signals:
    void gamsProcessStateChanged(PExGroupNode* group);
    void setNodeExpanded(const QModelIndex &mi, bool expanded = true);
    void isNodeExpanded(const QModelIndex &mi, bool &expanded) const;
    void loadProjects(const QString &gspFile);
    void openFile(FileMeta* fileMeta, bool focus = true, PExProjectNode *project = nullptr, int codecMib = -1,
                  bool forcedAsTextEditor = false, NewTabStrategy tabStrategy = tabAfterCurrent);
    void changed();
    void childrenChanged();
    void parentAssigned(const PExAbstractNode *node);
    void deselect(const QVector<QModelIndex> &declined);
    void select(const QVector<QModelIndex> &selected);
    void closeFileEditors(FileId fileId);
    void getParameterValue(QString param, QString &value);

public slots:
    void gamsProcessStateChange(PExGroupNode* group);
    void fileChanged(FileId fileId);
    void nodeChanged(NodeId nodeId);
    void closeNodeById(NodeId nodeId);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
//    void markTexts(NodeId groupId, const QList<TextMark*> &marks, QStringList &result);
    void errorTexts(NodeId groupId, const QVector<int> &lstLines, QStringList &result);
    void stepRunAnimation();
    void dropFiles(QModelIndex idx, QStringList files, QList<NodeId> knownIds, Qt::DropAction act,
                   QList<QModelIndex> &newSelection);
    void reassignFiles(PExProjectNode *project);

private:
    friend class PExProjectNode;

    void readProjectFiles(PExProjectNode *project, const QVariantList &children, const QString &workDir = QString());
    void writeProjectFiles(const PExProjectNode *project, QVariantList &childList, bool relativePaths = false) const;
    void addToProject(PExProjectNode *project, PExFileNode *file, bool withFolders);

    inline void addToIndex(PExAbstractNode* node) {
        mNodes.insert(node->id(), node);
    }
    inline void removeFromIndex(PExAbstractNode* node) {
        mNodes.remove(node->id());
    }
    bool parseGdxHeader(QString location);

private:
    FileId mNextId;
    ProjectTreeView* mTreeView = nullptr;
    ProjectTreeModel* mTreeModel = nullptr;
    QHash<NodeId, PExAbstractNode*> mNodes;
    QVector<PExAbstractNode*> mActiveStack;
    FileMetaRepo* mFileRepo = nullptr;
    TextMarkRepo* mTextMarkRepo = nullptr;
    QVector<PExProjectNode*> mRunnigGroups;
    QTimer mRunAnimateTimer;
    QHash<QPair<QIcon::Mode, int>, QVector<QIcon>> mRunIcons;
    int mRunIconCount = 1;
    int mRunAnimateIndex = 0;
    bool mDebugMode = false;
};

} // namespace studio
} // namespace gams

#endif // PROJECTREPO_H
