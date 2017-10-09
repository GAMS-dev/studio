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
#include "fileactioncontext.h"
#include "filecontext.h"
#include "filegroupcontext.h"

namespace gams {
namespace studio {


typedef unsigned int FileId; // TODO(JM) always use this type for fileIds

///
/// The FileRepository handles all open and assigned files of projects or simple gms-runables. It is based on an
/// QAbstractItemModel to provide a model for a QTreeView. The model has two default nodes: the **root** as base node
/// and the **tree root** as the first child of root. The normal tree view should use the tree root as base node.
///
class FileRepository : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit FileRepository(QObject *parent = nullptr);
    ~FileRepository();

    void setDefaultActions(QList<QAction*> directActions);
    /// \brief Get the <c>FileSystemContext</c> rellated to a file Id.
    /// \param fileId The file Id related to a <c>FileSystemContext</c>.
    /// \param startNode The node where the search starts.
    /// \return Returns the <c>FileSystemContext</c> pointer related to a file Id; otherwise <c>nullptr</c>.
    FileSystemContext* context(int fileId, FileSystemContext* startNode = nullptr);

    /// \brief Get the <c>FileContext</c> related to a file Id.
    /// \param fileId The file Id related to a <c>FileContext</c>.
    /// \param startNode The node where the search starts.
    /// \return Returns the <c>FileContext</c> pointer related to a file Id; otherwise <c>nullptr</c>.
    FileContext* fileContext(int fileId, FileSystemContext* startNode = nullptr);

    /// \brief Get the <c>FileGroupContext</c> related to a file Id.
    /// \param fileId The file Id related to a <c>FileGroupContext</c>.
    /// \param startNode The node where the search starts.
    /// \return Returns the <c>FileGroupContext</c> pointer related to the file Id; otherwise <c>nullptr</c>.
    FileGroupContext* groupContext(int fileId, FileSystemContext* startNode = nullptr);

    QModelIndex index(FileSystemContext* entry);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /// Adds a group node to the file repository. This will watch the location for changes.
    /// \param name The name of the project (or gist).
    /// \param location The location of the directory.
    /// \param projectFile The file name w/o path of the project OR gms-start-file
    /// \param parentIndex The parent of this node (default: rootTreeModelIndex)
    /// \return Model index to the new FileGroupContext.
    QModelIndex addGroup(QString name, QString location, QString runInfo, QModelIndex parentIndex = QModelIndex());

    QModelIndex addFile(QString name, QString location, QModelIndex parentIndex = QModelIndex());

    void removeNode(FileSystemContext *node);
    QModelIndex rootTreeModelIndex();
    QModelIndex rootModelIndex();
    QModelIndex ensureGroup(const QString& filePath);
    void close(int fileId);
    void setSuffixFilter(QStringList filter);
    void dump(FileSystemContext* fc, int lv = 0);
    QModelIndex findEntry(QString name, QString location, QModelIndex parentIndex);

    FileSystemContext* node(const QModelIndex& index) const;
    FileContext* file(const QModelIndex& index) const;
    FileGroupContext* group(const QModelIndex& index) const;
    FileActionContext* action(const QModelIndex& index) const;

signals:
    void fileClosed(int fileId, QPrivateSignal);
    void fileChangedExtern(int fileId);
    void fileDeletedExtern(int fileId);

public slots:
    void nodeChanged(int fileId);
    void updatePathNode(int fileId, QDir dir);
    void nodeClicked(QModelIndex index);

private slots:
    void onFileChangedExtern(int fileId);
    void onFileDeletedExtern(int fileId);
    void processExternFileEvents();

private:

private:
    int mNextId;
    FileGroupContext* mRoot;
    FileGroupContext* mTreeRoot;
    QStringList mSuffixFilter;
    QList<int> mChangedIds;
    QList<int> mDeletedIds;
    QList<FileActionContext*> mFileActions;
};

} // namespace studio
} // namespace gams

#endif // FILEREPOSITORY_H
