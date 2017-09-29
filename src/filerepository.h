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
#include "filegroupcontext.h"
#include "filecontext.h"

namespace gams {
namespace studio {

class FileRepository : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit FileRepository(QObject *parent = nullptr);
    ~FileRepository();

    FileSystemContext* context(int fileId, FileSystemContext* startNode = nullptr);
    FileContext* fileContext(int fileId, FileSystemContext* startNode = nullptr);
    FileGroupContext* groupContext(int fileId, FileSystemContext* startNode = nullptr);
    QModelIndex index(FileSystemContext* entry);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QModelIndex addGroup(QString name, QString location, QModelIndex parentIndex = QModelIndex());
    QModelIndex addFile(QString name, QString location, QModelIndex parentIndex = QModelIndex());
    void removeNode(FileSystemContext *node);
    QModelIndex rootTreeModelIndex();
    QModelIndex rootModelIndex();
    QModelIndex ensureGroup(const QString& filePath);
    void close(int fileId);
    void setSuffixFilter(QStringList filter);
    void dump(FileSystemContext* fc, int lv = 0);

    FileSystemContext* node(const QModelIndex& index) const;
    FileSystemContext* file(const QModelIndex& index) const;
    FileGroupContext* group(const QModelIndex& index) const;

signals:
    void fileClosed(int fileId);

public slots:
    void nodeChanged(int fileId);
    void updatePathNode(int fileId, QDir dir);
    void nodeExpanded(const QModelIndex& index);

private:
    QModelIndex findEntry(QString name, QString location, QModelIndex parentIndex);

private:
    int mNextId;
    FileGroupContext* mRoot;
    FileGroupContext* mTreeRoot;
    QStringList mSuffixFilter;
};

} // namespace studio
} // namespace gams

#endif // FILEREPOSITORY_H
