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
#include <QToolTip>
#include <QTextDocumentFragment>
#include "editors/abstractedit.h"
#include "logger.h"

namespace gams {
namespace studio {

AbstractEdit::AbstractEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    viewport()->installEventFilter(this);
    installEventFilter(this);
}

AbstractEdit::~AbstractEdit()
{}

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
    mimeData->setText(plainTextStr);

    return mimeData;
}

void AbstractEdit::updateGroupId()
{
    marksChanged();
}

void AbstractEdit::setMarks(const LineMarks *marks)
{
    mMarks = marks;
    marksChanged();
}

const LineMarks &AbstractEdit::marks() const
{
    return *mMarks;
}

void AbstractEdit::showToolTip(const QList<TextMark*> marks)
{
    if (marks.size() > 0) {
        QTextCursor cursor(document()->findBlockByNumber(marks.first()->line()));
                //(marks.first()->textCursor());
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, marks.first()->column());
        QPoint pos = cursorRect(cursor).bottomLeft();
        QStringList tips;
        emit requestLstTexts(groupId(), marks, tips);
        QToolTip::showText(mapToGlobal(pos), tips.join("\n"), this);
    }
}

bool AbstractEdit::event(QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        e->ignore();
        return true;
    }
    if (e->type() == QEvent::FontChange) {
        QFontMetrics metric(font());
        setTabStopDistance(8*metric.width(' '));
    }
    return QPlainTextEdit::event(e);
}

bool AbstractEdit::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(e);
        if (!mMarksAtMouse.isEmpty())
            showToolTip(mMarksAtMouse);
        mTipPos = helpEvent->pos();
        return !mMarksAtMouse.isEmpty();
    }
    return QPlainTextEdit::eventFilter(o, e);
}

void AbstractEdit::keyPressEvent(QKeyEvent *e)
{
    QPlainTextEdit::keyPressEvent(e);
    Qt::CursorShape shape = Qt::IBeamCursor;
    if (e->modifiers() & Qt::ControlModifier) {
        if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->cursorShape(&shape, true);
    }
    viewport()->setCursor(shape);
}

void AbstractEdit::keyReleaseEvent(QKeyEvent *e)
{
    QPlainTextEdit::keyReleaseEvent(e);
    Qt::CursorShape shape = Qt::IBeamCursor;
    if (e->modifiers() & Qt::ControlModifier) {
        if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->cursorShape(&shape, true);
    }
    viewport()->setCursor(shape);
}

void AbstractEdit::mousePressEvent(QMouseEvent *e)
{
    QPlainTextEdit::mousePressEvent(e);
    if (!mMarksAtMouse.isEmpty()) {
        mClickPos = e->pos();
    }
}

QList<TextMark*> AbstractEdit::cachedLineMarks(int lineNr)
{
    if (lineNr < 0) {
        mCacheMarks.clear();
        mCacheLine = -1;
        return mCacheMarks;
    }
    if (mCacheLine != lineNr || mCacheGroup != groupId()) {
        mCacheMarks.clear();
        mCacheLine = lineNr;
        mCacheGroup = groupId();
        for (TextMark* mark: mMarks->values(mCacheLine)) {
            if (mark->groupId() == mCacheGroup) mCacheMarks << mark;
        }
    }
    return mCacheMarks;
}

const QList<TextMark *> &AbstractEdit::marksAtMouse() const
{
    return mMarksAtMouse;
}

void AbstractEdit::mouseMoveEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseMoveEvent(e);
    if (QToolTip::isVisible() && (mTipPos-e->pos()).manhattanLength() > 3) {
        mTipPos = QPoint();
        QToolTip::hideText();
    }
    Qt::CursorShape shape = Qt::IBeamCursor;
    if (!mMarks || mMarks->isEmpty()) {
        // No marks or the text is editable
        viewport()->setCursor(shape);
        return;
    }
    QTextCursor cursor = cursorForPosition(e->pos());
    QList<TextMark*> marks = cachedLineMarks(cursor.blockNumber());
    mMarksAtMouse.clear();
    int col = cursor.positionInBlock();
    for (TextMark* mark: marks) {
        if ((!mark->groupId().isValid() || mark->groupId() == groupId()) && mark->inColumn(col))
            mMarksAtMouse << mark;
    }
    if (!mMarksAtMouse.isEmpty() && (isReadOnly() || e->x() < 0))
        shape = mMarksAtMouse.first()->cursorShape(&shape, true);
    viewport()->setCursor(shape);
}

void AbstractEdit::mouseReleaseEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseReleaseEvent(e);
    if (!isReadOnly() && e->pos().x() >= 0) return;
    if (mMarksAtMouse.isEmpty() || !mMarksAtMouse.first()->isValidLink(true)) return;
    if ((mClickPos-e->pos()).manhattanLength() >= 4) return;

    mMarksAtMouse.first()->jumpToRefMark();
}

void AbstractEdit::marksChanged()
{
    cachedLineMarks(-1);
}


void AbstractEdit::jumpTo(const QTextCursor &cursor)
{
    QTextCursor tc = cursor;
    tc.clearSelection();
    setTextCursor(tc);
    // center line vertically
    qreal lines = qreal(rect().height()) / cursorRect().height();
    qreal line = qreal(cursorRect().bottom()) / cursorRect().height();
    int mv = qRound(line - lines/2);
    if (qAbs(mv) > lines/3)
        verticalScrollBar()->setValue(verticalScrollBar()->value()+mv);
}

void AbstractEdit::jumpTo(int line, int column)
{
    QTextCursor cursor;
    if (document()->blockCount()-1 < line) return;
    cursor = QTextCursor(document()->findBlockByNumber(line));
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
    jumpTo(cursor);
}

}
}
