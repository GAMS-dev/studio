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
#include "resulttreeview.h"

#include <QEvent>
#include <QMenu>
#include <QWheelEvent>

namespace gams {
namespace studio {
namespace search {

ResultTreeView::ResultTreeView(QWidget *parent)
    : QTreeView(parent)
    , mMenu(new QMenu(this))
    , mCollapsAllAction(new QAction("Collapse All", this))
    , mExpandAllAction(new QAction("Expand All", this))
{
    mMenu->addAction(mCollapsAllAction);
    mMenu->addAction(mExpandAllAction);

    connect(this, &ResultTreeView::customContextMenuRequested,
            this, &ResultTreeView::showCustomContextMenu);

    connect(this, &ResultTreeView::collapsed, this, &ResultTreeView::resizeColumns);
    connect(this, &ResultTreeView::expanded, this, &ResultTreeView::resizeColumns);
    connect(mCollapsAllAction, &QAction::triggered, this, [this]{
        disconnect(this, &ResultTreeView::collapsed, nullptr, nullptr);
        collapseAll();
        resizeColumns();
        connect(this, &ResultTreeView::collapsed, this, &ResultTreeView::resizeColumns);
    });
    connect(mExpandAllAction, &QAction::triggered, this, [this]{
        disconnect(this, &ResultTreeView::expanded, nullptr, nullptr);
        expandAll();
        resizeColumns();
        connect(this, &ResultTreeView::expanded, this, &ResultTreeView::resizeColumns);
    });
}

bool ResultTreeView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheel = static_cast<QWheelEvent*>(event);
        if (wheel->modifiers() == Qt::ControlModifier) {
            if (wheel->angleDelta().y() > 0)
                zoomIn(ZOOM_FACTOR);
            else
                zoomOut(ZOOM_FACTOR);
            return true;
        }
    }
    return QTreeView::eventFilter(watched, event);
}

void ResultTreeView::zoomIn(int range)
{
    zoom(range);
}

void ResultTreeView::zoomOut(int range)
{
    zoom(-range);
}

void ResultTreeView::resetZoom()
{
    setFont(mBaseFont);
}

void ResultTreeView::showCustomContextMenu(const QPoint &pos)
{
    mMenu->popup(viewport()->mapToGlobal(pos));
}

void ResultTreeView::resizeColumns()
{
    for (int i=0; i<model()->columnCount(); ++i) {
        resizeColumnToContents(i);
    }
}

void ResultTreeView::zoom(int range)
{
    if (range == 0)
        return;
    QFont f = font();
    auto newSize = f.pointSizeF() + range;
    if (newSize <= 0)
        return;
    f.setPointSizeF(newSize);
    setFont(f);
    resizeColumns();
}

}
}
}
