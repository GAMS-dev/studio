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
#include "tableview.h"

#include <QEvent>
#include <QHeaderView>
#include <QWheelEvent>
#include <QTimer>

namespace gams {
namespace studio {
namespace modeldialog {

TableView::TableView(QWidget *parent)
    : QTableView(parent)
{

}

bool TableView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheel = static_cast<QWheelEvent*>(event);
        if (wheel->modifiers() == Qt::ControlModifier) {
            if (wheel->angleDelta().y() > 0)
                zoomIn(mZoomFactor);
            else
                zoomOut(mZoomFactor);
            return true;
        }
    }
    return QTableView::eventFilter(watched, event);
}

void TableView::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier ||
        (event->modifiers() ^ Qt::ControlModifier && event->modifiers() ^ Qt::KeypadModifier)) {
        if (event->key() == Qt::Key_Plus) {
            zoomIn(mZoomFactor);
        } else if (event->key() == Qt::Key_Minus) {
            zoomOut(mZoomFactor);
        } else if (event->key() == Qt::Key_0) {
            resetZoom();
        }
    }
    if (event->modifiers() != Qt::ControlModifier) {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            emit confirmCurrent();
        } else if (event->key() == Qt::Key_Escape) {
            emit closeDialog();
        } else if (event->key() == Qt::Key_Left) {
            emit nextTab(true);
        } else if (event->key() == Qt::Key_Right) {
            emit nextTab(false);
        } else if (event->key() == Qt::Key_Tab) {
            emit tabOut(false);
        } else if (event->key() == Qt::Key_Backtab) {
            emit tabOut(true);
        } if (model()->rowCount() > 1) {
            if (event->key() == Qt::Key_Down) {
                selectRow(qMin(currentIndex().row() + 1, model()->rowCount() - 1));
            } else if (event->key() == Qt::Key_Up) {
                selectRow(qMax(currentIndex().row() - 1, 0));
            } else if (event->key() == Qt::Key_PageDown) {
                int visRows = (geometry().height() / rowHeight(0));
                selectRow(qMin(currentIndex().row() + visRows, model()->rowCount() - 1));
            } else if (event->key() == Qt::Key_PageUp) {
                int visRows = (geometry().height() / rowHeight(0));
                selectRow(qMax(currentIndex().row() - visRows, 0));
            } else if (event->key() == Qt::Key_Home) {
                selectRow(0);
            } else if (event->key() == Qt::Key_End) {
                selectRow(model()->rowCount() - 1);
            }
        }
    }
}

void TableView::focusInEvent(QFocusEvent *event)
{
    QTableView::focusInEvent(event);
    QTimer::singleShot(0, this, [this]() {
        if (!currentIndex().isValid() && model()->rowCount())
            selectRow(0);
    });
}

void TableView::zoomIn(int range)
{
    zoom(range);
    resizeColumnsToContents();
    resizeRowsToContents();
}

void TableView::zoomOut(int range)
{
    zoom(-range);
    resizeColumnsToContents();
    resizeRowsToContents();
}

void TableView::resetZoom()
{
    setFont(mBaseFont);
    resizeColumnsToContents();
    resizeRowsToContents();
}

void TableView::zoom(int range)
{
    if (range == 0.f)
        return;
    QFont f = font();
    const float newSize = f.pointSizeF() + range;
    if (newSize <= 0)
        return;
    f.setPointSize(newSize);
    setFont(f);
    resizeColumnsToContents();
    resizeRowsToContents();
}

}
}
}
