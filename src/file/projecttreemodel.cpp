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
#include "filemeta.h"
#include "logger.h"

namespace gams {
namespace studio {

ProjectTreeModel::ProjectTreeModel(ProjectRepo* parent, ProjectGroupNode* root)
    : QAbstractItemModel(parent), mProjectRepo(parent), mRoot(root)
{
    if (!mProjectRepo)
        FATAL() << "nullptr not allowed. The FileTreeModel needs a valid FileRepository.";
}

QModelIndex ProjectTreeModel::index(ProjectAbstractNode *entry) const
{
    if (!entry)
        return QModelIndex();
    if (!entry->parentNode())
        return createIndex(0, 0, entry->id());
    for (int i = 0; i < entry->parentNode()->childCount(); ++i) {
        if (entry->parentNode()->childEntry(i) == entry) {
            return createIndex(i, 0, entry->id());
        }
    }
    return QModelIndex();
}

QModelIndex ProjectTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    ProjectAbstractNode *fc = mProjectRepo->groupNode(parent)->childEntry(row);
    if (!fc)
        FATAL() << "invalid child for row " << row;
    return createIndex(row, column, fc->id());
}

QModelIndex ProjectTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) return QModelIndex();
    ProjectAbstractNode* eChild = mProjectRepo->node(child);
    if (!eChild || eChild == mRoot)
        return QModelIndex();
    ProjectGroupNode* eParent = eChild->parentNode();
    if (!eParent)
        return QModelIndex();
    if (eParent == mRoot)
        return createIndex(0, child.column(), eParent->id());
    ProjectGroupNode* parParent = eParent->parentNode();
    int row = parParent ? parParent->indexOf(eParent) : -1;
    if (row < 0)
        FATAL() << "could not find child in parent";
    return createIndex(row, child.column(), eParent->id());
}

int ProjectTreeModel::rowCount(const QModelIndex& parent) const
{
    ProjectGroupNode* entry = mProjectRepo->groupNode(parent);
    if (!entry) return 0;
    return entry->childCount();
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
    case Qt::BackgroundColorRole:
        if (isSelected(ind)) return QColor("#4466BBFF");

    case Qt::DisplayRole:
        return mProjectRepo->node(ind)->name(NameModifier::EditState);

    case Qt::FontRole: {
        if (isCurrent(ind) || isCurrentGroup(ind)) {
            QFont f;
            f.setBold(true);
            return f;
        }
    }
        break;

    case Qt::ForegroundRole: {
        ProjectAbstractNode* node = mProjectRepo->node(ind);
        if (!node) return QVariant();
        if (node->toFile() && !node->toFile()->file()->exists())
            return QColor(Qt::red);

        if (node->isActive()) {
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
    return createIndex(0, 0, mRoot->id());
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
    EXCEPT() << "FileTreeModel::removeRows is unsupported, please use FileTreeModel::removeChild";
    return false;
}

bool ProjectTreeModel::insertChild(int row, ProjectGroupNode* parent, ProjectAbstractNode* child)
{
    QModelIndex parMi = index(parent);
    if (!parMi.isValid()) return false;
    beginInsertRows(parMi, row, row);
    child->setParentNode(parent);
    endInsertRows();
    return true;
}

bool ProjectTreeModel::removeChild(ProjectAbstractNode* child)
{
    QModelIndex mi = index(child);
    if (!mi.isValid()) return false;
    QModelIndex parMi = index(child->parentNode());
    beginRemoveRows(parMi, mi.row(), mi.row());
    child->setParentNode(nullptr);
    endRemoveRows();
    return true;
}

bool ProjectTreeModel::isCurrent(const QModelIndex& ind) const
{
    return (mCurrent.isValid() && ind == mCurrent);
}

void ProjectTreeModel::setCurrent(const QModelIndex& ind)
{
    if (!isCurrent(ind)) {
        QModelIndex mi = mCurrent;
        mCurrent = ind;
        if (mi.isValid()) {
            dataChanged(mi, mi);                        // invalidate old
            if (mProjectRepo->node(mi)) {
                QModelIndex par = index(mProjectRepo->node(mi)->parentNode());
                if (par.isValid()) dataChanged(par, par);
            }
        }
        if (mCurrent.isValid()) {
            dataChanged(mCurrent, mCurrent);            // invalidate new
            QModelIndex par = index(mProjectRepo->node(mCurrent)->parentNode());
            if (par.isValid()) dataChanged(par, par);
        }
    }
}

bool ProjectTreeModel::isCurrentGroup(const QModelIndex& ind) const
{
    if (mCurrent.isValid()) {
        ProjectAbstractNode* fsc = mProjectRepo->node(mCurrent);
        if (!fsc) return false;
        if (fsc->parentNode()->id() == static_cast<FileId>(ind.internalId())) {
            return true;
        }
    }
    return false;
}

bool ProjectTreeModel::isSelected(const QModelIndex& ind) const
{
    return (mSelected.isValid() && ind == mSelected);
}

void ProjectTreeModel::setSelected(const QModelIndex& ind)
{
    if (!isSelected(ind)) {
        QModelIndex mi = mSelected;
        mSelected = ind;
        if (mi.isValid()) dataChanged(mi, mi);                      // invalidate old
        if (mSelected.isValid()) dataChanged(mSelected, mSelected); // invalidate new
    }
}

} // namespace studio
} // namespace gams
