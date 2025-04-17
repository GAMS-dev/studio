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
#include "gdxsymboltableview.h"

#include <QScrollBar>
#include <QKeyEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolTableView::GdxSymbolTableView(QWidget *parent) :
    QTableView(parent)
{
}

void GdxSymbolTableView::scrollTo(const QModelIndex &index, ScrollHint hint) {
    // prevent jumping of horizontal scrollbar when selected symbol changes but still allow to scroll with arrow keys
    const int x = horizontalScrollBar()->value();
    QTableView::scrollTo(index, hint);
    if (!mHorizontalUserNavigation)
        horizontalScrollBar()->setValue(x);
}

void GdxSymbolTableView::keyPressEvent(QKeyEvent *event)
{
    // enable horizontal scrolling in case it was triggered by arrow keys
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
        mHorizontalUserNavigation = true;

    QTableView::keyPressEvent(event);
    mHorizontalUserNavigation = false;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
