/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "projectproxymodel.h"
#include "projecttreemodel.h"
#include "pexlognode.h"
#include "pexabstractnode.h"
#include "pexfilenode.h"
#include "pexgroupnode.h"
#include "filetype.h"
#include "filemetarepo.h"
#include "projecttreeview.h"
#include "pexgroupnode.h"

namespace gams {
namespace studio {

enum ProjectExistFlag {
    onExist_Null,
    onExist_Project,
    onExist_AddNr,
};
enum MultiCopyCheck {
    mcsOk,
    mcsMiss,
    mcsCollide,
    mcsMissCollide,
    mcsMissAll,
};

///
/// The ProjectRepo handles all open and assigned nodes of projects or simple gms-runnables. It is based on an
/// QAbstractItemModel to provide a model for a QTreeView.
///
class ProjectRepo : public QObject
{
    Q_OBJECT
public:
    explicit ProjectRepo(QObject *parent = nullptr);
    ~ProjectRepo() override;
    void init(ProjectTreeView *treeView, FileMetaRepo* fileRepo, TextMarkRepo* textMarkRepo);

    PExProjectNode *findProject(const QString &projectFile) const;
    PExProjectNode *findProject(const NodeId& nodeId) const;
    PExProjectNode *findProject(const AbstractProcess* process, PExGroupNode *group = nullptr) const;
    PExProjectNode *findProject(QWidget *edit) const;
    PExProjectNode *findProjectForPEdit(QWidget *projectEdit) const;
    PExFileNode *findFile(const QString& filePath, PExGroupNode *fileGroup = nullptr) const;
    PExFileNode *findFile(FileMeta *fileMeta, PExGroupNode *fileGroup = nullptr) const;

    /// Get the <c>PExAbstractNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>PExAbstractNode</c>.
    /// \return The associated <c>PExAbstractNode</c> or a <c>nullptr</c>.
    PExAbstractNode* node(const NodeId &id) const;
    PExAbstractNode* node(const QModelIndex& index) const;

    /// Get the <c>PExGroupNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>PExGroupNode</c>.
    /// \return The associated <c>PExGroupNode</c> or a <c>nullptr</c>.
    PExGroupNode* asGroup(const NodeId &id) const;

    /// \brief Get the <c>PExGroupNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>PExGroupNode</c>.
    /// \return The associated <c>PExGroupNode</c> or a <c>nullptr</c>.
    PExGroupNode* asGroup(const QModelIndex& index) const;

    /// Get the <c>PExProjectNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>PExGroupNode</c>.
    /// \return The associated <c>PExProjectNode</c> or a <c>nullptr</c>.
    PExProjectNode* asProject(const NodeId &id) const;

    /// \brief Get the <c>PExProjectNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>PExGroupNode</c>.
    /// \return The associated <c>PExProjectNode</c> or a <c>nullptr</c>.
    PExProjectNode* asProject(const QModelIndex& index) const;

    /// Get the <c>PExFileNode</c> related to a <c>NodeId</c>.
    /// \param id The NodeId pointing to the <c>PExFileNode</c>.
    /// \return The associated <c>PExFileNode</c> or a <c>nullptr</c>.
    PExFileNode* asFileNode(const NodeId &id) const;

    /// \brief Get the <c>PExFileNode</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>PExFileNode</c>.
    /// \return The associated <c>PExFileNode</c> or a <c>nullptr</c>.
    PExFileNode* asFileNode(const QModelIndex& index) const;

    PExFileNode* findFileNode(QWidget *editWidget) const;

    PExAbstractNode* next(PExAbstractNode* node);
    PExAbstractNode* previous(PExAbstractNode* node);
    PExProjectNode* gamsSystemProject();

    ProjectProxyModel* proxyModel() const;
    FileMetaRepo* fileRepo() const;
    TextMarkRepo* textMarkRepo() const;

    bool checkRead(const QVariantMap &map, int &count, int &ignored, QStringList &missed, const QString &baseDir);
    bool readList(const QVariantList &projectsList);
    bool read(const QVariantMap &projectMap, QString gspFile = QString());
    void write(QVariantList &projects) const;
    void save(PExProjectNode *project, const QVariantMap &data) const;
    QVariantMap getProjectMap(PExProjectNode *project, bool relativePaths = false) const;

    PExProjectNode *createProject(QString name, const QString &path, const QString &runFileName, ProjectExistFlag mode,
                                  const QString &workDir = QString(), PExProjectNode::Type type = PExProjectNode::tSmall);
    MultiCopyCheck getCopyPaths(PExProjectNode *project, const QString &filePath, QStringList &srcFiles, QStringList &dstFiles, QStringList &missFiles, QStringList &existFiles);
    void moveProject(PExProjectNode *project, const QString &filePath, bool fullCopy);
    PExGroupNode *findOrCreateFolder(const QString &folderName, PExGroupNode *parentNode, bool isAbs);
    PExFileNode *findOrCreateFileNode(QString location, PExProjectNode *project = nullptr, FileType *knownType = nullptr
            , const QString &explicitName = QString());
    PExFileNode *findOrCreateFileNode(FileMeta* fileMeta, PExProjectNode *project = nullptr, const QString &explicitName = QString());
    QVector<PExFileNode*> fileNodes(const FileId &fileId, const NodeId &groupId = NodeId()) const;
    const QList<PExProjectNode *> projects(const FileId &fileId) const;
    const QList<PExProjectNode *> projects() const;
    const QVector<AbstractProcess*> listProcesses();
    void editorActivated(QWidget *edit, bool select);
    void editProjectName(PExProjectNode *project);

