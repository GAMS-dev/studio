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
#include <QMimeData>
#include <QTextBlock>
#include <QScrollBar>
#include <QTextDocumentFragment>
#include "editors/abstractedit.h"

namespace gams {
namespace studio {

AbstractEdit::AbstractEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
}

AbstractEdit::~AbstractEdit()
{

}

void AbstractEdit::setOverwriteMode(bool overwrite)
{
    QPlainTextEdit::setOverwriteMode(overwrite);
}

bool AbstractEdit::overwriteMode() const
{
    return QPlainTextEdit::overwriteMode();
}

QMimeData* AbstractEdit::createMimeDataFromSelection() const
{
    QMimeData* mimeData = new QMimeData();
    QTextCursor c = textCursor();
    QString plainTextStr = c.selection().toPlainText();
    mimeData->setText( plainTextStr );

    return mimeData;
}

void AbstractEdit::afterContentsChanged(int, int, int)
{
    QTextCursor tc = textCursor();
    int pos = tc.position();
    tc.setPosition(pos);
    setTextCursor(tc);
}

bool AbstractEdit::event(QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        e->ignore();
        return true;
    } else {
        return QPlainTextEdit::event(e);
    }
}

void AbstractEdit::jumpTo(const QTextCursor &cursor, int altLine, int altColumn)
{
    QTextCursor tc;
    if (cursor.isNull()) {
        if (document()->blockCount()-1 < altLine) return;
        tc = QTextCursor(document()->findBlockByNumber(altLine));
    } else {
        tc = cursor;
    }

    if (cursor.isNull()) tc.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, altColumn);
    tc.clearSelection();
    setTextCursor(tc);
    // center line vertically
    qreal lines = qreal(rect().height()) / cursorRect().height();
    qreal line = qreal(cursorRect().bottom()) / cursorRect().height();
    int mv = line - lines/2;
    if (qAbs(mv) > lines/3)
        verticalScrollBar()->setValue(verticalScrollBar()->value()+mv);
}

}
}
