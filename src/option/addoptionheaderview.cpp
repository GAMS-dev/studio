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
#include <QPainter>
#include <QTableView>
#include <QMouseEvent>
#include <QToolTip>

#include "addoptionheaderview.h"

namespace gams {
namespace studio {


AddOptionHeaderView::AddOptionHeaderView(Qt::Orientation orientation, QWidget *parent) :
        QHeaderView(orientation, parent)
{

}

AddOptionHeaderView::~AddOptionHeaderView()
{

}

bool AddOptionHeaderView::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        if (isAddOptionCoordinate(helpEvent->pos())) {
            QToolTip::showText(helpEvent->globalPos(), "add new option");
        } else {
            QToolTip::hideText();
            event->ignore();
        }

        return true;
    }
    return QWidget::event(event);
}

void AddOptionHeaderView::mousePressEvent(QMouseEvent* event)
{
    if (Qt::LeftButton == event->button() && isAddOptionCoordinate(event->pos())) {
        QTableView* tableView = static_cast<QTableView*>(this->parent());
        tableView->model()->insertRows(tableView->model()->rowCount(), 1, QModelIndex());
    }

   QHeaderView::mousePressEvent(event);
}

bool AddOptionHeaderView::isAddOptionCoordinate(QPoint p)
{
    int index = logicalIndexAt(p);
    if (index != mLogicalIndex)
        return false;

    return (p.x() >= mIconX && p.x() <= mIconX+mIconWidth && p.y() >= mIconY && p.y() <= mIconY+mIconWidth);
}

void AddOptionHeaderView::paintSection(QPainter* painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (logicalIndex == 0) {
         QIcon icon(iconStr);
         int iconWidth = rect.height()*ICON_SCALE_FACTOR;
         int iconMargin = (rect.height() - iconWidth)*ICON_MARGIN_FACTOR;
         QPixmap pm = icon.pixmap(iconWidth, iconWidth);

         int posX = rect.topLeft().x() + iconMargin;
         int posY = rect.topLeft().y() + iconMargin;

         painter->drawImage(posX, posY, pm.toImage());

         mIconWidth = iconWidth;
         mIconX = posX;
         mIconY= posY;
         mLogicalIndex = logicalIndex;
    }
}

} // namespace studio
} // namespace gams
