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
#include "projecttreemodel.h"

#include "exception.h"
#include "projectrepo.h"
#include "projectfilenode.h"
#include "projectabstractnode.h"
#include "projectgroupnode.h"
#include "logger.h"

namespace gams {
namespace studio {

ProjectTreeModel::ProjectTreeModel(ProjectRepo* parent, ProjectGroupNode* root)
    : QAbstractItemModel(parent), mProjectRepo(parent), mRoot(root)
{
    if (!mProjectRepo)
        FATAL() << "nullptr not allowed. The FileTreeModel needs a valid FileRepository.";
}

QModelIndex ProjectTreeModel::index(const ProjectAbstractNode *entry) const
{
    if (!entry)
        return QModelIndex();
    if (!entry->parentNode())
        return createIndex(0, 0, quintptr(entry->id()));
    for (int i = 0; i < entry->parentNode()->childCount(); ++i) {
        if (entry->parentNode()->childNode(i) == entry) {
            return createIndex(i, 0, quintptr(entry->id()));
        }
    }
    return QModelIndex();
}

QModelIndex ProjectTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    ProjectGroupNode *group = (mProjectRepo->node(parent) ? mProjectRepo->node(parent)->toGroup() : nullptr);
    if (!group) {
        DEB() << "error: retrieved invalid parent from this QModelIndex";
        return QModelIndex();
    }
    ProjectAbstractNode *node = group->childNode(row);
    if (!node) {
        DEB() << "invalid child for row " << row;
        return QModelIndex();
    }
    return createIndex(row, column, quintptr(node->id()));
}

QModelIndex ProjectTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) return QModelIndex();
    ProjectAbstractNode* eChild = mProjectRepo->node(child);
    if (!eChild || eChild == mRoot)
        return QModelIndex();
    ProjectGroupNode* group = eChild->parentNode();
    if (!group)
        return QModelIndex();
    if (group == mRoot)
        return createIndex(0, child.column(), quintptr(group->id()));
    ProjectGroupNode* parParent = group->parentNode();
    int row = parParent ? parParent->indexOf(group) : -1;
    if (row < 0)
        FATAL() << "could not find child in parent";
    return createIndex(row, child.column(), quintptr(group->id()));
}

int ProjectTreeModel::rowCount(const QModelIndex& parent) const
{
    ProjectAbstractNode* node = mProjectRepo->node(parent);
    if (!node) return 0;
    ProjectGroupNode* group = node->toGroup();
    if (!group) return 0;
    return group->childCount();
}

int ProjectTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ProjectTreeModel::data(const QModelIndex& ind, int role) const
{
    if (!ind.isValid()) return QVariant();
    switch (role) {
    case Qt::BackgroundRole:
        if (isSelected(ind)) return QColor("#4466BBFF");
        break;

    case Qt::DisplayRole:
        return mProjectRepo->node(ind)->name(NameModifier::editState);

    case Qt::EditRole:
        return mProjectRepo->node(ind)->name(NameModifier::raw);

    case Qt::FontRole:
        if (isCurrent(ind) || isCurrentGroup(ind)) {
            QFont f;
            f.setBold(true);
            return f;
        }
        break;

    case Qt::ForegroundRole: {
        ProjectFileNode *node = mProjectRepo->node(ind)->toFile();
        if (node && !node->file()->exists(true))
            return isCurrent(ind) ? QColor(Qt::red).darker()
                                  : QColor(Qt::gray);
        if (mProjectRepo->node(ind)->isActive()) {
            return (isCurrent(ind)) ? QColor(Qt::blue)
                                    : QColor(Qt::black);
        }
        break;
    }

    case Qt::DecorationRole:
        return mProjectRepo->node(ind)->icon();

    case Qt::ToolTipRole:
        return mProjectRepo->node(ind)->tooltip();

    default:
        break;
    }
    return QVariant();
}

QModelIndex ProjectTreeModel::rootModelIndex() const
{
    return createIndex(0, 0, quintptr(mRoot->id()));
}

ProjectGroupNode* ProjectTreeModel::rootNode() const
{
    return mRoot;
}

bool ProjectTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(parent);
    DEB() << "FileTreeModel::removeRows is unsupported, please use FileTreeModel::removeChild";
    return false;
}

void ProjectTreeModel::setDebugMode(bool debug)
{
    mDebug = debug;
    if (debug) {
        QStringList tree;
        tree << "------ TREE ------";
        int maxLen = tree.last().length();
        QModelIndex root = rootModelIndex();
        for (int i = 0; i < rowCount(root); ++i) {
            QModelIndex groupChild = index(i,0,root);
            tree << mProjectRepo->node(groupChild)->name();
            maxLen = qMax(maxLen, tree.last().length());
            for (int j = 0; j < rowCount(groupChild); ++j) {
                QModelIndex child = index(j,0,groupChild);
                tree << "   "+mProjectRepo->node(child)->name();
                maxLen = qMax(maxLen, tree.last().length());
            }
        }
        QStringList proj;
        proj << "------ PROJ ------";
        ProjectGroupNode *rn = rootNode();
        for (int i = 0; i < rn->childCount(); ++i) {
            ProjectAbstractNode *n = rn->childNode(i);
            proj << n->name();
            ProjectGroupNode *gn = n->toGroup();
            for (int j = 0; j < (gn ? gn->childCount() : 0); ++j) {
                proj << "   "+gn->childNode(j)->name();
            }
        }
        int lines = qMax(tree.length(), proj.length());
        for (int i = 0; i < lines; ++i) {
            QString str(maxLen, ' ');
            QString sTree = (tree.length() > i) ? tree.at(i) : "";
            QString sProj = (proj.length() > i) ? proj.at(i) : "";
            str = (sTree + str).left(maxLen) + "  "+((sTree==sProj || !i) ? " " : "!")+"  "+sProj;
            DEB() << str;
        }
    }
}

