/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "quickselectlistview.h"
#include "labelfilterdelegate.h"
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

QuickSelectListView::QuickSelectListView(QWidget *parent) :
    QListView(parent)
{
    setItemDelegate(new LabelFilterDelegate(this));
}

void QuickSelectListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier)
        event->accept();
    else if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)) {
        QModelIndex idx = this->indexAt(event->pos());
        if (idx.isValid()) {
            QSortFilterProxyModel *proxy = suspendSorting();
            for(int row=0; row<model()->rowCount(); row++)
                model()->setData(model()->index(row,0), false, Qt::CheckStateRole);
            this->model()->setData(idx, true, Qt::CheckStateRole);
            resumeSorting(proxy);
            emit quickSelect();
        }
        event->accept();
    }
    else
        QListView::mousePressEvent(event);
}

void QuickSelectListView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier) {
        QModelIndex idxTo = this->indexAt(event->pos());
        if (idxTo.isValid()) {
            QSortFilterProxyModel *proxy = suspendSorting();
            int start = 0;
            int end = idxTo.row();
            QModelIndexList indexList = this->selectedIndexes();
            if (indexList.size() > 0)
                start = this->selectedIndexes().at(0).row();
            if (start > end) {
                int tmp = start;
                start = end;
                end = tmp;
            }
            bool checked = this->model()->data(idxTo, Qt::CheckStateRole).toBool();
            for (int i = start; i<=end; i++)
                model()->setData(model()->index(i,0), !checked, Qt::CheckStateRole);
            resumeSorting(proxy);
        }
    } else
        QListView::mouseReleaseEvent(event);
}

QSortFilterProxyModel *QuickSelectListView::suspendSorting()
{
    QSortFilterProxyModel* proxy = qobject_cast<QSortFilterProxyModel*>(model());
    if (proxy)
        proxy->setDynamicSortFilter(false);
    return proxy;
}

void QuickSelectListView::resumeSorting(QSortFilterProxyModel *proxy)
{
    if (proxy) {
        proxy->setDynamicSortFilter(true);
        proxy->invalidate();
    }
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
