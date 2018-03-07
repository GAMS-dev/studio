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
#include <QtWidgets>
#include "editors/codeeditor.h"
#include "studiosettings.h"
#include "searchwidget.h"
#include "exception.h"
#include "logger.h"
#include "syntax.h"
#include "keys.h"

namespace gams {
namespace studio {

inline const KeySeqList &hotkey(Hotkey _hotkey) { return Keys::instance().keySequence(_hotkey); }

CodeEditor::CodeEditor(StudioSettings *settings, QWidget *parent)
    : AbstractEditor(settings, parent)
{
    mLineNumberArea = new LineNumberArea(this);
    mLineNumberArea->setMouseTracking(true);
    mBlinkBlockEdit.setInterval(500);

    connect(&mBlinkBlockEdit, &QTimer::timeout, this, &CodeEditor::blockEditBlink);
    connect(&mWordDelay, &QTimer::timeout, this, &CodeEditor::updateExtraSelections);
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::recalcExtraSelections);
    connect(this, &CodeEditor::textChanged, this, &CodeEditor::recalcExtraSelections);
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &CodeEditor::updateExtraSelections);

    updateLineNumberAreaWidth(0);
    recalcExtraSelections();
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

void CodeEditor::clearSelection()
{
    if (isReadOnly()) return;
    if (mBlockEdit && !mBlockEdit->blockText().isEmpty()) {
        mBlockEdit->replaceBlockText(QStringList()<<QString());
    } else {
        textCursor().clearSelection();
    }
}

void CodeEditor::cutSelection()
{
    if (mBlockEdit && !mBlockEdit->blockText().isEmpty()) {
        mBlockEdit->selectionToClipboard();
        mBlockEdit->replaceBlockText(QStringList()<<QString());
    } else {
        cut();
    }
}

void CodeEditor::copySelection()
{
    if (mBlockEdit && !mBlockEdit->blockText().isEmpty()) {
        mBlockEdit->selectionToClipboard();
    } else {
        copy();
    }
}

void CodeEditor::pasteClipboard()
{
    bool isBlock;
    QStringList texts = clipboard(&isBlock);
    if (!mBlockEdit) {
        if (isBlock) {
            QTextCursor c = textCursor();
            QTextCursor anc = c;
            anc.setPosition(c.anchor());
            startBlockEdit(anc.blockNumber(), anc.columnNumber());
            mBlockEdit->selectTo(c.blockNumber(), c.columnNumber());
            mBlockEdit->replaceBlockText(texts);
        } else {
            paste();
        }
    } else {
        mBlockEdit->replaceBlockText(texts);
    }
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    updateExtraSelections();
}

void CodeEditor::keyPressEvent(QKeyEvent* e)
{
    if (!mBlockEdit && e == Hotkey::BlockEditStart) {
        QTextCursor c = textCursor();
        QTextCursor anc = c;
        anc.setPosition(c.anchor());
        startBlockEdit(anc.blockNumber(), anc.columnNumber());
    }

    if (mBlockEdit) {
        if (e->key() == Hotkey::NewLine) {
            endBlockEdit();
            return;
        }
        if (e == Hotkey::BlockEditEnd || e == Hotkey::Undo || e == Hotkey::Redo) {
            endBlockEdit();
        } else {
            mBlockEdit->keyPressEvent(e);
            return;
        }
    }

    if (!isReadOnly()) {
        if (e->key() == Hotkey::NewLine) {
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
    }

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
    // return pressed, if current block consists of whitespaces only: ignore here
    if (!isReadOnly() && e->key() == Hotkey::NewLine) {
        e->accept();
        return;
    } else {
        QPlainTextEdit::keyReleaseEvent(e);
    }
}

void CodeEditor::adjustIndent(QTextCursor cursor)
{
    if (!mSettings->autoIndent()) return;

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
        QTextCursor anchor = textCursor();
        anchor.setPosition(anchor.anchor());
        if (e->modifiers() == Qt::AltModifier) {
            if (mBlockEdit) endBlockEdit();
            startBlockEdit(cursor.blockNumber(), textCursorColumn(e->pos()));
        } else if (e->modifiers() == (Qt::AltModifier | Qt::ShiftModifier)) {
            if (mBlockEdit) mBlockEdit->selectTo(cursor.blockNumber(), textCursorColumn(e->pos()));
            else startBlockEdit(anchor.blockNumber(), anchor.columnNumber());
        }
        if (mBlockEdit) {
            mBlockEdit->selectTo(cursor.blockNumber(), textCursorColumn(e->pos()));
        }
    } else {
        if (mBlockEdit && (e->modifiers() || e->buttons() != Qt::RightButton))
            endBlockEdit();
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

void CodeEditor::wheelEvent(QWheelEvent *e) {
    if (e->modifiers() & Qt::ControlModifier) {
        const int delta = e->delta();
        if (delta < 0)
            zoomOut();
        else if (delta > 0)
            zoomIn();
        updateTabSize();
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
        mBlockEdit->paintEvent(e);
    }
}

void CodeEditor::contextMenuEvent(QContextMenuEvent* e)
{
    QMenu *menu = createStandardContextMenu();
    bool hasBlockSelection = mBlockEdit && !mBlockEdit->blockText().isEmpty();
    QAction *lastAct = nullptr;
    for (int i = menu->actions().count()-1; i >= 0; --i) {
        QAction *act = menu->actions().at(i);
        if (act->objectName() == "select-all" && mBlockEdit) {
            act->setEnabled(false);
        } else if (act->objectName() == "edit-paste" && act->isEnabled()) {
            menu->removeAction(act);
            act->disconnect();
            connect(act, &QAction::triggered, this, &CodeEditor::pasteClipboard);
            menu->insertAction(lastAct, act);
        } else if (hasBlockSelection) {
            if (act->objectName() == "edit-cut") {
                menu->removeAction(act);
                act->disconnect();
                act->setEnabled(true);
                connect(act, &QAction::triggered, this, &CodeEditor::cutSelection);
                menu->insertAction(lastAct, act);
            } else if (act->objectName() == "edit-copy") {
                menu->removeAction(act);
                act->disconnect();
                act->setEnabled(true);
                connect(act, &QAction::triggered, this, &CodeEditor::copySelection);
                menu->insertAction(lastAct, act);
            } else if (act->objectName() == "edit-delete") {
                menu->removeAction(act);
                act->disconnect();
                act->setEnabled(true);
                connect(act, &QAction::triggered, this, &CodeEditor::clearSelection);
                menu->insertAction(lastAct, act);
            }
        }
        lastAct = act;
    }
    menu->exec(e->globalPos());
    delete menu;
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
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.insertText(cursor.block().text()+'\n');
    cursor.endEditBlock();
}

void CodeEditor::removeLine()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();
}

int CodeEditor::minIndentCount(int fromLine, int toLine)
{
    QTextCursor cursor = textCursor();
    QTextBlock block = (fromLine < 0) ? document()->findBlock(cursor.anchor()) : document()->findBlockByNumber(fromLine);
    QTextBlock last = (toLine < 0) ? document()->findBlock(cursor.position()) : document()->findBlockByNumber(toLine);
    if (block.blockNumber() > last.blockNumber()) qSwap(block, last);
    int res = block.text().length();
    QRegularExpression rex("^(\\s*)");
    while (true) {
        QRegularExpressionMatch match = rex.match(block.text());
        if (res > match.capturedLength(1)) res = match.capturedLength(1);
        if (block == last) break;
        block = block.next();
    }
    return res;
}

int CodeEditor::indent(int size, int fromLine, int toLine)
{
    int res = 0;
    QTextCursor cursor = textCursor();
    QTextBlock block = (fromLine < 0) ? document()->findBlock(cursor.anchor()) : document()->findBlockByNumber(fromLine);
    QTextBlock last = (toLine < 0) ? document()->findBlock(cursor.position()) : document()->findBlockByNumber(toLine);
    if (block.blockNumber() > last.blockNumber()) qSwap(block, last);
    cursor.beginEditBlock();
    while (true) {
        QTextCursor editCursor(block);
        if (size > 0) {
            editCursor.insertText(QString(size, ' '));
            res = size;
        } else {
            int wchars = 0;
            for (int i = 0; i < -size; ++i) {
                if (block.text().startsWith(' ')) {
                    editCursor.deleteChar();
                    wchars--;
                }
            }
            if (wchars < res) res = wchars;
        }
        if (block == last) break;
        block = block.next();
    }
    cursor.endEditBlock();
    return res;
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
    mBlockEdit->stopCursorTimer();
    mBlockEdit->adjustCursor();
    delete mBlockEdit;
    mBlockEdit = nullptr;
}

void dumpClipboard()
{
    QClipboard *clip = QGuiApplication::clipboard();
    const QMimeData * mime = clip->mimeData();
    DEB() << "----------------- Clipboard --------------------" ;
    DEB() << "  C " << clip->ownsClipboard()<< "  F " << clip->ownsFindBuffer()<< "  S " << clip->ownsSelection();
    for (QString fmt: mime->formats()) {
        DEB() << "   -- " << fmt << "   L:" << mime->data(fmt).length()
              << "\n" << mime->data(fmt)
              << "\n" << QString(mime->data(fmt).toHex(' '));
    }
}

QStringList CodeEditor::clipboard(bool *isBlock)
{
//    dumpClipboard();
    QString mimes = "|" + QGuiApplication::clipboard()->mimeData()->formats().join("|") + "|";
    bool asBlock = (mimes.indexOf("MSDEVColumnSelect") >= 0) || (mimes.indexOf("Borland IDE Block Type") >= 0);
    QStringList texts = QGuiApplication::clipboard()->mimeData()->text().split("\n");
    if (texts.last().isEmpty()) texts.removeLast();
    if (!asBlock || texts.count() <= 1) {
        if (isBlock) *isBlock = false;
        texts = QStringList() << QGuiApplication::clipboard()->mimeData()->text();
    }
//    if (isBlock) *isBlock = true; // TODO: find better handling for this
    return texts;
}

CharType CodeEditor::charType(QChar c)
{
    switch (c.category()) {
    case QChar::Number_DecimalDigit:
        return CharType::Number;
    case QChar::Separator_Space:
    case QChar::Separator_Line:
    case QChar::Separator_Paragraph:
        return CharType::Seperator;
    case QChar::Letter_Uppercase:
        return CharType::LetterUCase;
    case QChar::Letter_Lowercase:
        return CharType::LetterLCase;
    case QChar::Punctuation_Dash:
    case QChar::Punctuation_Open:
    case QChar::Punctuation_Close:
    case QChar::Punctuation_InitialQuote:
    case QChar::Punctuation_FinalQuote:
    case QChar::Punctuation_Other:
    case QChar::Symbol_Math:
    case QChar::Symbol_Currency:
    case QChar::Symbol_Modifier:
    case QChar::Symbol_Other:
        return CharType::Punctuation;
    default:
        break;
    }
    return CharType::Other;
}

void CodeEditor::updateTabSize()
{
    QFontMetrics metric(font());
    setTabStopDistance(8*metric.width(' '));
}

CodeEditor::BlockEdit *CodeEditor::blockEdit() const
{
    return mBlockEdit;
}

AbstractEditor::EditorType CodeEditor::type()
{
    return EditorType::CodeEditor;
}

inline int findAlphaNum(QString text, int start, bool back)
{
    QChar c = ' ';
    int pos = (back && start == text.length()) ? start-1 : start;
    while (pos >= 0 && pos < text.length()) {
        c = text.at(pos);
        if (!c.isLetterOrNumber() && c != '_' && (pos != start || !back)) break;
        pos = pos + (back?-1:1);
    }
    pos = pos - (back?-1:1);
    if (pos == start) {
        c = (pos >= 0 && pos < text.length()) ? text.at(pos) : ' ';
        if (!c.isLetterOrNumber() && c != '_') return -1;
    }
    if (pos >= 0 && pos < text.length()) { // must not start with number
        c = text.at(pos);
        if (!c.isLetter() && c != '_') return -1;
    }
    return pos;
}

void CodeEditor::recalcExtraSelections()
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly() && !mBlockEdit) {
        extraSelCurrentLine(selections);

        mWordUnderCursor = "";
        QTextEdit::ExtraSelection selection;
        selection.cursor = textCursor();
        QString text = selection.cursor.block().text();
        int start = qMin(selection.cursor.position(), selection.cursor.anchor()) - selection.cursor.block().position();
        int from = findAlphaNum(text, start, true);
        int to = findAlphaNum(text, from, false);
        if (from >= 0 && from <= to) {
            if (!textCursor().hasSelection() || text.mid(from, to-from+1) == textCursor().selectedText())
                mWordUnderCursor = text.mid(from, to-from+1);
        }
        mWordDelay.start(500);
    }
    extraSelBlockEdit(selections);
    setExtraSelections(selections);
}

