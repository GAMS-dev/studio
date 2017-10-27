/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include <QtWidgets>
#include "codeeditor.h"

namespace gams {
namespace studio {

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    this->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, &CodeEditor::updateBlockSelection, this, &CodeEditor::onUpdateBlockSelection, Qt::QueuedConnection);
    connect(this, &CodeEditor::updateBlockEdit, this, &CodeEditor::onUpdateBlockEdit, Qt::QueuedConnection);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    setMouseTracking(true);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        lineNumberArea->scroll(0, dy);
    } else {
        int top = rect.y();
        int bottom = top + rect.height();
        QTextBlock b = firstVisibleBlock();
        while (b.isValid() && b.isVisible()) {
            QRect blockBounds = blockBoundingGeometry(b).translated(contentOffset()).toAlignedRect();
            if (top > blockBounds.top() && top < blockBounds.bottom())
                top = blockBounds.top();
            if (bottom > blockBounds.top() && bottom < blockBounds.bottom()-1)
                bottom = blockBounds.bottom()-1;
            b = b.next();
        }
        lineNumberArea->update(0, top, lineNumberArea->width(), bottom-top);
    }

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::onUpdateBlockSelection()
{
    if (mBlockLastCursor != textCursor()) {
        mBlockLastCursor = textCursor();

        int cols = textCursor().columnNumber()-mBlockStartCursor.columnNumber();
        QTextCursor::MoveOperation colMove = (cols > 0) ? QTextCursor::Right : QTextCursor::Left;
        int rows = textCursor().blockNumber()-mBlockStartCursor.blockNumber();
        QTextCursor::MoveOperation rowMove = (rows > 0) ? QTextCursor::Down : QTextCursor::Up;

        QList<QTextEdit::ExtraSelection> sel = QList<QTextEdit::ExtraSelection>();

        QTextCursor cursor = mBlockStartCursor;
        for (int r = 0; r <= qAbs(rows); ++r) {
            QTextEdit::ExtraSelection es;
            es.format.setBackground(Qt::cyan);
            es.cursor = cursor;

            if (cols) {
                es.cursor.movePosition(colMove, QTextCursor::KeepAnchor, qAbs(cols));
            }
            int dist = es.cursor.anchor() - es.cursor.position();
            mCurrentCol = es.cursor.columnNumber() + (dist>0 ? 0 : dist);
            sel << es;
            cursor.movePosition(rowMove);
        }
        setExtraSelections(sel);
        qDebug() << "Now we got " << sel.size() << " extraSelections";
    }
}

void CodeEditor::onUpdateBlockEdit()
{
    qDebug() << "Enter onUpdateBlockEdit()   /extraSelections: " << extraSelections().size();
    if (extraSelections().count() < 2)
        return;
    qDebug() << "Process onUpdateBlockEdit()";
    // Get the inserted Text
    QTextCursor cursor = textCursor();
    int diff = cursor.columnNumber() - mCurrentCol;
    cursor.setPosition(cursor.position()-diff);
    cursor.setPosition(textCursor().position(), QTextCursor::KeepAnchor);
    QString text = cursor.selectedText();
    qDebug() << "Found inserted Text: " << text << " ... " << diff;
    cursor = textCursor();
    // TODO(JM) repeat selection-remove and insertion
    for (QTextEdit::ExtraSelection sel: extraSelections()) {
        if (sel.cursor == cursor) continue;
        sel.cursor.removeSelectedText();
        sel.cursor.insertText(text);
        setTextCursor(sel.cursor);
    }
    setTextCursor(cursor);
    mCurrentCol = cursor.columnNumber();
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent* e)
{
    QSet<int> moveKeys;
    QKeyEvent ev(*e);
    moveKeys << Qt::Key_Home << Qt::Key_End << Qt::Key_Down << Qt::Key_Up << Qt::Key_Left << Qt::Key_Right
             << Qt::Key_PageUp << Qt::Key_PageDown;
    // TODO(JM) get definition from studio key-config
    if (!mBlockStartKey && e->modifiers() & Qt::AltModifier && e->modifiers() & Qt::ShiftModifier) {
        mBlockStartKey = e->key();
        mBlockStartCursor = textCursor();
        mBlockLastCursor = mBlockStartCursor;
        qDebug() << "blockEdit START";
    }
    if (!mBlockStartKey) {
        if (moveKeys.contains(ev.key())) {
            setExtraSelections(QList<QTextEdit::ExtraSelection>());
            qDebug() << "Now we got " << extraSelections().size() << " extraSelections";
            mBlockStartCursor = QTextCursor();
        } else if (extraSelections().size() > 1) {
            if (mBlockStartCursor.blockNumber() > mBlockLastCursor.blockNumber()) {
                setTextCursor(extraSelections().first().cursor);
            } else {
                setTextCursor(extraSelections().last().cursor);
            }
            emit updateBlockEdit();
        }
    }
    if (mBlockStartKey) {
        ev.setModifiers(0);
        emit updateBlockSelection();
    }
    QPlainTextEdit::keyPressEvent(&ev);
}

