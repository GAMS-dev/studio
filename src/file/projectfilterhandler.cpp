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
#include "projectfilterhandler.h"
#include "projectproxymodel.h"
#include "projecttreemodel.h"

namespace gams {
namespace studio {

ProjectFilterHandler::ProjectFilterHandler(ProjectProxyModel *parent, ProjectTreeView *treeView)
    : QObject{parent}, mModel(parent), mTreeView(treeView)
{
    mModel->setRecursiveFilteringEnabled(true);
}

void ProjectFilterHandler::regExpChanged(QRegularExpression regExp)
{
    QModelIndex root = mTreeView->rootIndex();
    mModel->setFilterRegularExpression(regExp);
    mTreeView->setRootIndex(root);
    mTreeView->restoreExpansion();
}

void ProjectFilterHandler::setSortCombined(SortType type)
{
    int roles[] = {Qt::EditRole, ProjectTreeModel::SuffixRole, ProjectTreeModel::UsageRole};
    int sortRole = roles[(type / 2) % 3];

    mModel->setSortRole(sortRole);
    mModel->sort(0, Qt::SortOrder(type % 2));
    mType = type;
}

void ProjectFilterHandler::setSortKey(SortType type)
{
    Qt::SortOrder order = mType % 2 ? Qt::DescendingOrder : Qt::AscendingOrder;
    SortType newType = SortType(((type / 2) * 2) + order);
    setSortCombined(newType);
}

void ProjectFilterHandler::setSortOrder(Qt::SortOrder order)
{
    SortType lastType = SortType((mType / 2) * 2);
    setSortCombined(SortType(lastType + order));
}

Qt::SortOrder ProjectFilterHandler::sortOrder() const
{
    return mType % 2 ? Qt::DescendingOrder : Qt::AscendingOrder;
}

int ProjectFilterHandler::sortKey() const
{
    return mType / 2;
}

void ProjectFilterHandler::invalidate()
{
    mModel->invalidate();
}

ProjectFilterHandler::SortType ProjectFilterHandler::lastSortType() const
{
    return mType;
}

} // namespace studio
} // namespace gams
