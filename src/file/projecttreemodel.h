/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "pexgroupnode.h"

namespace gams {
namespace studio {

class ProjectRepo;

class ProjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum ProjectRoles {
        LocationRole = Qt::UserRole,
        NodeIdRole = Qt::UserRole + 1,
        IsProjectRole = Qt::UserRole + 2,
        NameExtRole = Qt::UserRole + 3,
        IsGamsSys = Qt::UserRole + 4,
    };

public:
    explicit ProjectTreeModel(ProjectRepo *parent, PExRootNode *root);

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &ind, int role = Qt::DisplayRole) const override;

    QModelIndex index(const PExAbstractNode *entry) const;
    QModelIndex rootModelIndex() const;
    PExRootNode *rootNode() const;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    void setDebugMode(bool debug);
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex current() {return index(mCurrent);}
    QVector<NodeId> selectedIds() const;
    QMap<int, QVariant> itemData(const QModelIndex &index) const override;

signals:
    void childrenChanged();
    void projectListChanged();
    void parentAssigned(const gams::studio::PExAbstractNode *node);

protected:
    friend class ProjectRepo;
    friend class ProjectTreeView;

    bool insertChild(int row, PExGroupNode* parent, PExAbstractNode* child);
    bool removeChild(PExAbstractNode* child);
    NodeId nodeId(const QModelIndex &ind) const;
    QModelIndex index(const NodeId &id) const;

    /// Tells if a model index is the current node
    /// \param ind
    /// \return
    bool isCurrent(const QModelIndex& ind) const;
    void setCurrent(const QModelIndex& ind);
    bool isCurrentProject(const QModelIndex& ind) const;
    QModelIndex findProject(QModelIndex ind, bool *locked);
    void sortChildNodes(PExGroupNode *group);
    void updateProjectExtNums();

    bool isSelected(const QModelIndex& ind) const;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void deselectAll();
    const QVector<QModelIndex> popDeclined();
    const QVector<QModelIndex> popAddProjects();
    void update(const QModelIndex& ind = QModelIndex());

private:
    QVector<QModelIndex> gatherChildren(QModelIndex index);

private:
    ProjectRepo *mProjectRepo;
    PExRootNode* mRoot = nullptr;
    bool mDebug = false;
    NodeId mCurrent;
    QVector<NodeId> mSelected;
    QVector<QModelIndex> mDeclined;
    QVector<QModelIndex> mAddProjects;
};

} // namespace studio
} // namespace gams

#endif // PROJECTTREEMODEL_H
