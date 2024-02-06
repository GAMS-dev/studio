/*
 * This file is part of the GAMS Studio project.
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
#include "navigatorlineedit.h"

namespace gams {
namespace studio {

NavigatorLineEdit::NavigatorLineEdit(QWidget* parent) : FilterLineEdit(parent)
{ }

void NavigatorLineEdit::mouseReleaseEvent(QMouseEvent* event)
{
    emit receivedFocus();
    QLineEdit::mouseReleaseEvent(event);
}

void NavigatorLineEdit::focusInEvent(QFocusEvent *event)
{
    // do not send receivedFocus() here because this widget gets focus after closing the dialog
    QLineEdit::focusInEvent(event);
}

void NavigatorLineEdit::focusOutEvent(QFocusEvent *event)
{
    emit lostFocus();
    QLineEdit::focusOutEvent(event);
}

void NavigatorLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
            event->key() == Qt::Key_Escape || event->key() == Qt::Key_Return ||
            event->key() == Qt::Key_Enter) {
        emit sendKeyEvent(event);
    } else if ( event->key() == Qt::Key_Colon) {
        QLineEdit::keyPressEvent(event);
        emit autocompleteTriggered();
    } else {
        QLineEdit::keyPressEvent(event);
    }
}

}
}
