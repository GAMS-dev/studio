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
#include "projectproxymodel.h"
#include "projecttreemodel.h"
#include "projectrepo.h"
#include "exception.h"

namespace gams {
namespace studio {

ProjectProxyModel::ProjectProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{}

void ProjectProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
    mSourceModel = qobject_cast<ProjectTreeModel*>(sourceModel);
    if (!mSourceModel) FATAL() << "Incompatible source model, ProjectTreeModel required.";
}

QModelIndex ProjectProxyModel::current()
{
    return mapFromSource(mSourceModel->current());
}

QModelIndex ProjectProxyModel::findProject(QModelIndex ind, bool *locked)
{
    return mapFromSource(mSourceModel->findProject(mapToSource(ind), locked));
}

QList<NodeId> ProjectProxyModel::selectedIds() const
{
    return mSourceModel->selectedIds();
}

QModelIndex ProjectProxyModel::asIndex(const PExAbstractNode *entry) const
{
    return mapFromSource(mSourceModel->asIndex(entry));
}

QModelIndex ProjectProxyModel::asIndex(const NodeId &id) const
{
    return mapFromSource(mSourceModel->asIndex(id));
}

void ProjectProxyModel::focusProject(PExProjectNode *project)
{
    beginFilterChange();
    mFocusProject = project;
    endFilterChange(Direction::Rows);
}

PExProjectNode *ProjectProxyModel::focussedProject() const
{
    return mFocusProject;
}

NodeId ProjectProxyModel::nodeId(const QModelIndex &ind) const
{
    QModelIndex index = mapToSource(ind);
    if (!index.isValid() || !index.internalPointer()) return NodeId();
    return static_cast<PExAbstractNode*>(index.internalPointer())->id();
}

PExAbstractNode *ProjectProxyModel::node(const QModelIndex &index) const
{
    return mSourceModel->node(mapToSource(index));
}

bool ProjectProxyModel::isSelected(const QModelIndex &ind) const
{
    return mSourceModel->isSelected(mapToSource(ind));
}

void ProjectProxyModel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    const auto dInds = deselected.indexes();
    for (const QModelIndex &ind: dInds) {
        NodeId id = nodeId(ind);
        if (id.isValid() && mSourceModel->mSelected.contains(id)) {
            mSourceModel->mSelected.removeAll(id);
            emit dataChanged(ind, ind);
        }
    }
    NodeId firstId = mSourceModel->mSelected.isEmpty() ? selected.isEmpty() ? NodeId()
                                                                            : nodeId(selected.indexes().first())
                                                       : mSourceModel->mSelected.first();
    PExAbstractNode *first = mSourceModel->projectRepo()->node(firstId);
    mSourceModel->mAddProjects.clear();
    int selKind = !first ? 0 : first->toProject() ? 1 : 2;
    const auto selInds = selected.indexes();
    for (const QModelIndex &ind: selInds) {
        NodeId id = nodeId(ind);
        QModelIndex srcInd = mapToSource(ind);
        PExAbstractNode *node = mSourceModel->projectRepo()->node(id);
        int nodeKind = !node ? 0 : node->toProject() ? 1 : 2;
        if (nodeKind == 1 && selKind == 2) {
            mSourceModel->mAddProjects << srcInd;
        }
        if (id.isValid() && !mSourceModel->mSelected.contains(id) && (!selKind || nodeKind == selKind)) {
            mSourceModel->mSelected << id;
            emit dataChanged(ind, ind);
        } else {
            mSourceModel->mDeclined << srcInd;
        }
    }
}

void ProjectProxyModel::setCurrent(const QModelIndex &ind)
{
    mSourceModel->setCurrent(mapToSource(ind));
}

void ProjectProxyModel::deselectAll()
{
    mSourceModel->deselectAll();
}

const QList<QModelIndex> ProjectProxyModel::popDeclined()
{
    QList<QModelIndex> res;
    for (QModelIndex sMi : mSourceModel->mDeclined) {
        QModelIndex pMi = mapFromSource(sMi);
        if (pMi.isValid())
            res << pMi;
    }
    mSourceModel->mDeclined.clear();
    return res;
}

const QList<QModelIndex> ProjectProxyModel::popAddProjects()
{
    QList<QModelIndex> res;
    for (QModelIndex sMi : mSourceModel->mAddProjects) {
        QModelIndex pMi = mapFromSource(sMi);
        if (pMi.isValid())
            res << pMi;
    }
    mSourceModel->mAddProjects.clear();
    return res;
}

bool ProjectProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!mFocusProject)
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

    QModelIndex nodeIndex = sourceModel()->index(source_row, 0, source_parent);
    PExAbstractNode *node = mSourceModel->node(nodeIndex);
    if (!node) {
        DEB() << "No node assigned to this model index";
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
    PExProjectNode *project = node->assignedProject();
    if (!project) {
        DEB() << "No project assigned to node " << node->name(NameModifier::withNameExt);
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
    if (node == mFocusProject || project == mFocusProject || project->type() > PExProjectNode::tCommon)
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

    return false;
}

} // namespace studio
} // namespace gams
