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
#include "projecttreemodel.h"

#include "exception.h"
#include "projectrepo.h"
#include "pexfilenode.h"
#include "pexabstractnode.h"
#include "pexgroupnode.h"
#include "logger.h"

namespace gams {
namespace studio {

ProjectTreeModel::ProjectTreeModel(ProjectRepo* parent, PExRootNode* root)
    : QAbstractItemModel(parent), mProjectRepo(parent), mRoot(root)
{
    Q_ASSERT_X(mProjectRepo, "ProjectTreeModel constructor", "The FileTreeModel needs a valid FileRepository");
}

QModelIndex ProjectTreeModel::index(const PExAbstractNode *entry) const
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
    PExGroupNode *group = (mProjectRepo->node(parent) ? mProjectRepo->node(parent)->toGroup() : nullptr);
    if (!group) {
        DEB() << "error: retrieved invalid parent from this QModelIndex";
        return QModelIndex();
    }
    PExAbstractNode *node = group->childNode(row);
    if (!node) {
        DEB() << "invalid child for row " << row;
        return QModelIndex();
    }
    return createIndex(row, column, quintptr(node->id()));
}

QModelIndex ProjectTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) return QModelIndex();
    PExAbstractNode* eChild = mProjectRepo->node(child);
    if (!eChild || eChild == mRoot)
        return QModelIndex();
    PExGroupNode* group = eChild->parentNode();
    if (!group)
        return QModelIndex();
    if (group == mRoot)
        return createIndex(0, child.column(), quintptr(group->id()));
    PExGroupNode* parParent = group->parentNode();
    int row = parParent ? parParent->indexOf(group) : -1;
    if (row < 0)
        FATAL() << "could not find child in parent";
    return createIndex(row, child.column(), quintptr(group->id()));
}

int ProjectTreeModel::rowCount(const QModelIndex& parent) const
{
    PExAbstractNode* node = mProjectRepo->node(parent);
    if (!node) return 0;
    PExGroupNode* group = node->toGroup();
    if (!group) return 0;
    return group->childCount();
}

int ProjectTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ProjectTreeModel::data(const QModelIndex& ind, int role) const
{
    if (!ind.isValid()) return QVariant();
    switch (role) {
    case Qt::BackgroundRole: {
        if (isSelected(ind)) {
            if (Theme::isDark())
                return QColor(243,150,25); // GAMS orange
            else
                return QColor(102,187,255,68); // "#4466BBFF"
        }
    }   break;

    case Qt::DisplayRole:
        return mProjectRepo->node(ind)->name(NameModifier::editState);

    case Qt::EditRole:
        return mProjectRepo->node(ind)->name(NameModifier::raw);

    case Qt::FontRole: {
        if (isCurrent(ind) || isCurrentProject(ind)) {
            QFont f;
            f.setBold(true);
            return f;
        }
    }   break;

    case Qt::ForegroundRole: {
        PExFileNode *node = mProjectRepo->node(ind)->toFile();
        if (node && !node->file()->exists(true)) {
            return isCurrent(ind) ? Theme::color(Theme::Normal_Red).darker()
                                  : QColor(Qt::gray);
        } else if (Theme::instance()->isDark()) {
            if (isSelected(ind))
                return QColor(Qt::black);
            if (!isCurrent(ind) && !isCurrentProject(ind))
                return QColor(Qt::white).darker(125);
        }
    }   break;

    case Qt::DecorationRole: {
        QModelIndex parInd = ind;
        while (parInd.isValid() && parInd.parent().isValid() && parInd.parent().parent().isValid())
            parInd = parInd.parent();
        QIcon::Mode mode = isCurrent(ind) ? QIcon::Active : isSelected(ind) ? QIcon::Selected : QIcon::Normal;
        return mProjectRepo->node(ind)->icon(mode, isCurrentProject(parInd) ? 100 : 50);
    }

    case Qt::ToolTipRole:
        return mProjectRepo->node(ind)->tooltip();

    case LocationRole: {
        PExFileNode *node = mProjectRepo->node(ind)->toFile();
        if (node) return node->location();
    }   break;

    case NodeIdRole: {
        PExAbstractNode *node = mProjectRepo->node(ind);
        if (node) return int(node->id());
        break;
    }
    case IsProjectRole: {
        PExProjectNode *node = mProjectRepo->node(ind)->toProject();
        return bool(node && node->type() <= PExProjectNode::tCommon);
    }
    case NameExtRole: {
        PExProjectNode *node = mProjectRepo->node(ind)->toProject();
        if (!node) return QString();
        return node->nameExt();
    }
    case IsGamsSys: {
        PExProjectNode *node = mProjectRepo->node(ind)->assignedProject();
        return (node && node->type() == PExProjectNode::tGams);
    }
    default:
        break;
    }
    return QVariant();
}

