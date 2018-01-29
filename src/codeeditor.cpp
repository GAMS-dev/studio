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
#include "studiosettings.h"
#include "searchwidget.h"
#include "exception.h"
#include "logger.h"
#include "syntax.h"
#include "keys.h"

namespace gams {
namespace studio {

inline const KeySeqList &hotkey(Hotkey _hotkey) { return Keys::instance().keySequence(_hotkey); }

CodeEditor::CodeEditor(StudioSettings *settings, QWidget *parent) : QPlainTextEdit(parent), mSettings(settings)
{
    mLineNumberArea = new LineNumberArea(this);
    mLineNumberArea->setMouseTracking(true);
    mBlinkBlockEdit.setInterval(500);

    connect(&mBlinkBlockEdit, &QTimer::timeout, this, &CodeEditor::blockEditBlink);
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    if(mSettings->lineWrapEditor())
        setLineWrapMode(QPlainTextEdit::WidgetWidth);
    else
        setLineWrapMode(QPlainTextEdit::NoWrap);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 0;

    if(mSettings->showLineNr())
        space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    bool marksEmpty = true;
    emit requestMarksEmpty(&marksEmpty);
    space += (marksEmpty ? 0 : iconSize());

    return space;
}

int CodeEditor::iconSize()
{
    return fontMetrics().height()-3;
}

LineNumberArea* CodeEditor::lineNumberArea()
{
    return mLineNumberArea;
}

QMimeData* CodeEditor::createMimeDataFromSelection() const
{
    QMimeData* mimeData = new QMimeData();
    QTextCursor c = textCursor();
    QString plainTextStr = c.selection().toPlainText();
    mimeData->setText( plainTextStr );

    return mimeData;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        mLineNumberArea->scroll(0, dy);
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
            if (blockBounds.bottom() >= rect.bottom())
                break;
            b = b.next();
        }
        mLineNumberArea->update(0, top, mLineNumberArea->width(), bottom-top);
    }

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::blockEditBlink()
{
    if (mBlockEdit) mBlockEdit->refreshCursors();
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent* e)
{
    if (!mBlockEdit && e == Hotkey::BlockEditStart) {
        QTextCursor c = textCursor();
        startBlockEdit(c.blockNumber(), c.columnNumber());
    }

    if (mBlockEdit) {
        if (e == Hotkey::BlockEditEnd || e == Hotkey::Undo || e == Hotkey::Redo) {
            endBlockEdit();
        } else {
            mBlockEdit->keyPressEvent(e);
            return;
        }
    }

    if (!isReadOnly() && e->key() == Hotkey::NewLine) {
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        cursor.insertText("\n");
        if (cursor.block().previous().isValid())
            truncate(cursor.block().previous());
        adjustIndent(cursor);
        cursor.endEditBlock();
        setTextCursor(cursor);
        e->accept();
        return;
    }

    if (e == Hotkey::DuplicateLine) {
        duplicateLine();
        e->accept();
        return;
    }


    QPlainTextEdit::keyPressEvent(e);
}

void CodeEditor::privateKeyPressEvent(QKeyEvent* e)
{
    QPlainTextEdit::keyPressEvent(e);
}

void CodeEditor::keyReleaseEvent(QKeyEvent* e)
{
    if (isReadOnly()) {
        QPlainTextEdit::keyReleaseEvent(e);
        return;
    }
    if (mBlockEdit) {
        mBlockEdit->keyReleaseEvent(e);
        return;
    }
    // return pressed, check if current block consists of whitespaces only
    if (!isReadOnly() && e->key() == Hotkey::NewLine) {
        e->accept();
        return;
    } else {
        QPlainTextEdit::keyReleaseEvent(e);
    }
}

void CodeEditor::adjustIndent(QTextCursor cursor)
{
    QRegularExpression rex("^(\\s*).*$");
    QRegularExpressionMatch match = rex.match(cursor.block().text());
    if (match.hasMatch()) {
        QTextBlock prev = cursor.block().previous();
        QRegularExpression pRex("^((\\s+)|([^\\s]+))");
        QRegularExpressionMatch pMatch = pRex.match(prev.text());
        while (true) {
            if (pMatch.hasMatch()) {
                if (pMatch.capturedLength(2) < 1)
                    break;
                QString spaces = pMatch.captured(2);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, match.capturedLength(1));
                cursor.removeSelectedText();
                cursor.insertText(spaces);
                setTextCursor(cursor);
                break;
            }
            prev = prev.previous();
            if (!prev.isValid() || prev.blockNumber() < cursor.blockNumber()-50)
                break;
            pMatch = pRex.match(prev.text());
        }
    }
}