void CodeEditor::updateExtraSelections()
{
    QList<QTextEdit::ExtraSelection> selections;
    extraSelCurrentLine(selections);
    if (!mBlockEdit) extraSelCurrentWord(selections);
    extraSelBlockEdit(selections);
    setExtraSelections(selections);
}

void CodeEditor::extraSelBlockEdit(QList<QTextEdit::ExtraSelection>& selections)
{
    if (mBlockEdit) {
        selections.append(mBlockEdit->extraSelections());
    }
}

void CodeEditor::extraSelCurrentLine(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!mSettings->highlightCurrentLine()) return;

    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::cyan).lighter(190);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.movePosition(QTextCursor::StartOfBlock);
    selection.cursor.clearSelection();
    selections.append(selection);
}

void CodeEditor::extraSelCurrentWord(QList<QTextEdit::ExtraSelection> &selections)
{
    if (!mSettings->wordUnderCursor()) return;
    if (!mWordUnderCursor.isEmpty()) {
        QTextBlock block = firstVisibleBlock();
        QRegularExpression rex(QString("(?i)(^|[^\\w]|-)(%1)($|[^\\w]|-)").arg(mWordUnderCursor));
        QRegularExpressionMatch match;
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        while (block.isValid() && top < viewport()->height()) {
            int i = 0;
            while (true) {
                i = block.text().indexOf(rex, i, &match);
                if (i < 0) break;
                QTextEdit::ExtraSelection selection;
                selection.cursor = textCursor();
                selection.cursor.setPosition(block.position()+match.capturedStart(2));
                selection.cursor.setPosition(block.position()+match.capturedEnd(2), QTextCursor::KeepAnchor);
//                QPen outlinePen( Qt::lightGray, 1);
//                selection.format.setProperty(QTextFormat::OutlinePen, outlinePen);
                QColor wordColor = QColor(Qt::lightGray).lighter(115);
                selection.format.setBackground(wordColor);
                selections << selection;
                i += match.capturedLength(1) + match.capturedLength(2);
            }
            top += qRound(blockBoundingRect(block).height());
            block = block.next();
        }
    }
}

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
            QFont f = font();
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
    mStartLine = blockNr;
    mCurrentLine = blockNr;
    mColumn = colNr;
    mSize = 0;
}

