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
#include <QPainter>
#include <QTableView>
#include <QMouseEvent>
#include <QMenu>

#include "sortedfileheaderview.h"
#include "symboltablemodel.h"
#include "theme.h"

namespace gams {
namespace studio {
namespace reference {

SortedFileHeaderView::SortedFileHeaderView(Qt::Orientation orientation, QWidget *parent)  :
    QHeaderView(orientation, parent)
{
    mHeaderContextMenu = new QMenu(this);
    QAction* actionAscendingSort  = new QAction("Ascending Order", this);
    actionAscendingSort->setData(SymbolTableModel::AscendingOrder);
    actionAscendingSort->setIcon( QIcon(Theme::icon(":/%1/sort-asc")) );
    QAction* actionSortByOrderUsed  = new QAction("Original Order", this);
    actionSortByOrderUsed->setData(SymbolTableModel::OriginalOrder);
    actionSortByOrderUsed->setIcon( QIcon(Theme::icon(":/%1/sort")) );
    QAction* actionDescendingSort  = new QAction("Descending Order", this);
    actionDescendingSort->setData(SymbolTableModel::DescendingOrder);
    actionDescendingSort->setIcon( QIcon(Theme::icon(":/%1/sort-desc")) );
    mHeaderContextMenu->addAction(actionAscendingSort);
    mHeaderContextMenu->addAction(actionSortByOrderUsed);
    mHeaderContextMenu->addAction(actionDescendingSort);
    connect(mHeaderContextMenu, &QMenu::triggered, this, &SortedFileHeaderView::onSortFileUsed);
}

SortedFileHeaderView::~SortedFileHeaderView()
{
    if (mHeaderContextMenu)
        delete mHeaderContextMenu;
}

bool SortedFileHeaderView::event(QEvent *event)
{
    return QWidget::event(event);
}

void SortedFileHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && isCoordinate(event->pos())) {
        QPoint p = QPoint(mIconX-mHeaderContextMenu->sizeHint().width()+mIconWidth, mIconY + mIconWidth);
        QTableView* tableview = static_cast<QTableView*>(this->parent());
        mHeaderContextMenu->popup( tableview->horizontalHeader()->viewport()->mapToGlobal( p ) );
    }
    QHeaderView::mousePressEvent(event);
}

void SortedFileHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if (!painter->isActive()) return;
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (logicalIndex == 0) {
        QIcon icon;
        switch(mCurrentSortOrder) {
          case SymbolTableModel::AscendingOrder : icon = QIcon(Theme::icon(":/%1/sort-asc"));  break;
          case SymbolTableModel::DescendingOrder: icon = QIcon(Theme::icon(":/%1/sort-desc")); break;
          case SymbolTableModel::OriginalOrder  : icon = QIcon(Theme::icon(":/%1/sort")); break;
          default: icon = QIcon(Theme::icon(":/%1/sort")); break;
        }
        int iconWidth = static_cast<int>(rect.height()*ICON_SCALE_FACTOR);
        int iconMargin = static_cast<int>((rect.height() - iconWidth)*ICON_MARGIN_FACTOR);
        QPixmap pm = icon.pixmap(iconWidth, iconWidth);

        int posX = rect.topRight().x() - iconWidth - iconMargin;
        int posY = rect.topRight().y() + iconMargin;

        painter->drawImage(posX, posY, pm.toImage());

        mIconWidth = iconWidth;
        mIconX = posX;
        mIconY= posY;
        mLogicalIndex = logicalIndex;
    }
}


bool SortedFileHeaderView::isCoordinate(QPoint p)
{
    int index = logicalIndexAt(p);
    if (index != mLogicalIndex)
        return false;

    return (p.x() >= mIconX && p.x() <= mIconX+mIconWidth && p.y() >= mIconY && p.y() <= mIconY+mIconWidth);
}

void SortedFileHeaderView::onSortFileUsed(QAction *action)
{
    QTableView* tableView = static_cast<QTableView*>(this->parent());
    SymbolTableModel* tablemodel = static_cast<SymbolTableModel*>(tableView->model());
    mCurrentSortOrder = static_cast<SymbolTableModel::FileUsedSortOrder>(action->data().toInt());
    tablemodel->sortFileUsed( mCurrentSortOrder );
}

} // namepsace reference
} // namespace studio
} // namespace gams