QModelIndex ProjectTreeModel::rootModelIndex() const
{
    return createIndex(0, 0, quintptr(mRoot->id()));
}

PExRootNode* ProjectTreeModel::rootNode() const
{
    return mRoot;
}

bool ProjectTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)
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
        PExGroupNode *rn = rootNode();
        for (int i = 0; i < rn->childCount(); ++i) {
            PExAbstractNode *n = rn->childNode(i);
            proj << n->name();
            PExGroupNode *gn = n->toGroup();
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
    if (!index.isValid()) return false;
    PExAbstractNode *node = mProjectRepo->node(index);
    if (!node) return false;
    if (role == Qt::EditRole) {
        PExGroupNode *group = node->toGroup();
        if (!group) return false;
        group->setName(value.toString());
        emit dataChanged(index, index);
        sortChildNodes(group->parentNode());
    }
    return true;
}

Qt::ItemFlags ProjectTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid()) return flags;
    PExAbstractNode *node = mProjectRepo->node(index);
    if (!node) return flags;
    PExProjectNode *project = node->toProject();
    if (project) {
        flags.setFlag(Qt::ItemIsDropEnabled);
    } else flags.setFlag(Qt::ItemIsDragEnabled);
    return flags;
}

bool ProjectTreeModel::insertChild(int row, PExGroupNode* parent, PExAbstractNode* child)
{
    QModelIndex parMi = index(parent);
    if (!parMi.isValid()) return false;
    if (child->parentNode() == parent) return false;
    beginInsertRows(parMi, row, row);
    child->setParentNode(parent);
    endInsertRows();
    emit childrenChanged();
    emit parentAssigned(child);
    return true;
}

bool ProjectTreeModel::removeChild(PExAbstractNode* child)
{
    QModelIndex mi = index(child);
    if (!mi.isValid()) {
        DEB() << "FAILED removing NodeId " << child->id();
        return false;
    } else {
        if (mDebug) DEB() << "removing NodeId " << child->id();
    }
    PExGroupNode *parent = child->parentNode();
    QModelIndex parMi = index(parent);
    if (!parMi.isValid()) return false;
    beginRemoveRows(parMi, mi.row(), mi.row());
    child->setParentNode(nullptr);
    endRemoveRows();
    if (parent == mRoot)
        updateProjectExtNums();
    emit childrenChanged();
    return true;
}

NodeId ProjectTreeModel::nodeId(const QModelIndex &ind) const
{
    return ind.isValid() ? static_cast<NodeId>(int(ind.internalId())) : NodeId();
}

QModelIndex ProjectTreeModel::index(const NodeId &id) const
{
    if (!id.isValid())
        return QModelIndex();
    if (mRoot->id() == id)
        return createIndex(0, 0, quintptr(id));
    PExAbstractNode *node = mRoot->projectRepo()->node(id);
    if (!node) return QModelIndex();
    for (int i = 0; i < node->parentNode()->childCount(); ++i) {
        if (node->parentNode()->childNode(i) == node) {
            return createIndex(i, 0, quintptr(id));
        }
    }
    return QModelIndex();
}

bool lessThan(PExAbstractNode*n1, PExAbstractNode*n2)
{
    bool isGroup1 = n1->toGroup();
    if (isGroup1 != bool(n2->toGroup())) return isGroup1;
    int cmp = n1->name().compare(n2->name(), Qt::CaseInsensitive);
    if (cmp != 0) return cmp < 0;
    cmp = isGroup1 ? n1->toGroup()->location().compare(n2->toGroup()->location(), Qt::CaseInsensitive)
                   : n1->toFile()->location().compare(n2->toFile()->location(), Qt::CaseInsensitive);
    if (cmp != 0) return cmp < 0;
    cmp = isGroup1 ? n1->toGroup()->location().compare(n2->toGroup()->location())
                   : n1->toFile()->location().compare(n2->toFile()->location());
    return cmp < 0;
}

void ProjectTreeModel::sortChildNodes(PExGroupNode *group)
{
    QList<PExAbstractNode*> order = group->childNodes();
    std::sort(order.begin(), order.end(), lessThan);
    for (int i = 0; i < order.size(); ++i) {
        QModelIndex parMi = index(group);
        QModelIndex mi = index(order.at(i));
        if (mi.isValid() && mi.row() != i) {
            int row = mi.row();
            beginMoveRows(parMi, row, row, parMi, i);
            group->moveChildNode(row, i);
            endMoveRows();
        }
    }
    if (group == mRoot) {
        updateProjectExtNums();
    }
}