CodeEditor::BlockEdit::~BlockEdit()
{
    mSelections.clear();
    mEdit->updateExtraSelections();
}

void CodeEditor::BlockEdit::selectTo(int blockNr, int colNr)
{
    mCurrentLine = blockNr;
    mSize = colNr - mColumn;
    updateExtraSelections();
    startCursorTimer();
}

void CodeEditor::BlockEdit::selectToEnd()
{
    int end = 0;
    for (QTextBlock block = mEdit->document()->findBlockByNumber(qMin(mStartLine, mCurrentLine))
         ; block.blockNumber() <= qMax(mStartLine, mCurrentLine); block=block.next()) {
        if (end < block.length()-1) end = block.length()-1;
    }
    mSize = end - mColumn;
}

QString CodeEditor::BlockEdit::blockText()
{
    QString res = "";
    if (!mSize) return res;
    QTextBlock block = mEdit->document()->findBlockByNumber(qMin(mStartLine, mCurrentLine));
    int from = qMin(mColumn, mColumn+mSize);
    int to = qMax(mColumn, mColumn+mSize);
    for (int i = qMin(mStartLine, mCurrentLine); i <= qMax(mStartLine, mCurrentLine); ++i) {
        QString text = block.text();
        if (text.length()-1 < to) text.append(QString(qAbs(mSize), ' '));
        res.append(text.mid(from, to-from)+"\n");
        block = block.next();
    }
    return res;
}

