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

QModelIndex ProjectTreeModel::asIndex(const PExAbstractNode *entry) const
{
    if (!entry)
        return QModelIndex();
    if (!entry->parentNode())
        return createIndex(0, 0, entry);
    for (int i = 0; i < entry->parentNode()->childCount(); ++i) {
        if (entry->parentNode()->childNode(i) == entry) {
            return createIndex(i, 0, entry);
        }
    }
    return QModelIndex();
}

QModelIndex ProjectTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    if (!parent.internalPointer())
        return (row == 0 && column == 0) ? createIndex(0, 0, mRoot) : QModelIndex();

    PExAbstractNode *parNode = static_cast<PExAbstractNode*>(parent.internalPointer());
    PExGroupNode *group = parNode->toGroup();
    if (!group)
        return QModelIndex();
    PExAbstractNode *node = group->childNode(row);
    if (!node) return QModelIndex();
    return createIndex(row, column, node);
}

QModelIndex ProjectTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid() || !child.internalPointer()) return QModelIndex();
    PExAbstractNode* eChild = static_cast<PExAbstractNode*>(child.internalPointer());
    if (!eChild || !child.isValid() || !child.internalPointer()) return QModelIndex();
    if (eChild == mRoot)
        return createIndex(0, 0, nullptr);
    PExGroupNode* group = eChild->parentNode();
    if (!group)
        return QModelIndex();
    if (group == mRoot)
        return createIndex(0, child.column(), group);
    PExGroupNode* parParent = group->parentNode();
    int row = parParent ? parParent->indexOf(group) : -1;
    if (row < 0)
        FATAL() << "could not find child in parent";
    return createIndex(row, child.column(), group);
}

int ProjectTreeModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) return 0;
    PExAbstractNode* aNode = node(parent);
    if (!aNode) return 1; // parent of root
    PExGroupNode* group = aNode->toGroup();
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
        } else {
            PExProjectNode *project = node(ind)->toProject();
            bool isRunningProject = (project && project->process() && project->process()->state() != QProcess::NotRunning);
            if (isRunningProject) {
                QColor back = Theme::color(Theme::Normal_Green);
                back.setAlphaF(.3F);
                return back;
            }
        }
    }   break;

    case Qt::DisplayRole:
        return node(ind)->name(NameModifier::editState);

    case Qt::EditRole:
        return node(ind)->name(NameModifier::raw);

    case Qt::FontRole: {
        PExProjectNode *project = node(ind)->toProject();
        bool isRunningProject = (project && project->process() && project->process()->state() != QProcess::NotRunning);
        if (isCurrent(ind) || isCurrentProject(ind) || isRunningProject) {
            QFont f;
            f.setBold(true);
            return f;
        }
    }   break;

    case Qt::ForegroundRole: {
        PExFileNode *aNode = node(ind)->toFile();
        if (aNode && !aNode->file()->exists(true)) {
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
        PExProjectNode *project = node(ind)->assignedProject();
        bool isCurProject = false;
        if (mCurrent.isValid() && project) {
            PExAbstractNode *curNode = mProjectRepo->node(mCurrent);
            if (curNode) {
                PExProjectNode *curPro = curNode->assignedProject();
                if (curPro && project->id() == curPro->id()) isCurProject = true;
            }
        }

        QIcon::Mode mode = isCurrent(ind) ? QIcon::Active : isSelected(ind) ? QIcon::Selected : QIcon::Normal;
        return node(ind)->icon(mode, isCurProject ? 100 : 50);
    }

    case Qt::ToolTipRole:
        return node(ind)->tooltip();

    case LocationRole: {
        PExFileNode *aNode = node(ind)->toFile();
        if (aNode) return aNode->location();
    }   break;

    case NodeIdRole: {
        PExAbstractNode *aNode = node(ind);
        if (aNode) return int(aNode->id());
        break;
    }
    case IsProjectRole: {
        PExProjectNode *aNode = node(ind)->toProject();
        return bool(aNode && aNode->type() <= PExProjectNode::tCommon);
    }
    case NameExtRole: {
        PExAbstractNode *aNode = node(ind);
        if (!aNode)
            return QString();
        return aNode->nameExt();
    }
    case IsGamsSys: {
        PExProjectNode *aNode = node(ind)->assignedProject();
        return (aNode && aNode->type() == PExProjectNode::tGams);
    }
    default:
        break;
    }
    return QVariant();
}

