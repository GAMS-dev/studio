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
#ifndef PROJECTPROXYMODEL_H
#define PROJECTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "common.h"

namespace gams {
namespace studio {

class ProjectTreeModel;
class PExAbstractNode;
class PExProjectNode;

class ProjectProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ProjectProxyModel(QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *sourceModel);
    QModelIndex rootModelIndex() const;
    QModelIndex current();
    QModelIndex findProject(QModelIndex ind, bool *locked);
    QList<NodeId> selectedIds() const;
    QModelIndex asIndex(const PExAbstractNode *entry) const;
    QModelIndex asIndex(const NodeId &id) const;
    void focusProject(PExProjectNode* project);
    PExProjectNode *focussedProject() const;

signals:

protected:
    friend class ProjectRepo;

    NodeId nodeId(const QModelIndex &ind) const;
    PExAbstractNode* node(const QModelIndex& index) const;
    bool isSelected(const QModelIndex& ind) const;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void setCurrent(const QModelIndex& ind);
    void deselectAll();
    const QList<QModelIndex> popDeclined();
    const QList<QModelIndex> popAddProjects();
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    ProjectTreeModel* mSourceModel = nullptr;
    PExProjectNode* mFocusProject = nullptr;
};

} // namespace studio
} // namespace gams

#endif // PROJECTPROXYMODEL_H
