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
#include "treecellresizer.h"
#include <QMouseEvent>
#include <QHeaderView>

namespace gams {
namespace studio {
namespace connect {

TreeCellResizer::TreeCellResizer(QTreeView* view):
    QObject(view), m_view(view)
{
    m_view->viewport()->installEventFilter(this);
    m_view->viewport()->setMouseTracking(true);
    m_sensibility = 5;
    m_drag_in_progress = false;
}

bool TreeCellResizer::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_view->viewport()) {
      QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
      if (mouse_event) {
         if (mouse_event->type() == QEvent::MouseMove) {
            if (m_drag_in_progress) { // apply dragging
               int delta = 0;
               QHeaderView* header_view = m_view->header();
               if (m_drag_orientation == Qt::Horizontal) {
                   delta = mouse_event->pos().x() - m_drag_previous_pos;
                   m_drag_previous_pos = mouse_event->pos().x();
               }
               //using minimal size = m_sensibility * 2 to prevent collapsing
               header_view->resizeSection(m_drag_section,
                      qMax(m_sensibility * 2, header_view->sectionSize(m_drag_section) + delta));
               return true;
          } else { // set mouse cursor shape
            if (index_resizable(mouse_event->pos(), Qt::Horizontal).isValid()) {
              m_view->viewport()->setCursor(Qt::SplitHCursor);
            } else {
              m_view->viewport()->setCursor(QCursor());
            }
          }
        } else if (mouse_event->type() == QEvent::MouseButtonPress &&
                   mouse_event->button() == Qt::LeftButton &&
                   !m_drag_in_progress) { // start dragging
          if (index_resizable(mouse_event->pos(), Qt::Horizontal).isValid()) {
               m_drag_in_progress = true;
               m_drag_orientation = Qt::Horizontal;
               m_drag_previous_pos = static_cast<int>(mouse_event->position().x());
               m_drag_section = index_resizable(mouse_event->pos(), Qt::Horizontal).column();
               return true;
          }
        } else if (mouse_event->type() == QEvent::MouseButtonRelease &&
                   mouse_event->button() == Qt::LeftButton &&
                   m_drag_in_progress) { // stop dragging
                 m_drag_in_progress = false;
                 return true;
        }
      }
    }
    return false;
}

QModelIndex TreeCellResizer::index_resizable(QPoint mouse_pos, Qt::Orientation orientation)
{
    QModelIndex index = m_view->indexAt(mouse_pos - (orientation == Qt::Vertical ? QPoint(0, m_sensibility + 1) : QPoint(m_sensibility + 1, 0)));
    if (index.isValid()) {
        if (orientation == Qt::Horizontal) {
          if (qAbs(m_view->visualRect(index).right() - mouse_pos.x()) < m_sensibility &&
              m_view->header()->sectionResizeMode(index.column()) == QHeaderView::Interactive) {
            return index;
          }
        }
     }
     return QModelIndex();
}


} // namespace connect
} // namespace studio
} // namespace gams