void CodeEditor::truncate(QTextBlock block)
{
    QRegularExpression pRex("(^.*[^\\s]|^)(\\s+)$");
    QRegularExpressionMatch match = pRex.match(block.text());
    if (match.hasMatch()) {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, match.capturedLength(2));
        cursor.removeSelectedText();
    }
}

int CodeEditor::textCursorColumn(QPoint mousePos)
{
    QTextCursor cursor = cursorForPosition(mousePos);
    int col = cursor.columnNumber();
    int addX = mousePos.x()-cursorRect(cursor).right();
    if (addX > 0) {
        QFontMetrics metric(font());
        col += addX / metric.width(' ');
    }
    return col;
}

void CodeEditor::mousePressEvent(QMouseEvent* e)
{
    if (e->modifiers() & Qt::AltModifier) {
        QTextCursor cursor = cursorForPosition(e->pos());
        if (e->modifiers() == Qt::AltModifier) {
            if (mBlockEdit) endBlockEdit();
            startBlockEdit(cursor.blockNumber(), textCursorColumn(e->pos()));
        } else if (e->modifiers() == (Qt::AltModifier | Qt::ShiftModifier)) {
            if (mBlockEdit) endBlockEdit();
            startBlockEdit(textCursor().blockNumber(), textCursor().columnNumber());
        }
        if (mBlockEdit) {
            mBlockEdit->selectTo(cursor.blockNumber(), textCursorColumn(e->pos()));
        } else {
            mDragStart = e->pos();
            startBlockEdit(cursor.blockNumber(), textCursorColumn(e->pos()));
        }
    } else {
        if (mBlockEdit) {
            endBlockEdit();
        } else {
        }
        QPlainTextEdit::mousePressEvent(e);
    }
}

void CodeEditor::mouseMoveEvent(QMouseEvent* e)
{
    if (mBlockEdit) {
        if ((e->buttons() & Qt::LeftButton) && (e->modifiers() & Qt::AltModifier)) {
            mBlockEdit->selectTo(cursorForPosition(e->pos()).blockNumber(), textCursorColumn(e->pos()));
        }
    } else
        QPlainTextEdit::mouseMoveEvent(e);
}

void CodeEditor::mouseReleaseEvent(QMouseEvent* e)
{
//    if (!mBlockEdit)
        QPlainTextEdit::mouseReleaseEvent(e);
    mDragStart = QPoint();
}

void CodeEditor::wheelEvent(QWheelEvent *e) {
    if (e->modifiers() & Qt::ControlModifier) {
        const int delta = e->delta();
        if (delta < 0)
            zoomOut();
        else if (delta > 0)
            zoomIn();
        return;
    }
    QPlainTextEdit::wheelEvent(e);
}