void CodeEditor::BlockEdit::selectionToClipboard()
{
    QMimeData *mime = new QMimeData();
    mime->setText(blockText());
    mime->setData("application/x-qt-windows-mime;value=\"MSDEVColumnSelect\"", QByteArray());
    mime->setData("application/x-qt-windows-mime;value=\"Borland IDE Block Type\"", QByteArray(1,char(2)));
    QApplication::clipboard()->setMimeData(mime);
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
        if (e->key() == Qt::Key_Home) mSize = -mColumn;
        if (e->key() == Qt::Key_End) selectToEnd();
        QTextBlock block = mEdit->document()->findBlockByNumber(mCurrentLine);
        QTextCursor cursor(block);
        if (block.length() > mColumn+mSize)
            cursor.setPosition(block.position()+mColumn+mSize);
        else
            cursor.setPosition(block.position()+block.length()-1);
        mEdit->setTextCursor(cursor);
        updateExtraSelections();
    } else if (e == Hotkey::Paste) {
        QStringList texts = mEdit->clipboard();
        if (texts.count() > 1 || (texts.count() == 1 && texts.first().length() > 0))
            replaceBlockText(texts);
    } else if (e == Hotkey::Indent) {
        mColumn += mEdit->indent(mEdit->mSettings->tabSize(), mStartLine, mCurrentLine);
    } else if (e == Hotkey::Outdent) {
        int minWhiteCount = mEdit->minIndentCount(mStartLine, mCurrentLine);
        if (minWhiteCount)
            mColumn += mEdit->indent(qMax(-minWhiteCount, -mEdit->mSettings->tabSize()), mStartLine, mCurrentLine);
    } else if (e == Hotkey::Cut || e == Hotkey::Copy) {
        // TODO(JM) copy selected text to clipboard
//    X    selectionToClipboard();
//    X   if (e == Hotkey::Cut) replaceBlockText("");
    } else if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
        if (!mSize && mColumn) mSize = (e->key() == Qt::Key_Backspace) ? -1 : 1;
        replaceBlockText("");
    } else if (e == Hotkey::DuplicateLine) {
        return;
    } else if (e->text().length()) {
        replaceBlockText(e->text());
    }
    startCursorTimer();
}

