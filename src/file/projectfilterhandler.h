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
#ifndef PROJECTFILTERHANDLER_H
#define PROJECTFILTERHANDLER_H

#include <QObject>
#include <QRegularExpression>
#include "projecttreeview.h"
// #include <QModelIndexList>

namespace gams {
namespace studio {

class ProjectProxyModel;

class ProjectFilterHandler : public QObject
{
    Q_OBJECT
public:
    enum SortType { sortNameAsc, sortNameDesc, sortSuffixAsc, sortSuffixDesc, sortUseAsc, sortUseDesc };

    explicit ProjectFilterHandler(ProjectProxyModel *parent, ProjectTreeView *treeView);

public slots:
    void regExpChanged(QRegularExpression regExp);
    void setSortCombined(SortType type);
    void setSortKey(SortType type);
    void setSortOrder(Qt::SortOrder order);
    Qt::SortOrder sortOrder() const;
    int sortKey() const;
    SortType lastSortType() const;
    void invalidate();

private:
    ProjectProxyModel *mModel = nullptr;
    ProjectTreeView *mTreeView = nullptr;
    QHash<NodeId, bool> mFoldedNodes;
    SortType mType = sortNameAsc;

};

} // namespace studio
} // namespace gams

#endif // PROJECTFILTERHANDLER_H
