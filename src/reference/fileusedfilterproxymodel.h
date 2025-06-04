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
#ifndef FILEUSEDFILTERPROXYMODEL_H
#define FILEUSEDFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace gams {
namespace studio {
namespace reference {

class FileUsedFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FileUsedFilterProxyModel(QObject *parent = nullptr);

public slots:
    void showFileOnceChanged(bool showOnce);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
    bool filterAcceptsSelfRow(int sourceRow, const QModelIndex& sourceParent) const;
    bool hasAcceptedChildren(int sourceRow, const QModelIndex& sourceParent) const;

    bool mShowFileOnce;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // FILEUSEDFILTERPROXYMODEL_H
