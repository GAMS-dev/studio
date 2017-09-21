/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include "codeeditor.h"

namespace gams {
namespace ide {

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, &CodeEditor::updateBlockSelection, this, &CodeEditor::onUpdateBlockSelection, Qt::QueuedConnection);
    connect(this, &CodeEditor::updateBlockEdit, this, &CodeEditor::onUpdateBlockEdit, Qt::QueuedConnection);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
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
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

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
    // TODO(JM) get definition from ide key-config
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
    painter.fillRect(event->rect(), QColor(245,245,245));


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    int markFrom = (mBlockStartCursor.isNull() ? textCursor() : mBlockStartCursor).blockNumber();
    int markTo = (mBlockLastCursor.isNull() ? textCursor() : mBlockLastCursor).blockNumber();
    if (markFrom > markTo) qSwap(markFrom, markTo);

    QRect markRect(event->rect().left(), top, event->rect().width(), (int) blockBoundingRect(block).height()+1);
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            bool mark = blockNumber >= markFrom && blockNumber <= markTo;
            if (mark) {
                markRect.moveTop(top);
                painter.fillRect(markRect, QColor(225,255,235));
            }
            QString number = QString::number(blockNumber + 1);
            QFont f = painter.font();
            f.setBold(mark);
            painter.setFont(f);
            painter.setPen(mark ? Qt::black : Qt::gray);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

} // namespace ide
} // namespace gams
