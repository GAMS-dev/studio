/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef FILEREPOSITORY_H
#define FILEREPOSITORY_H

#include <QtWidgets>
#include "filetreemodel.h"
#include "fileactioncontext.h"
#include "logcontext.h"
#include "filecontext.h"
#include "filegroupcontext.h"
#include "filetype.h"

namespace gams {
namespace studio {


typedef unsigned int FileId; // TODO(JM) always use this type for fileIds

///
/// The FileRepository handles all open and assigned files of projects or simple gms-runables. It is based on an
/// QAbstractItemModel to provide a model for a QTreeView. The model has two default nodes: the **root** as base node
/// and the **tree root** as the first child of root. The normal tree view should use the tree root as base node.
///
class FileRepository : public QObject
{
    Q_OBJECT
public:
    explicit FileRepository(QObject *parent = nullptr);
    ~FileRepository();

    /// Defines actions for the <c>FileRepository</c>. For each action a <c>FileActionContext</c>-node is generated
    /// that is shown only, if there are no other nodes in the tree.
    /// \param directActions A list of actions to be triggered directly.
    void setDefaultActions(QList<QAction*> directActions);

    /// \brief Get the <c>FileSystemContext</c> related to a file Id.
    /// \param fileId The file Id related to a <c>FileSystemContext</c>.
    /// \param startNode The node where the search starts.
    /// \return Returns the <c>FileSystemContext</c> pointer related to a file Id; otherwise <c>nullptr</c>.
    FileSystemContext* context(int fileId, FileSystemContext* startNode = nullptr);

    /// \brief Get the <c>FileSystemContext</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>FileSystemContext</c>.
    /// \return The associated <c>FileSystemContext</c>.
    FileSystemContext* context(const QModelIndex& index) const;

    /// \brief Get the <c>FileContext</c> related to a file Id.
    /// \param fileId The file Id related to a <c>FileContext</c>.
    /// \param startNode The node where the search starts.
    /// \return Returns the <c>FileContext</c> pointer related to a file Id; otherwise <c>nullptr</c>.
    FileContext* fileContext(int fileId, FileSystemContext* startNode = nullptr);

    /// \brief Get the <c>FileContext</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>FileContext</c>.
    /// \return The associated <c>FileContext</c>, otherwise <c>nullptr</c>.
    FileContext* fileContext(const QModelIndex& index) const;

    /// \brief Get the <c>FileContext</c> related to a <c>QPlainTextEdit</c>.
    /// \param edit The <c>QPlainTextEdit</c> assigned to the <c>FileContext</c>.
    /// \return The associated <c>FileContext</c>, otherwise <c>nullptr</c>.
    FileContext* fileContext(QPlainTextEdit* edit);

    /// \brief Get the <c>FileGroupContext</c> related to a file Id.
    /// \param fileId The file Id related to a <c>FileGroupContext</c>.
    /// \param startNode The node where the search starts.
    /// \return Returns the <c>FileGroupContext</c> pointer related to the file Id; otherwise <c>nullptr</c>.
    FileGroupContext* groupContext(int fileId, FileSystemContext* startNode = nullptr);

    /// \brief Get the <c>FileGroupContext</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>FileGroupContext</c>.
    /// \return The associated <c>FileGroupContext</c>, otherwise <c>nullptr</c>.
    FileGroupContext* groupContext(const QModelIndex& index) const;

    /// \brief Get the <c>FileActionContext</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>FileActionContext</c>.
    /// \return The associated <c>FileActionContext</c>, otherwise <c>nullptr</c>.
    FileActionContext* actionContext(const QModelIndex& index) const;

    QList<QPlainTextEdit*> editors(int fileId = -1);

    /// Adds a group node to the file repository. This will watch the location for changes.
    /// \param name The name of the project (or gist).
    /// \param location The location of the directory.
    /// \param projectFile The file name w/o path of the project OR gms-start-file
    /// \param parentIndex The parent of this node (default: rootTreeModelIndex)
    /// \return Model index to the new FileGroupContext.
    FileGroupContext* addGroup(QString name, QString location, QString runInfo, QModelIndex parentIndex = QModelIndex());

    /// Adds a file node to the file repository.
    /// \param name The filename without path.
    /// \param location The filename with full path.
    /// \param parentIndex The parent index to assign the file. If invalid the root model index is taken.
    /// \return a <c>QModelIndex</c> to the new node.
    FileContext* addFile(QString name, QString location, FileGroupContext* parent = nullptr);

    void removeNode(FileSystemContext *node);
    FileGroupContext* ensureGroup(const QString& filePath, const QString& additionalFile = "");
    void close(int fileId);
    void setSuffixFilter(QStringList filter);
    void dump(FileSystemContext* fc, int lv = 0);
    QModelIndex findEntry(QString name, QString location, QModelIndex parentIndex);
    FileSystemContext* findContext(QString filePath, FileGroupContext* fileGroup = nullptr);
    QList<FileContext*> modifiedFiles(FileGroupContext* fileGroup = nullptr);
    int saveAll();
    void editorActivated(QPlainTextEdit* edit);
    FileTreeModel* treeModel() const;
    LogContext* logContext(FileSystemContext* node);
    void removeMarks(FileGroupContext* group);

    void updateLinkDisplay(QPlainTextEdit* editUnderCursor);

signals:
    void fileClosed(int fileId, QPrivateSignal);
    void fileChangedExtern(int fileId);
    void fileDeletedExtern(int fileId);
    void openOrShowContext(FileContext* fileContext);

public slots:
    void nodeChanged(int fileId);
    void updatePathNode(int fileId, QDir dir);
    void nodeClicked(QModelIndex index);
    void findFile(QString filePath, FileContext** resultFile, FileGroupContext* fileGroup = nullptr);

private slots:
    void onFileChangedExtern(int fileId);
    void onFileDeletedExtern(int fileId);
    void processExternFileEvents();
    void setErrorHint(const int errCode, const QString& hint);
    void getErrorHint(const int errCode, QString& hint);

private:
    void updateActions();

private:
    int mNextId;
    FileTreeModel* mTreeModel = nullptr;
    QStringList mSuffixFilter;
    QList<int> mChangedIds;
    QList<int> mDeletedIds;
    QList<FileActionContext*> mFileActions;
    QHash<int, QString> mErrorHints;
};

} // namespace studio
} // namespace gams

#endif // FILEREPOSITORY_H
