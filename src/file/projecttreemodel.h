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
#ifndef PROJECTTREEMODEL_H
#define PROJECTTREEMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "projectgroupnode.h"

namespace gams {
namespace studio {

class ProjectRepo;

// TODO(JM) Inherit QSortFilterProxyModel to hide log-node and allow different sort-orders

class ProjectTreeModel : public QAbstractItemModel
{
public:
    explicit ProjectTreeModel(ProjectRepo *parent, ProjectGroupNode* root);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &ind, int role = Qt::DisplayRole) const;

    QModelIndex index(const ProjectAbstractNode *entry) const;
    QModelIndex rootModelIndex() const;
    ProjectGroupNode* rootNode() const;
    bool removeRows(int row, int count, const QModelIndex &parent);
    void setDebugMode(bool debug);

protected:
    friend class ProjectRepo;

    bool insertChild(int row, ProjectGroupNode* parent, ProjectAbstractNode* child);
    bool removeChild(ProjectAbstractNode* child);

    bool isCurrent(const QModelIndex& ind) const;
    void setCurrent(const QModelIndex& ind);
    bool isCurrentGroup(const QModelIndex& ind) const;

    bool isSelected(const QModelIndex& ind) const;
    void setSelected(const QModelIndex& ind);

    void update(const QModelIndex& ind = QModelIndex());
private:
    ProjectRepo *mProjectRepo;
    ProjectGroupNode* mRoot = nullptr;
    bool mDebug = false;
    QModelIndex mCurrent;
    QModelIndex mSelected;

};

} // namespace studio
} // namespace gams

#endif // PROJECTTREEMODEL_H