bool ProjectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role);
    if (!index.isValid()) return false;
    ProjectAbstractNode *node = mProjectRepo->node(index);
    if (!node) return false;
    ProjectGroupNode *group = node->toGroup();
    if (!group) return false;
    group->setName(value.toString());
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags ProjectTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid()) return flags;
    ProjectAbstractNode *node = mProjectRepo->node(index);
    if (!node) return flags;
    ProjectGroupNode *group = node->toGroup();
    if (group) flags.setFlag(Qt::ItemIsEditable);
    return flags;
}

bool ProjectTreeModel::insertChild(int row, ProjectGroupNode* parent, ProjectAbstractNode* child)
{
    QModelIndex parMi = index(parent);
    if (!parMi.isValid()) return false;
    beginInsertRows(parMi, row, row);
    child->setParentNode(parent);
//    updateIndex(parMi, row, 1);
    endInsertRows();
    return true;
}

bool ProjectTreeModel::removeChild(ProjectAbstractNode* child)
{
    QModelIndex mi = index(child);
    if (!mi.isValid()) {
        DEB() << "FAILED removing NodeId " << child->id();
        return false;
    } else {
        if (mDebug) DEB() << "removing NodeId " << child->id();
    }
    QModelIndex parMi = index(child->parentNode());
    beginRemoveRows(parMi, mi.row(), mi.row());
    child->setParentNode(nullptr);
//    updateIndex(parMi, mi.row(), -1);
    endRemoveRows();
    return true;
}

NodeId ProjectTreeModel::nodeId(const QModelIndex &ind) const
{
    return ind.isValid() ? static_cast<NodeId>(int(ind.internalId())) : NodeId();
}

QModelIndex ProjectTreeModel::index(const NodeId id) const
{
    if (!id.isValid())
        return QModelIndex();
    if (mRoot->id() == id)
        return createIndex(0, 0, quintptr(id));
    ProjectAbstractNode *node = mRoot->projectRepo()->node(id);
    if (!node) return QModelIndex();
    for (int i = 0; i < node->parentNode()->childCount(); ++i) {
        if (node->parentNode()->childNode(i) == node) {
            return createIndex(i, 0, quintptr(id));
        }
    }
    return QModelIndex();
}

//void ProjectTreeModel::updateIndex(const QModelIndex &parent, int row, int change)
//{
//    if (mCurrent.isValid() && mCurrent.parent() == parent && mCurrent.row() >= row) {
//        if (change < 0 && mCurrent.row() < row-change)
//            mCurrent = QModelIndex();
//        else
//            mCurrent = index(mCurrent.row()+change, mCurrent.column(), parent);
//    }
//}

bool ProjectTreeModel::isCurrent(const QModelIndex& ind) const
{
    return (mCurrent.isValid() && nodeId(ind) == mCurrent);
}

void ProjectTreeModel::setCurrent(const QModelIndex& ind)
{
    if (!isCurrent(ind)) {
        QModelIndex mi = index(mCurrent);
        mCurrent = nodeId(ind);
        while (mi.isValid()) { // invalidate old
            dataChanged(mi, mi);
            mi = mProjectRepo->node(mi) ? index(mProjectRepo->node(mi)->parentNode()) : QModelIndex();
        }
        mi = ind;
        while (mi.isValid()) { // invalidate new
            dataChanged(mi, mi);
            mi = mProjectRepo->node(mi) ? index(mProjectRepo->node(mi)->parentNode()) : QModelIndex();
        }
    }
}

bool ProjectTreeModel::isCurrentGroup(const QModelIndex& ind) const
{
    if (mCurrent.isValid()) {
        ProjectAbstractNode* node = mProjectRepo->node(mCurrent);
        if (!node) return false;
        if (node->parentNode()->id() == nodeId(ind)) {
            return true;
        }
    }
    return false;
}

bool ProjectTreeModel::isSelected(const QModelIndex& ind) const
{
    return ind.isValid() && mSelected.contains(nodeId(ind));
}

void ProjectTreeModel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    for (const QModelIndex &ind: deselected.indexes()) {
        NodeId id = nodeId(ind);
        if (id.isValid() && mSelected.contains(id)) {
            mSelected.removeAll(id);
            dataChanged(ind, ind);
        }
    }
    for (const QModelIndex &ind: selected.indexes()) {
        NodeId id = nodeId(ind);
        if (id.isValid() && !mSelected.contains(id)) {
            mSelected << id;
            dataChanged(ind, ind);
        }
    }
}

void ProjectTreeModel::update(const QModelIndex &ind)
{
    if (ind.isValid()) dataChanged(ind, ind);
}

} // namespace studio
} // namespace gams
