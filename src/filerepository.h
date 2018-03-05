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
#ifndef FILEREPOSITORY_H
#define FILEREPOSITORY_H

#include <QtWidgets>
#include "filetreemodel.h"
#include "fileactioncontext.h"
#include "logcontext.h"
#include "filesystemcontext.h"
#include "filecontext.h"
#include "filegroupcontext.h"
#include "filetype.h"

namespace gams {
namespace studio {

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

    /// \brief Get the <c>FileSystemContext</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>FileSystemContext</c>.
    /// \return The associated <c>FileSystemContext</c>.
    FileSystemContext* context(const QModelIndex& index) const;

    /// \brief Get the <c>FileContext</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>FileContext</c>.
    /// \return The associated <c>FileContext</c>, otherwise <c>nullptr</c>.
    FileContext* fileContext(const QModelIndex& index) const;

    /// \brief Get the <c>FileContext</c> related to a <c>QPlainTextEdit</c>.
    /// \param edit The <c>QPlainTextEdit</c> assigned to the <c>FileContext</c>.
    /// \return The associated <c>FileContext</c>, otherwise <c>nullptr</c>.
    FileContext* fileContext(QWidget* edit);

    /// \brief Get the <c>FileGroupContext</c> related to a <c>QModelIndex</c>.
    /// \param index The QModelIndex pointing to the <c>FileGroupContext</c>.
    /// \return The associated <c>FileGroupContext</c>, otherwise <c>nullptr</c>.
    FileGroupContext* groupContext(const QModelIndex& index) const;

    QWidgetList editors(FileId fileId = -1);

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

    FileGroupContext* ensureGroup(const QString& filePath);
    void close(FileId fileId);
    void setSuffixFilter(QStringList filter);
    void dump(FileSystemContext* fc, int lv = 0);
    QModelIndex findEntry(QString name, QString location, QModelIndex parentIndex);
    FileSystemContext* findContext(QString filePath, FileGroupContext* fileGroup = nullptr);
    QList<FileContext*> modifiedFiles(FileGroupContext* fileGroup = nullptr);
    int saveAll();
    void editorActivated(QWidget* edit);
    FileTreeModel* treeModel() const;
    LogContext* logContext(QWidget* edit);
    LogContext* logContext(FileSystemContext* node);
    void removeMarks(FileGroupContext* group);

    void updateLinkDisplay(QPlainTextEdit* editUnderCursor);
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    inline FileSystemContext* context(FileId id) const {
        return mContext.value(id, nullptr);
    }
    inline FileGroupContext* groupContext(FileId id) const {
        FileSystemContext* res = mContext.value(id, nullptr);
        return (res && res->type() == FileSystemContext::FileGroup) ? static_cast<FileGroupContext*>(res) : nullptr;
    }
    inline FileContext* fileContext(FileId id) const {
        FileSystemContext* res = mContext.value(id, nullptr);
        return (res && res->type() == FileSystemContext::File) ? static_cast<FileContext*>(res) : nullptr;
    }
    inline LogContext* logContext(FileId id) const {
        FileSystemContext* res = mContext.value(id, nullptr);
        return (res && res->type() == FileSystemContext::Log) ? static_cast<LogContext*>(res) : nullptr;
    }

signals:
    void fileClosed(FileId fileId, QPrivateSignal);
    void fileChangedExtern(FileId fileId);
    void fileDeletedExtern(FileId fileId);
    void openFileContext(FileContext* fileContext, bool focus = true);
    void gamsProcessStateChanged(FileGroupContext* group);
    void setNodeExpanded(const QModelIndex &mi, bool expanded = true);
    void getNodeExpanded(const QModelIndex &mi, bool *expanded);

public slots:
    void nodeChanged(FileId fileId);
    void findFile(QString filePath, FileContext** resultFile, FileGroupContext* fileGroup = nullptr);
    void findOrCreateFileContext(QString filePath, FileContext** resultFile, FileGroupContext* fileGroup = nullptr);
    void setSelected(const QModelIndex& ind);
    void removeGroup(FileGroupContext* fileGroup);
    void removeFile(FileContext* file);

private slots:
    void onFileChangedExtern(FileId fileId);
    void onFileDeletedExtern(FileId fileId);
    void processExternFileEvents();
    void addNode(QString name, QString location, FileGroupContext* parent = nullptr);
    void removeNode(FileSystemContext *node);

private:
    void writeGroup(const FileGroupContext* group, QJsonArray &jsonArray) const;
    void readGroup(FileGroupContext* group, const QJsonArray &jsonArray);
    inline void storeContext(FileSystemContext* context) {
        mContext.insert(context->id(), context);
    }
    inline void deleteContext(FileSystemContext* context) {
        context->setParentEntry(nullptr);
        mContext.remove(context->id());
        delete context;
    }

private:
    FileId mNextId;
    FileTreeModel* mTreeModel = nullptr;
    QStringList mSuffixFilter;
    QList<FileId> mChangedIds;
    QList<FileId> mDeletedIds;
    QHash<FileId, FileSystemContext*> mContext;
};

} // namespace studio
} // namespace gams

#endif // FILEREPOSITORY_H