void CodeEditor::BlockEdit::keyReleaseEvent(QKeyEvent* e)
{
    Q_UNUSED(e)
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

void CodeEditor::BlockEdit::paintEvent(QPaintEvent *e)
{
    QPainter painter(mEdit->viewport());
    QPointF offset(mEdit->contentOffset()); //
    QRect evRect = e->rect();
    bool editable = !mEdit->isReadOnly();
    painter.setClipRect(evRect);
    int cursorColumn = mColumn+mSize;
    QFontMetrics metric(mEdit->font());
    double spaceWidth = metric.width(QString(100,' ')) / 100.0;
    QTextBlock block = mEdit->firstVisibleBlock();
    QTextCursor cursor(block);
    cursor.setPosition(block.position()+block.length()-1);
//    qreal cursorOffset = 0; //mEdit->cursorRect(cursor).left()-block.layout()->minimumWidth();

    while (block.isValid()) {
        QRectF blockRect = mEdit->blockBoundingRect(block).translated(offset);
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
            QRectF selRect = mEdit->cursorRect(cursor);
//            qreal x = block.layout()->minimumWidth()+cursorOffset;
//            selRect.setLeft(x);
            if (block.length() <= beyondStart)
                selRect.translate(((beyondStart-block.length()+1) * spaceWidth), 0);
            selRect.setWidth((beyondEnd-beyondStart) * spaceWidth);
            QColor beColor = QColor(Qt::cyan).lighter(150);
            painter.fillRect(selRect, QBrush(beColor));
        }

        if (mBlinkStateHidden) {
            offset.ry() += blockRect.height();
            block = block.next();
            continue;
        }

        cursor.setPosition(block.position()+qMin(block.length()-1, cursorColumn));
        QRectF cursorRect = mEdit->cursorRect(cursor);
        if (block.length() <= cursorColumn) {
            cursorRect.translate((cursorColumn-block.length()+1) * spaceWidth, 0);
        }
        cursorRect.setWidth(2);

        if (cursorRect.bottom() >= evRect.top() && cursorRect.top() <= evRect.bottom()) {
            bool drawCursor = ((editable || (mEdit->textInteractionFlags() & Qt::TextSelectableByKeyboard)));
            if (drawCursor) {
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
    mSelections.clear();
    QTextCursor cursor(mEdit->document());
    for (int lineNr = qMin(mStartLine, mCurrentLine); lineNr <= qMax(mStartLine, mCurrentLine); ++lineNr) {
        QTextBlock block = mEdit->document()->findBlockByNumber(lineNr);
        QTextEdit::ExtraSelection select;
        // TODO(JM) Later, this color has to be retrieved from StyleSheet-definitions
        QColor beColor = QColor(Qt::cyan).lighter(150);
        select.format.setBackground(beColor);

        int start = qMin(block.length()-1, qMin(mColumn, mColumn+mSize));
        int end = qMin(block.length()-1, qMax(mColumn, mColumn+mSize));
        cursor.setPosition(block.position()+start);
        if (end>start) cursor.setPosition(block.position()+end, QTextCursor::KeepAnchor);
        select.cursor = cursor;
        mSelections << select;
        if (cursor.blockNumber() == mCurrentLine) {
            QTextCursor c = mEdit->textCursor();
            c.setPosition(cursor.position());
            mEdit->setTextCursor(c);
        }
    }
    mEdit->updateExtraSelections();
}

void CodeEditor::BlockEdit::adjustCursor()
{
    QTextBlock block = mEdit->document()->findBlockByNumber(mCurrentLine);
    QTextCursor cursor(block);
    cursor.setPosition(block.position() + qMin(block.length()-1, mColumn+mSize));
    mEdit->setTextCursor(cursor);
}

void CodeEditor::BlockEdit::replaceBlockText(QString text)
{
    replaceBlockText(QStringList() << text);
}

void CodeEditor::BlockEdit::replaceBlockText(QStringList texts)
{
    if (mEdit->isReadOnly()) return;
    if (texts.isEmpty()) texts << "";
    CharType charType = texts.at(0).length()>0 ? mEdit->charType(texts.at(0).at(0)) : CharType::None;
    bool newUndoBlock = texts.count()>1 || mLastCharType!=charType || texts.at(0).length()>1;
    // append empty lines if needed
    int missingLines = texts.count() - (qAbs(mStartLine - mCurrentLine) % texts.count()) - 1;
    if (missingLines > 0) {
        QTextBlock block = mEdit->document()->findBlockByNumber(qMax(mStartLine, mCurrentLine));
        QTextCursor cursor(block);
        cursor.setPosition(block.position() + block.length());
        cursor.beginEditBlock();
        cursor.insertText(QString(missingLines, '\n'));
        if (mStartLine > mCurrentLine) mStartLine += missingLines;
        else mCurrentLine += missingLines;
        cursor.endEditBlock();
        newUndoBlock = false;
    }

    int i = texts.count()-1;
    QTextBlock block = mEdit->document()->findBlockByNumber(qMax(mCurrentLine, mStartLine));
    int fromCol = qMin(mColumn, mColumn+mSize);
    int toCol = qMax(mColumn, mColumn+mSize);
    QTextCursor cursor = mEdit->textCursor();
    int maxLen = 0;
    for (const QString &s: texts) {
        if (maxLen < s.length()) maxLen = s.length();
    }

    if (newUndoBlock)  cursor.beginEditBlock();
    else cursor.joinPreviousEditBlock();

    QChar ch(' ');
    while (block.blockNumber() >= qMin(mCurrentLine, mStartLine)) {
//        if (ch=='.') ch='0'; else ch=',';
        QString addText = texts.at(i);
        if (maxLen && addText.length() < maxLen)
            addText += QString(maxLen-addText.length(), ch);
        int offsetFromEnd = fromCol - block.length()+1;
        if (offsetFromEnd > 0 && !texts.at(i).isEmpty()) {
            // line ends before start of mark -> calc additional spaces
            cursor.setPosition(block.position()+block.length()-1);
            QString s(offsetFromEnd, ch);
            addText = s + addText;
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
        block = block.previous();
        i = (i>0) ? i-1 : texts.count()-1;
    }
    cursor.endEditBlock();
    if (mSize < 0) mColumn += mSize;
    int insertWidth = -1;
    for (QString s: texts) {
        for (QStringRef ref: s.splitRef('\n')) {
            if (insertWidth < ref.length()) insertWidth = ref.length();
        }
    }
    mColumn += insertWidth;
    mSize = 0;
    mLastCharType = charType;
}


} // namespace studio
} // namespace gams