    PExLogNode *logNode(PExAbstractNode *node);
    void saveNodeAs(PExFileNode* node, const QString &target);
    void closeGroup(PExGroupNode* group);
    void closeNode(PExFileNode* node);
    void purgeGroup(PExGroupNode *group);
    void sortChildNodes(PExGroupNode *group);
    void focusProject(PExProjectNode *project);
    PExProjectNode *focussedProject() const;
    void storeExpansionState(QModelIndex parent);
    void restoreExpansionState(QModelIndex parent);
    bool isExpanded(NodeId id, bool *ok) const;

    void setDebugMode(bool debug);
    bool debugMode() const;
    QIcon runAnimateIcon(QIcon::Mode mode = QIcon::Normal, int alpha = 100);
    int activeProcesses();

signals:
    void gamsProcessStateChanged(gams::studio::PExProjectNode* project);
    void openProject(const QString &gspFile);
    void importGprProject(const QString &gprFile);
    void openFile(gams::studio::FileMeta* fileMeta, bool focus = true, gams::studio::PExProjectNode *project = nullptr,
                  QString encoding = QString(), bool forcedAsTextEditor = false, gams::studio::NewTabStrategy tabStrategy = tabAfterCurrent);
    void openFolder(QString path, gams::studio::PExProjectNode* project);
    void changed();
    void mainFileChanged();
    void updateProfilerAction();
    void childrenChanged();
    void logTabRenamed(QWidget *wid, const QString &newName);
    void refreshProjectTabName(QWidget *wid);
    void openRecentFile();
    void projectListChanged();
    void doFocusProject(PExProjectNode *project);
    void parentAssigned(const gams::studio::PExAbstractNode *node);
    void getConfigPaths(QStringList &configPaths);
    void closeFileEditors(gams::studio::FileId fileId);
    void getParameterValue(QString param, QString &value);
    void addWarning(const QString &warning);
    void openInPinView(gams::studio::PExProjectNode *project, QWidget *editInMainTabs);
    void switchToTab(FileMeta *fileMeta);

public slots:
    void gamsProcessStateChange(PExProjectNode *project);
    void fileChanged(const FileId &fileId);
    void nodeChanged(const NodeId &nodeId);
    void closeNodeById(const NodeId &nodeId);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
//    void markTexts(NodeId groupId, const QList<TextMark*> &marks, QStringList &result);
    void errorTexts(const NodeId &groupId, const QVector<int> &lstLines, QStringList &result);
    void stepRunAnimation();
    void dropFiles(QModelIndex idx, QStringList files, QList<gams::studio::NodeId> knownIds, Qt::DropAction act,
                   QList<QModelIndex> &newSelection);
    void reassignFiles(gams::studio::PExProjectNode *project);

private:
    friend class PExProjectNode;

    QVariantMap parseProjectFile(const QString &gspFile) const;
    bool readProjectFiles(PExProjectNode *project, const QVariantList &children, const QString &baseDir = QString());
    void writeProjectFiles(const PExProjectNode *project, QVariantList &childList, bool relativePaths = false) const;
    void addToProject(PExProjectNode *project, PExFileNode *file);
    QString uniqueNameExt(PExGroupNode *parentNode, const QString &name, PExAbstractNode *node = nullptr);
    void uniqueProjectFile(PExGroupNode *parentNode, QString &name);

    inline void addToIndex(PExAbstractNode* node) {
        mNodes.insert(node->id(), node);
    }
    inline void removeFromIndex(PExAbstractNode* node) {
        mNodes.remove(node->id());
    }
    bool parseGdxHeader(const QString &location);

private:
    FileId mNextId;
    ProjectTreeView* mTreeView = nullptr;
    ProjectTreeModel* mTreeModel = nullptr;
    ProjectProxyModel* mProxyModel = nullptr;
    QHash<NodeId, PExAbstractNode*> mNodes;
    FileMetaRepo* mFileRepo = nullptr;
    TextMarkRepo* mTextMarkRepo = nullptr;
    QVector<PExProjectNode*> mRunnigGroups;
    QTimer mRunAnimateTimer;
    QHash<QPair<QIcon::Mode, int>, QVector<QIcon>> mRunIcons;
    int mRunIconCount = 1;
    int mRunAnimateIndex = 0;
    bool mDebugMode = false;
    QHash<NodeId, bool> mIsExpanded;

    static const QString CIgnoreSuffix;

};

} // namespace studio
} // namespace gams

#endif // PROJECTREPO_H
