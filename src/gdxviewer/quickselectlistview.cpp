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
#include "columnfilterframe.h"
#include "quickselectlistview.h"
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

QuickSelectListView::QuickSelectListView(QWidget *parent) :
    QListView(parent)
{

}

void QuickSelectListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier)
        event->accept();
    else
        QListView::mousePressEvent(event);
}

void QuickSelectListView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)) {
        QModelIndex idx = this->indexAt(event->pos());
        if (idx.isValid()) {
            for(int row=0; row<model()->rowCount(); row++)
                model()->setData(model()->index(row,0), false, Qt::CheckStateRole);
            this->model()->setData(idx, true, Qt::CheckStateRole);
            emit quickSelect();
        }
        event->accept();
    } else if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier) {
        QModelIndex idxTo = this->indexAt(event->pos());
        if (idxTo.isValid()) {
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
        }
    } else
        QListView::mouseReleaseEvent(event);
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