QModelIndex ProjectTreeModel::rootModelIndex() const
{
    return createIndex(0, 0, mRoot);
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
            tree << node(groupChild)->name();
            maxLen = qMax(maxLen, tree.last().length());
            for (int j = 0; j < rowCount(groupChild); ++j) {
                QModelIndex child = index(j,0,groupChild);
                tree << "   "+node(child)->name();
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
    PExAbstractNode *aNode = node(index);
    if (!aNode) return false;
    if (role == Qt::EditRole) {
        PExGroupNode *group = aNode->toGroup();
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
    PExAbstractNode *aNode = node(index);
    if (!aNode) return flags;
    PExProjectNode *project = aNode->toProject();
    if (project) {
        flags.setFlag(Qt::ItemIsDropEnabled);
        if (project->type() < PExProjectNode::tCommon)
            flags.setFlag(Qt::ItemIsEditable);
    } else flags.setFlag(Qt::ItemIsDragEnabled);
    return flags;
}

bool ProjectTreeModel::insertChild(int row, PExGroupNode* parent, PExAbstractNode* child)
{
    QModelIndex parMi = asIndex(parent);
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
    QModelIndex mi = asIndex(child);
    if (!mi.isValid()) {
        DEB() << "FAILED removing NodeId " << child->id();
        return false;
    } else {
        if (mDebug) DEB() << "removing NodeId " << child->id();
    }
    PExGroupNode *parent = child->parentNode();
    QModelIndex parMi = asIndex(parent);
    if (!parMi.isValid()) return false;
    beginRemoveRows(parMi, mi.row(), mi.row());
    child->setParentNode(nullptr);
    endRemoveRows();
    PExProjectNode *project = parent->toProject();
    if (parent == mRoot || (project && project->type() == PExProjectNode::tGams))
        updateProjectExtNums(parent);
    emit childrenChanged();
    return true;
}

NodeId ProjectTreeModel::nodeId(const QModelIndex &ind) const
{
    if (!ind.isValid()) return NodeId();
    return ind.internalPointer() ? static_cast<PExAbstractNode*>(ind.internalPointer())->id() : NodeId();
}

PExAbstractNode *ProjectTreeModel::node(const QModelIndex &index) const
{
    if (!index.isValid()) return nullptr;
    return index.internalPointer() ? static_cast<PExAbstractNode*>(index.internalPointer()) : nullptr;
}

QModelIndex ProjectTreeModel::asIndex(const NodeId &id) const
{
    if (!id.isValid())
        return QModelIndex();
    if (mRoot->id() == id)
        return createIndex(0, 0, mRoot);
    PExAbstractNode *node = mRoot->projectRepo()->node(id);
    if (!node)
        return QModelIndex();
    for (int i = 0; i < node->parentNode()->childCount(); ++i) {
        if (node->parentNode()->childNode(i) == node) {
            return createIndex(i, 0, node);
        }
    }
    return QModelIndex();
}

QStringList internConfigPaths;

bool isBefore(PExAbstractNode*n1, PExAbstractNode*n2)
{
    int cmp = n1->name().compare(n2->name(), Qt::CaseInsensitive);
    if (cmp != 0) return cmp < 0;
    QFileInfo fi1(n1->toFile()->location());
    QFileInfo fi2(n2->toFile()->location());
    for (int i = 0; i < internConfigPaths.size(); ++i) {
        if (fi1.path().compare(internConfigPaths.at(i), FileType::fsCaseSense()) == 0)
            return true;
        if (fi2.path().compare(internConfigPaths.at(i), FileType::fsCaseSense()) == 0)
            return false;
    }
    return true;
}

void ProjectTreeModel::sortGamsProject(PExProjectNode *project, QList<PExAbstractNode *> &order)
{
    Q_UNUSED(project);
    if (order.size() < 2) return;
    QStringList paths;
    emit getConfigPaths(paths);
    if (paths.isEmpty()) {
        qDebug() << "Couldn't get StandardConfigPaths";
        return;
    }
    internConfigPaths.clear();
    for (const QString &path : paths)
        internConfigPaths << path;
    std::sort(order.begin(), order.end(), isBefore);
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
    PExProjectNode *project = group->toProject();
    if (project && project->type() == PExProjectNode::tGams)
        sortGamsProject(project, order);
    else
        std::sort(order.begin(), order.end(), lessThan);

    for (int i = 0; i < order.size(); ++i) {
        QModelIndex parMi = asIndex(group);
        QModelIndex mi = asIndex(order.at(i));
        if (mi.isValid() && mi.row() != i) {
            int row = mi.row();
            beginMoveRows(parMi, row, row, parMi, i);
            group->moveChildNode(row, i);
            endMoveRows();
        }
    }
    if (group == mRoot || (project && project->type() == PExProjectNode::tGams))
        updateProjectExtNums(group);
}

void ProjectTreeModel::updateProjectExtNums(PExGroupNode *group)
{
    for (int i = 0; i < group->childCount(); ++i) {
        PExAbstractNode *node = group->childNode(i);
        if (!node) continue;
        QString name = node->name();
        QString ext;
        int nr = 0;
        for (int j = 0; j < i; ++j) {
            PExAbstractNode *other = group->childNode(j);
            if (other && other->name(NameModifier::withNameExt) == name + ext) {
                ++nr;
                ext = QString::number(nr);
            }
        }
        node->setNameExt(ext);
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
        QModelIndex mi = asIndex(mCurrent);
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
        PExAbstractNode* aNode = mProjectRepo->node(mCurrent);
        if (!aNode || !aNode->assignedProject()) return false;
        if (aNode->assignedProject()->id() == nodeId(ind)) {
            return true;
        }
    }
    return false;
}

QModelIndex ProjectTreeModel::findProject(QModelIndex ind, bool *locked)
{
    if (locked) *locked = false;
    if (ind.isValid()) {
        PExAbstractNode *aNode = node(ind);
        if (!aNode) return ind;
        PExProjectNode *project = aNode->assignedProject();
        if (project->type() > PExProjectNode::tCommon) {
            if (locked) *locked = true;
            return QModelIndex();
        }
        ind = asIndex(project);
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
    emit dataChanged(rootModelIndex(), rootModelIndex());
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
