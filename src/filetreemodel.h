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
#ifndef FILETREEMODEL_H
#define FILETREEMODEL_H

#include <QtWidgets>
#include "filegroupcontext.h"

namespace gams {
namespace studio {

class FileRepository;

class FileTreeModel : public QAbstractItemModel
{
public:
    explicit FileTreeModel(FileRepository *parent, FileGroupContext* root);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &ind, int role = Qt::DisplayRole) const;

    QModelIndex index(FileSystemContext* entry) const;
    QModelIndex rootModelIndex() const;
    FileGroupContext* rootContext() const;

protected:
    friend class FileRepository;

    bool insertChild(int row, FileGroupContext* parent, FileSystemContext* child);
    bool removeChild(FileSystemContext* child);

    bool isCurrent(const QModelIndex& ind) const;
    void setCurrent(const QModelIndex& ind);
    bool isCurrentGroup(const QModelIndex& ind) const;

    QModelIndex selected() const;
    void setSelected(const QModelIndex& selected);

private:
    FileRepository *mFileRepo;
    FileGroupContext* mRoot = nullptr;
    QModelIndex mCurrent;
    QModelIndex mSelected;

};

} // namespace studio
} // namespace gams

#endif // FILETREEMODEL_H
