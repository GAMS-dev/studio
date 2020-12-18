/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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

class ProjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
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
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex current() {return index(mCurrent);}
    QVector<NodeId> selectedIds() const;
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    void sortChildNodes(ProjectGroupNode *group);

signals:
    void childrenChanged();
    void parentAssigned(const ProjectAbstractNode *node);

protected:
    friend class ProjectRepo;
    friend class ProjectTreeView;

    bool insertChild(int row, ProjectGroupNode* parent, ProjectAbstractNode* child);
    bool removeChild(ProjectAbstractNode* child);
    NodeId nodeId(const QModelIndex &ind) const;
    QModelIndex index(const NodeId id) const;

    /// Tells if a model index is the current node
    /// \param ind
    /// \return
    bool isCurrent(const QModelIndex& ind) const;
    void setCurrent(const QModelIndex& ind);
    bool isCurrentGroup(const QModelIndex& ind) const;
    QModelIndex findGroup(QModelIndex ind);

    bool isSelected(const QModelIndex& ind) const;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void deselectAll();
    const QVector<QModelIndex> popDeclined();
    const QVector<QModelIndex> popAddGroups();

    void update(const QModelIndex& ind = QModelIndex());

private:
    ProjectRepo *mProjectRepo;
    ProjectGroupNode* mRoot = nullptr;
    bool mDebug = false;
    NodeId mCurrent;
    QVector<NodeId> mSelected;
    QVector<QModelIndex> mDeclined;
    QVector<QModelIndex> mAddGroups;
};

} // namespace studio
} // namespace gams

#endif // PROJECTTREEMODEL_H