void CodeEditor::keyReleaseEvent(QKeyEvent* e)
{
    QPlainTextEdit::keyReleaseEvent(e);
    if (mBlockStartKey && e->key() == mBlockStartKey) {
        mBlockStartKey = 0;
        mBlockLastCursor = QTextCursor();
        mBlockStartCursor = QTextCursor();
        qDebug() << "blockEdit END";
    }
}

void CodeEditor::mouseMoveEvent(QMouseEvent* e)
{
    QPlainTextEdit::mouseMoveEvent(e);
    if (mBlockStartKey)
        emit updateBlockSelection();
    else if (!mBlockStartCursor.isNull()) {
        setExtraSelections(QList<QTextEdit::ExtraSelection>());
        qDebug() << "Now we got " << extraSelections().size() << " extraSelections";
        mBlockStartCursor = QTextCursor();
    }
}

void CodeEditor::mousePressEvent(QMouseEvent* e)
{
    QPlainTextEdit::mousePressEvent(e);
    if (mBlockStartKey)
        emit updateBlockSelection();
    else if (!mBlockStartCursor.isNull()) {
        setExtraSelections(QList<QTextEdit::ExtraSelection>());
        qDebug() << "Now we got " << extraSelections().size() << " extraSelections";
        mBlockStartCursor = QTextCursor();
    }
}

void CodeEditor::mouseReleaseEvent(QMouseEvent* e)
{
    QPlainTextEdit::mouseReleaseEvent(e);
    if (mBlockStartKey)
        emit updateBlockSelection();
    else if (!mBlockStartCursor.isNull()) {
        setExtraSelections(QList<QTextEdit::ExtraSelection>());
        qDebug() << "Now we got " << extraSelections().size() << " extraSelections";
        mBlockStartCursor = QTextCursor();
    }
}

void CodeEditor::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls()) {
        e->ignore(); // paste to parent widget
    } else {
        QPlainTextEdit::dragEnterEvent(e);
    }
}

bool CodeEditor::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QString hint;
        QTextCursor cur(document());
        emit getHintForPos(this, helpEvent->pos(), hint, cur);
        if (hint != toolTip()) {
            setToolTip(hint);
        }
        if (!hint.isEmpty()) {
            qDebug() << "Event:  " << helpEvent->pos()
                     << "Cursor: " << cursorRect(cur).bottomLeft()
                     << "Viewport: " << viewport()->mapTo(this,cursorRect(cur).bottomLeft());
                        ;
            QToolTip::showText(viewport()->mapToGlobal(cursorRect(cur).bottomLeft()), hint);
        }

        else
            QToolTip::hideText();
        return true;
    }
    return QPlainTextEdit::event(event);
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

//    setExtraSelections(extraSelections);
}



void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());
    int markFrom = (mBlockStartCursor.isNull() ? textCursor() : mBlockStartCursor).blockNumber();
    int markTo = (mBlockLastCursor.isNull() ? textCursor() : mBlockLastCursor).blockNumber();
    if (markFrom > markTo) qSwap(markFrom, markTo);

    QRect paintRect(event->rect());
    // TODO(JM) fit paintRect to real height in case of wrapped lines (somehow it is clipped though)
    painter.fillRect(paintRect, QColor(245,245,245));

    QRect markRect(paintRect.left(), top, paintRect.width(), static_cast<int>(blockBoundingRect(block).height())+1);
    while (block.isValid()) { // && top <= paintRect.bottom()) {
        if (block.isVisible()) { // && bottom >= paintRect.top()) {
            bool mark = blockNumber >= markFrom && blockNumber <= markTo;
            if (mark) {
                markRect.moveTop(top);
                markRect.setHeight(bottom-top);
                painter.fillRect(markRect, QColor(225,255,235));
            }
            QString number = QString::number(blockNumber + 1);
            QFont f = painter.font();
            f.setBold(mark);
            painter.setFont(f);
            painter.setPen(mark ? Qt::black : Qt::gray);
            painter.drawText(0, (top+bottom-fontMetrics().height())/2, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

} // namespace studio
} // namespace gams