void CodeEditor::paintEvent(QPaintEvent* e)
{
    int cw = mBlockEdit ? 0 : 2;
    if (cursorWidth()!=cw) setCursorWidth(cw);
    QPlainTextEdit::paintEvent(e);
    if (mBlockEdit) {
        mBlockEdit->drawCursor(e);
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

void CodeEditor::duplicateLine()
{
    int blockNr = textCursor().blockNumber();
    int colNr = textCursor().columnNumber();
    QTextBlock blockFound = document()->findBlockByNumber(blockNr);
    QTextCursor t(blockFound);
    t.beginEditBlock();
    t.movePosition(QTextCursor::NextBlock);
    t.insertText(blockFound.text());
    t.insertBlock();
    moveCursor(QTextCursor::NextBlock);

    for(int i = 0; i < colNr; i++)
        moveCursor(QTextCursor::NextCharacter);

    t.endEditBlock();
}

void CodeEditor::startBlockEdit(int blockNr, int colNr)
{
    if (mBlockEdit) endBlockEdit();
    mBlockEdit = new BlockEdit(this, blockNr, colNr);
    mBlockEdit->startCursorTimer();
    updateLineNumberAreaWidth(0);
}

void CodeEditor::endBlockEdit()
{
    // TODO(JM) remove BlockEdit's selection and place cursor
    mBlockEdit->stopCursorTimer();
    mBlockEdit->adjustCursor();
    delete mBlockEdit;
    mBlockEdit = nullptr;
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
// _CRT_SECURE_NO_WARNINGS


void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(mLineNumberArea);
    QHash<int, TextMark*> textMarks;
    emit requestMarkHash(&textMarks);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());
    int markFrom = textCursor().blockNumber();
    int markTo = textCursor().blockNumber();
    if (markFrom > markTo) qSwap(markFrom, markTo);

    QRect paintRect(event->rect());
    painter.fillRect(paintRect, QColor(245,245,245));

    QRect markRect(paintRect.left(), top, paintRect.width(), static_cast<int>(blockBoundingRect(block).height())+1);
    while (block.isValid() && top <= paintRect.bottom()) {
        if (block.isVisible() && bottom >= paintRect.top()) {
            bool mark = mBlockEdit ? mBlockEdit->hasBlock(block.blockNumber())
                                   : blockNumber >= markFrom && blockNumber <= markTo;
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
            int realtop = top; // (top+bottom-fontMetrics().height())/2;

            if(mSettings->showLineNr())
                painter.drawText(0, realtop, mLineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);

            if (textMarks.contains(blockNumber)) {
                int iTop = (2+top+bottom-iconSize())/2;
                painter.drawPixmap(1, iTop, textMarks.value(blockNumber)->icon().pixmap(QSize(iconSize(),iconSize())));
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

CodeEditor::BlockEdit::BlockEdit(CodeEditor* edit, int blockNr, int colNr)
    : mEdit(edit)
{
    if (!edit) FATAL() << "BlockEdit needs a valid editor";
    DEB() << "blockEdit START";
    mStartLine = blockNr;
    mCurrentLine = blockNr;
    mColumn = colNr;
    mSize = 0;
}

CodeEditor::BlockEdit::~BlockEdit()
{
    mEdit->setExtraSelections(QList<QTextEdit::ExtraSelection>());
    DEB() << "blockEdit END";
}

void CodeEditor::BlockEdit::selectTo(int blockNr, int colNr)
{
    mCurrentLine = blockNr;
    mSize = colNr - mColumn;
    updateExtraSelections();
    startCursorTimer();
}

void CodeEditor::BlockEdit::keyPressEvent(QKeyEvent* e)
{
    QSet<int> moveKeys;
    moveKeys << Qt::Key_Home << Qt::Key_End << Qt::Key_Down << Qt::Key_Up << Qt::Key_Left << Qt::Key_Right
             << Qt::Key_PageUp << Qt::Key_PageDown;
    if (moveKeys.contains(e->key())) {
        if (e->key() == Qt::Key_Right) mSize++;
        if (e->key() == Qt::Key_Left && mColumn+mSize > 0) mSize--;
        if (e->key() == Qt::Key_Down && mCurrentLine < mEdit->document()->blockCount()-1) mCurrentLine++;
        if (e->key() == Qt::Key_Up && mCurrentLine > 0) mCurrentLine--;
        QTextBlock block = mEdit->document()->findBlockByNumber(mCurrentLine);
        QTextCursor cursor(block);
        if (block.length() > mColumn+mSize)
            cursor.setPosition(block.position()+mColumn+mSize);
        else
            cursor.setPosition(block.position()+block.length()-1);
        mEdit->setTextCursor(cursor);
        updateExtraSelections();
        startCursorTimer();
    } else if (e == Hotkey::Paste) {
        QString text = QGuiApplication::clipboard()->mimeData()->text();
        if (!text.isEmpty()) replaceBlockText(text);
    } else if (e == Hotkey::Cut || e == Hotkey::Copy) {
        // TODO(JM) copy selected text to clipboard
        if (e == Hotkey::Cut) replaceBlockText("");
    } else if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
        if (!mSize) mSize = (e->key() == Qt::Key_Backspace) ? -1 : 1;
        replaceBlockText("");
        // TODO(JM) process deletion
    } else if (e->text().length()) {
        replaceBlockText(e->text());
    }
}

void CodeEditor::BlockEdit::keyReleaseEvent(QKeyEvent* e)
{
    QSet<int> moveKeys;
    moveKeys << Qt::Key_Home << Qt::Key_End << Qt::Key_Down << Qt::Key_Up << Qt::Key_Left << Qt::Key_Right
             << Qt::Key_PageUp << Qt::Key_PageDown;
}

void CodeEditor::BlockEdit::startCursorTimer()
{
    mEdit->mBlinkBlockEdit.start();
    mBlinkStateHidden = true;
    mEdit->lineNumberArea()->update(mEdit->lineNumberArea()->visibleRegion());
    refreshCursors();
}

void CodeEditor::BlockEdit::stopCursorTimer()
{
    mEdit->mBlinkBlockEdit.stop();
    mBlinkStateHidden = true;
    mEdit->lineNumberArea()->update(mEdit->lineNumberArea()->visibleRegion());
    refreshCursors();
}

void CodeEditor::BlockEdit::refreshCursors()
{
    // TODO(JM) generate drawCursor-event for every line
    mBlinkStateHidden = !mBlinkStateHidden;
    mEdit->viewport()->update(mEdit->viewport()->visibleRegion());
}

void CodeEditor::BlockEdit::drawCursor(QPaintEvent *e)
{
    QPainter painter(mEdit->viewport());
    QPointF offset(mEdit->contentOffset()); //
    QRect evRect = e->rect();
    bool editable = !mEdit->isReadOnly();
    painter.setClipRect(evRect);

    QAbstractTextDocumentLayout::PaintContext context = mEdit->getPaintContext();

    QTextBlock block = mEdit->firstVisibleBlock();
    QTextCursor cursor(mEdit->document());
    int cursorColumn = mColumn+mSize;
    QFontMetrics metric(mEdit->font());
    int spaceWidth = metric.width(' ');

    while (block.isValid()) {
        QRectF blockRect = mEdit->blockBoundingRect(block).translated(offset);
        QTextLayout *layout = block.layout();

        if (!hasBlock(block.blockNumber()) || !block.isVisible()) {
            offset.ry() += blockRect.height();
            block = block.next();
            continue;
        }

        // draw extended extra-selection for lines past line-end
        int beyondEnd = qMax(mColumn, mColumn+mSize);
        if (beyondEnd >= block.length()) {
            cursor.setPosition(block.position()+block.length()-1);
            // we have to draw selection beyond the line-end
            int beyondStart = qMax(block.length()-1, qMin(mColumn, mColumn+mSize));
            QRect selRect = mEdit->cursorRect(cursor);
            if (block.length() <= beyondStart) {
                selRect.translate((beyondStart-block.length()+1) * spaceWidth, 0);
            }
            selRect.setWidth((beyondEnd-beyondStart) * spaceWidth);
            painter.fillRect(selRect, QBrush(Qt::cyan));
        }

        if (mBlinkStateHidden) {
            offset.ry() += blockRect.height();
            block = block.next();
            continue;
        }

        cursor.setPosition(block.position()+qMin(block.length()-1, cursorColumn));
        QRect cursorRect = mEdit->cursorRect(cursor);
        if (block.length() <= cursorColumn) {
            cursorRect.translate((cursorColumn-block.length()+1) * spaceWidth, 0);
        }
        cursorRect.setWidth(2);

        if (cursorRect.bottom() >= evRect.top() && cursorRect.top() <= evRect.bottom()) {
            int blpos = block.position();
            bool drawCursor = ((editable || (mEdit->textInteractionFlags() & Qt::TextSelectableByKeyboard)));
            if (drawCursor /*|| (editable && context.cursorPosition < -1
                               && !layout->preeditAreaText().isEmpty())*/) {
                int cpos = context.cursorPosition+1;
                if (cpos < -1)
                    cpos = layout->preeditAreaPosition() - (cpos + 2);
                else
                    cpos -= blpos;


                bool toggleAntialiasing = !(painter.renderHints() & QPainter::Antialiasing)
                                          && (painter.transform().type() > QTransform::TxTranslate);
                if (toggleAntialiasing)
                    painter.setRenderHint(QPainter::Antialiasing);
                QPainter::CompositionMode origCompositionMode = painter.compositionMode();
                if (painter.paintEngine()->hasFeature(QPaintEngine::RasterOpModes))
                    painter.setCompositionMode(QPainter::RasterOp_NotDestination);
                painter.fillRect(cursorRect, painter.pen().brush());
                painter.setCompositionMode(origCompositionMode);
                if (toggleAntialiasing)
                    painter.setRenderHint(QPainter::Antialiasing, false);

            }
        }

        offset.ry() += blockRect.height();
        if (offset.y() > mEdit->viewport()->rect().height())
            break;
        block = block.next();
    }

}

void CodeEditor::BlockEdit::updateExtraSelections()
{
    QList<QTextEdit::ExtraSelection> selections;
    QTextCursor cursor(mEdit->document());
    for (int lineNr = qMin(mStartLine, mCurrentLine); lineNr <= qMax(mStartLine, mCurrentLine); ++lineNr) {
        QTextBlock block = mEdit->document()->findBlockByNumber(lineNr);
        QTextEdit::ExtraSelection select;
        // TODO(JM) Later, this color has to be retrieved from StyleSheet-definitions
        select.format.setBackground(Qt::cyan);

        int start = qMin(block.length()-1, qMin(mColumn, mColumn+mSize));
        int end = qMin(block.length()-1, qMax(mColumn, mColumn+mSize));
        cursor.setPosition(block.position()+start);
        if (end>start) cursor.setPosition(block.position()+end, QTextCursor::KeepAnchor);
        select.cursor = cursor;
        selections << select;
    }
    mEdit->setExtraSelections(selections);
}

void CodeEditor::BlockEdit::adjustCursor()
{
    // This restores the hidden column-information when moving vertically with the cursor
    QTextBlock block = mEdit->document()->findBlockByNumber(qMin(mStartLine, mCurrentLine));
    int len = 0;
    QTextBlock searchBlock = block;
    while (block.blockNumber() <= qMax(mStartLine, mCurrentLine)) {
        if (block.length() > len) {
            searchBlock = block;
            len = block.length();
        }
        if (len >= mColumn+mSize) {
            searchBlock = block;
            break;
        }
        block = block.next();
    }
    QTextCursor cursor(searchBlock);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, qMin(mColumn, searchBlock.length()-1));
    mEdit->setTextCursor(cursor);
    QTextCursor::MoveOperation moveOp = searchBlock.blockNumber() < mCurrentLine ? QTextCursor::Down : QTextCursor::Up;
    for (int i = 0; i < searchBlock.blockNumber()-mCurrentLine; ++i) {
        mEdit->moveCursor(moveOp);
    }
}

void CodeEditor::BlockEdit::replaceBlockText(QString text)
{
    QTextBlock block = mEdit->document()->findBlockByNumber(qMin(mCurrentLine, mStartLine));
    int fromCol = qMin(mColumn, mColumn+mSize);
    int toCol = qMax(mColumn, mColumn+mSize);
    QTextCursor cursor(mEdit->document());

    if (text.length() != 1 || mBeginBlock) {
        mBeginBlock = false;
        cursor.beginEditBlock();
    } else
        cursor.joinPreviousEditBlock();


    while (block.blockNumber() <= qMax(mCurrentLine, mStartLine)) {
        QString addText = text;
        int offsetFromEnd = fromCol - block.length()+1;
        if (offsetFromEnd > 0 && !text.isEmpty()) {
            // line ends before start of mark -> calc additional spaces
            cursor.setPosition(block.position()+block.length()-1);
            QString s(offsetFromEnd, ' ');
            addText = s+text;
        } else if (mSize) {
            // block-edit contains marking -> remove to end of block/line
            int pos = block.position()+fromCol;
            int rmSize = block.position()+qMin(block.length()-1, toCol) - pos;
            cursor.setPosition(pos);
            if (rmSize) {
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, rmSize);
                cursor.removeSelectedText();
            }
        } else {
            cursor.setPosition(block.position() + qMin(block.length()-1, fromCol));
        }
        if (!addText.isEmpty()) cursor.insertText(addText);
        block = block.next();
    }
    cursor.endEditBlock();
    mColumn += text.length();
    mSize = 0;
}


} // namespace studio
} // namespace gams
