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
#ifndef SORTEDFILEHEADERVIEW_H
#define SORTEDFILEHEADERVIEW_H

#include <QHeaderView>

#include "symboltablemodel.h"
#include "headerviewproxy.h"

namespace gams {
namespace studio {
namespace reference {

class SortedFileHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    SortedFileHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
    ~SortedFileHeaderView() override;

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

private:
    bool isCoordinate(QPoint p);
    void onSortFileUsed(QAction *action);

private:
    const double ICON_SCALE_FACTOR = 0.7;
    const double ICON_MARGIN_FACTOR = 0.3;

    mutable int mIconWidth;
    mutable int mIconX;
    mutable int mIconY;
    mutable int mLogicalIndex;

    QMenu* mHeaderContextMenu;
    SymbolTableModel::FileUsedSortOrder  mCurrentSortOrder = SymbolTableModel::FileUsedSortOrder::OriginalOrder;
};

} // namepsage reference
} // namespace studio
} // namespace gams

#endif // SORTEDFILEHEADERVIEW_H
