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
#ifndef TREECELLRESIZER_H
#define TREECELLRESIZER_H

#include <QObject>
#include <QTreeView>

namespace gams {
namespace studio {
namespace connect {

class TreeCellResizer : public QObject
{
    Q_OBJECT
public:
    explicit TreeCellResizer(QTreeView *view = nullptr);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private:
    QTreeView* m_view;

    //max distance between mouse and a cell, small enough to trigger resize
    int m_sensibility;

    //variables for saving state while dragging
    bool m_drag_in_progress;
    Qt::Orientation m_drag_orientation;
    int m_drag_section;
    int m_drag_previous_pos;

    // check if mouse_pos is around right or bottom side of a cell
    // (depending on orientation)
    // and return the index of that cell if found
    QModelIndex index_resizable(QPoint mouse_pos, Qt::Orientation orientation);
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // TREECELLRESIZER_H