void ProjectTreeModel::updateProjectExtNums()
{
    for (int i = 0; i < mRoot->childCount(); ++i) {
        PExProjectNode *project = mRoot->childNode(i)->toProject();
        if (!project) continue;
        QString name = project->name();
        QString ext;
        int nr = 0;
        for (int j = 0; j < i; ++j) {
            PExProjectNode *other = mRoot->childNode(j)->toProject();
            if (other && other->name(NameModifier::withNameExt) == name + ext) {
                ++nr;
                ext = QString::number(nr);
            }
        }
        project->setNameExt(ext);
    }
    emit projectListChanged();
}

bool ProjectTreeModel::isCurrent(const QModelIndex& ind) const
{
    return (mCurrent.isValid() && nodeId(ind) == mCurrent);
}

void ProjectTreeModel::setCurrent(const QModelIndex& ind)
{
    if (!isCurrent(ind)) {
        QVector<QModelIndex> changeList;
        QModelIndex mi = index(mCurrent);
        while (mi.parent().parent().isValid())
            mi = mi.parent();
        changeList << mi << gatherChildren(mi);

        mCurrent = nodeId(ind);

        mi = ind;
        while (mi.parent().parent().isValid())
            mi = mi.parent();
        if (mi != changeList.at(0))
            changeList << mi << gatherChildren(mi);

        for (const QModelIndex &mi2 : std::as_const(changeList))
            emit dataChanged(mi2, mi2);
    }
}

bool ProjectTreeModel::isCurrentProject(const QModelIndex& ind) const
{
    if (mCurrent.isValid()) {
        PExAbstractNode* node = mProjectRepo->node(mCurrent);
        if (!node || !node->assignedProject()) return false;
        if (node->assignedProject()->id() == nodeId(ind)) {
            return true;
        }
    }
    return false;
}

QModelIndex ProjectTreeModel::findProject(QModelIndex ind, bool *locked)
{
    if (locked) *locked = false;
    if (ind.isValid()) {
        PExAbstractNode *node = mProjectRepo->node(ind);
        if (!node) return ind;
        PExProjectNode *project = node->assignedProject();
        if (project->type() > PExProjectNode::tCommon) {
            if (locked) *locked = true;
            return QModelIndex();
        }
        ind = index(project);
    }
    return ind;
}

bool ProjectTreeModel::isSelected(const QModelIndex& ind) const
{
    return ind.isValid() && mSelected.contains(nodeId(ind));
}

void ProjectTreeModel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    const auto dInds = deselected.indexes();
    for (const QModelIndex &ind: dInds) {
        NodeId id = nodeId(ind);
        if (id.isValid() && mSelected.contains(id)) {
            mSelected.removeAll(id);
            emit dataChanged(ind, ind);
        }
    }
    NodeId firstId = mSelected.isEmpty() ? selected.isEmpty() ? NodeId()
                                                              : nodeId(selected.indexes().first())
                                         : mSelected.first();
    PExAbstractNode *first = mProjectRepo->node(firstId);
    mAddProjects.clear();
    int selKind = !first ? 0 : first->toProject() ? 1 : 2;
    const auto sInds = selected.indexes();
    for (const QModelIndex &ind: sInds) {
        NodeId id = nodeId(ind);
        PExAbstractNode *node = mProjectRepo->node(id);
        int nodeKind = !node ? 0 : node->toProject() ? 1 : 2;
        if (nodeKind == 1 && selKind == 2) {
            mAddProjects << ind;
        }
        if (id.isValid() && !mSelected.contains(id) && (!selKind || nodeKind == selKind)) {
            mSelected << id;
            emit dataChanged(ind, ind);
        } else {
            mDeclined << ind;
        }
    }
}

void ProjectTreeModel::deselectAll()
{
    mSelected.clear();
    emit dataChanged(rootModelIndex(), index(rowCount(rootModelIndex())));
}

QVector<NodeId> ProjectTreeModel::selectedIds() const
{
    return mSelected;
}

QMap<int, QVariant> ProjectTreeModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> res = QAbstractItemModel::itemData(index);
    res.insert(LocationRole, data(index, LocationRole));
    res.insert(NodeIdRole, data(index, NodeIdRole));
    return res;
}

const QVector<QModelIndex> ProjectTreeModel::popDeclined()
{
    QVector<QModelIndex> res = mDeclined;
    mDeclined.clear();
    return res;
}

const QVector<QModelIndex> ProjectTreeModel::popAddProjects()
{
    QVector<QModelIndex> res = mAddProjects;
    mAddProjects.clear();
    return res;
}

void ProjectTreeModel::update(const QModelIndex &ind)
{
    if (ind.isValid()) emit dataChanged(ind, ind);
}

QVector<QModelIndex> ProjectTreeModel::gatherChildren(QModelIndex ind)
{
    QVector<QModelIndex> res;
    for (int row = 0; row < rowCount(ind); ++row) {
        QModelIndex i = index(row, 0, ind);
        res << i << gatherChildren(i);
    }
    return res;
}


} // namespace studio
} // namespace gams
